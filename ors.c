#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 512 //ノードの数
#define INIT_S_NUM 1 //はじめにSenderであるノードの数
#define THRESHOULD 1 //閾値
#define LAMBDA 0.125 //忘却率λ
#define OMAX 8 //次数の最大値
#define OMIN 2 //次数の最小値
#define RETRY 32 //試行回数

#define O 0 //Outsider
#define R 1 //Receiver
#define S 2 //Sender

struct node {
  int status: 3; //ORSモデル状態
  int a; //影響度
  double i; //興味度
  double s; //感度
  int FG; //はじめに情報を受け取った時刻
  double mot; //現在のMOT
  double an[SIZE]; //このノードが情報を与えるノードのインデックス
};

void init(struct node* vArray);
void iterate(struct node* vArray, int* o, int* r, int* s, int t);

void init(struct node* vArray) {

  int j, k, c;

  // seedを固定して再現性を確保
  srand((unsigned)0);

  for(j = 0; j < SIZE; j++) {

    //興味度設定
    vArray[j].i = ((double)rand() / (double)RAND_MAX);

    //感度設定
    vArray[j].s = ((double)rand() / (double)RAND_MAX);

    //ORS状態を Outsider に初期化
    vArray[j].status = O;

    //感度設定
    vArray[j].s = ((double)rand() / (double)RAND_MAX);

    //影響度設定
    vArray[j].a = (int)(((double)rand() / (double)RAND_MAX) * (double)(OMAX - OMIN)) + OMIN;


    for(k = 0; k < SIZE; k++) {
      vArray[j].an[k] = -1;
    }

    // ノードjが影響を与えるノードj
    c = vArray[j].a;
    while(c) {
      int index = SIZE * ((double)rand() / (double)RAND_MAX);
      if(index == j) {
	continue;
      }
      else {
	vArray[j].an[index] = index;
	c--;
      }
    }

    vArray[j].FG = -1;
    vArray[j].mot = 0;
  }

  //はじめにSender状態にいるノードを作る
  for(j = 0; j < INIT_S_NUM; j++) {
    int index = SIZE * ((double)rand() / (double)RAND_MAX);
    if(vArray[index].status == S) {
      j--;
      continue;
    }
    vArray[index].status = S;
    vArray[index].FG = 0;
  }
}


void iterate(struct node* vArray, int* o, int* r, int* s, int t) {

  int j, k;

  // MOT計算
  for(j = 0; j < SIZE; j++) {

    if(0 <= vArray[j].FG) {
      //      printf("mot = %lf\n", vArray[j].mot);
      vArray[j].mot /= pow(2.71828, LAMBDA * (double)(t - vArray[j].FG));
    }

    for(k = 0; k < SIZE; k++) {    
      int ti = vArray[j].an[k];
      if(ti < 0) {
	continue;
      }
      else if(vArray[j].status != S) {
	continue;
      }

      vArray[ti].mot += vArray[ti].i * vArray[ti].s * vArray[j].a;
      vArray[ti].FG = (vArray[ti].FG < t ? t : vArray[ti].FG);
    }
  }
  

  for(j = 0; j < SIZE; j++) {

    //    printf("%lf\n", vArray[j].mot);
    if(0 <= vArray[j].FG) {
      if(THRESHOULD < vArray[j].mot) {
	vArray[j].status = S;
      }
      else {
	vArray[j].status = R;
      }
    }
  }


  (*o) = 0;
  (*r) = 0;
  (*s) = 0;

  for(j = 0; j < SIZE; j++) {
    switch(vArray[j].status) {
    case O:
      (*o) ++;
      break;
    case R:
      (*r) ++;
      break;
    case S:
      (*s) ++;
      break;
    }
  }
}


int main(void) {

  int t, o, r, s;
  struct node vArray[SIZE];

  //グラフを初期化
  init(vArray);

  //RETRY回繰り返し
  for(t = 0, o = SIZE - INIT_S_NUM, r = 0, s = INIT_S_NUM; t < RETRY; t++) {

    printf("時刻 t = %d, Outsider:%d, Receiver:%d, Sender:%d\n", t, o, r, s);
    iterate(vArray, &o, &r, &s, t); //次の時刻でノードの状態を更新・取得
  }

  return 0;
}
