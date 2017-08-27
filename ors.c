#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 512 //ノードの数
#define INIT_S_NUM 1 //はじめにSenderであるノードの数
#define THRESHOULD 1 //閾値
#define LAMBDA 0.125 //忘却率λ
#define OMAX 8 //次数の最大値
#define OMIN 2 //次数の最小値
#define RETRY 512 //試行回数
#define NEW_INFO_PROB 0.0005 //各試行、各ノードが新しい情報を発信するようになる確率

#define O 0 //Outsider
#define R 1 //Receiver
#define S 2 //Sender

struct node {
  int status: 3; //ORSモデル状態
  int a; //影響度
  double i; //興味度
  double s; //感度
  int FG; //はじめに情報を受け取った時刻
  int infoID; //現在持っている情報ID
  double mot; //現在のMOT
  double an[SIZE]; //このノードが情報を与えるノードのインデックス
};

FILE* fp;

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

    //影響度設定
    vArray[j].a = (int)(((double)rand() / (double)RAND_MAX) * (double)(OMAX - OMIN)) + OMIN;

    //何も情報を持っていない状態に初期化
    vArray[j].infoID = -1;

    for(k = 0; k < SIZE; k++) {
      vArray[j].an[k] = -1;
    }

    // ノードjが影響を与えるノードindex設定
    c = vArray[j].a;
    while(c) {
      int index = SIZE * ((double)rand() / (double)RAND_MAX);
      if(index == j || 0 <= vArray[j].an[index]) {
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
    vArray[index].infoID = rand();
  }


  // Senderの影響度を出力するファイル
  fp = fopen("Sender_Influences.csv", "w");
}


void iterate(struct node* vArray, int* o, int* r, int* s, int t) {

  int j, k;

  //新しい情報を発信する新たなSender状態となるノードを作る
  for(j = 0; j < SIZE; j++) {
    double p = (double)rand() / (double)RAND_MAX;
    if(p < NEW_INFO_PROB) {
      vArray[j].status = S;
      vArray[j].FG = 0;
      vArray[j].infoID = rand();
    }
  }


  // MOT計算
  for(j = 0; j < SIZE; j++) {

    // 忘却率設定に従ってMOTを更新
    if(0 <= vArray[j].FG) {
      //      printf("mot = %lf\n", vArray[j].mot);
      vArray[j].mot /= pow(2.71828, LAMBDA * (double)(t - vArray[j].FG));
    }

    // 情報の伝播プロセス
    for(k = 0; k < SIZE; k++) {    
      int ti = vArray[j].an[k];
      if(ti < 0) {
	continue; // 自身jが影響を与えないノードkは無視
      }
      else if(vArray[j].status != S) {
	continue; // 自身がsenderでなかったらスキップ
      }


      if(vArray[ti].infoID != vArray[j].infoID) {
        vArray[ti].mot = 0; //新規情報を伝播させた場合、相手側のMOTをリセット
      }

      // 影響を受ける側のMOT, FGを更新
      vArray[ti].mot += vArray[ti].i * vArray[ti].s * vArray[j].a;
      vArray[ti].FG = (vArray[ti].FG < t ? t : vArray[ti].FG);
    }
  }
  

  // Outsider -> Receiver <-> Sender への状態遷移
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


  fprintf(fp, "time %d,", t);

  // 各ステップでの Outsider, Receiver, Sender のノード数をカウント
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
      fprintf(fp, "%d,", vArray[j].a);  
      break;
    }
  }

  fprintf(fp, "\n");
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


  fclose(fp);
  return 0;
}
