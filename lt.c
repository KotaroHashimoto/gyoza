#include <stdio.h>
#include <stdlib.h>

#define SIZE 512 //ノードの数
#define ACTIVE_NUM 4 //初期状態でActiveなノードの数
#define THETA_MAX 0.50 //ノード閾値上限
#define RETRY_LIMIT 4 //Activeノードが増えない試行がこの回数以上続いたら停止する

struct node {
  int isActive: 1; //Active/非Active状態
  double theta; //このノードの閾値
  double weight[SIZE]; //このノード(子)への、親ノードからの重み配列
};

void init(struct node* vArray);
int iterate(struct node* vArray);


void init(struct node* vArray) {

  int i, j;
  double w;

  // seedを固定して再現性を確保
  srand((unsigned)0);

  for(i = 0; i < SIZE; i++) {

    //閾値を設定
    vArray[i].theta = ((double)rand() / (double)RAND_MAX) * THETA_MAX;
    
    for(j = 0, w = 0.0; j < SIZE; j++) {
      if(i == j) {
	vArray[i].weight[j] = 0;
      }
      else { //重みを設定
	vArray[i].weight[j] = ((double)rand() / (double)RAND_MAX) / (double)SIZE;

	if(1.0 < w + vArray[i].weight[j]) { //重みの総和は1.0以下
	  vArray[i].weight[j] = 0.0;
	}
	else {
	  w += vArray[i].weight[j];
	}
      }

      if(i < ACTIVE_NUM) { //初期状態でActiveなノード
	vArray[i].isActive = 1;
      }
      else {
	vArray[i].isActive = 0;
      }
    }
  }
}


int iterate(struct node* vArray) {

  int i, j, n;
  double w;

  for(i = 0, n = 0; i < SIZE; i++) {

    if(vArray[i].isActive) {
      n ++; //Activeノードの数をカウント
      continue;
    }

    for(j = 0, w = 0.0; j < SIZE; j++) {
      //Activeノードからの重み総和
      w += vArray[j].isActive ? vArray[i].weight[j] : 0.0;
    }

    if(vArray[i].theta < w) {
      //重み総和が閾値を超えていたらActive状態にする
      vArray[i].isActive = 1;
      n ++;
    }
  }

  return n;
}


int main(void) {

  int t, n, m, r;
  struct node vArray[SIZE];

  //グラフを初期化
  init(vArray);

  //何も更新されなくなるか、全てActiveになるまで時刻を進めて状態を観察する
  for(t = 0, r = 0, n = ACTIVE_NUM, m = ACTIVE_NUM; n < SIZE && r < RETRY_LIMIT; t++) {

    printf("時刻 t = %d, Activeノード %d / %d\n", t, n, SIZE);
    n = iterate(vArray); //次の時刻でActive状態のノードの数を更新・取得

    if(n == m) {
      //時刻が進んでも何も更新がない場合
      r ++;
    }
    else {
      r = 0;
      m = n;
    }
  }

  return 0;
}
