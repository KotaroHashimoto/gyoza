#include <stdio.h>
#include <stdlib.h>

#define SIZE 512 //ノードの数
#define THETA_MAX 0.50 //ノード閾値上限
#define GREEDY_ITERATION 1024 //ノード追加ごとに重みを決める貪欲アルゴリズムの繰り返し回数
#define RETRY_LIMIT 4 //Activeノードが増えない試行がこの回数以上続いたら停止する

struct node {
  int isActive: 1; //Active/非Active状態
  double theta; //このノードの閾値
  double weight[SIZE]; //このノード(子)への、親ノードからの重み配列
};

void init(struct node* vArray);
int iterate(struct node* vArray);
struct node nextNode(int currentSize);
void writeMatrix(struct node* vArray);


// 貪欲アルゴリズム各試行中で使う次のノードを返す
struct node nextNode(int currentSize) {

  int j;
  double w;
  struct node newNode;
  
  newNode.isActive = 0; //自身のノードは非Active状態
  newNode.theta = ((double)rand() / (double)RAND_MAX) * THETA_MAX; //自身のノードの閾値

  for(j = 0, w = 0.0; j < SIZE; j++) {

    if(currentSize == j) {
      newNode.weight[j] = 0.0;
    }
    else {
      // 貪欲試行する重み付け
      newNode.weight[j] = ((double)rand() / (double)RAND_MAX) / (double)SIZE;

      if(1.0 < w + newNode.weight[j]) { //重みの総和は1.0以下
	newNode.weight[j] = 0.0;
      }
      else {
	w += newNode.weight[j];
      }
    }
  }

  return newNode;
}


void init(struct node* vArray) {

  int i, j;
  double w;

  srand((unsigned)0);

  //はじめは一つのActiveノードだけ
  vArray[0].theta = ((double)rand() / (double)RAND_MAX) * THETA_MAX;
  vArray[0].isActive = 1;
  vArray[0].weight[0] = 0;

  for(j = 1, w = 0.0; j < SIZE; j++) {
    vArray[0].weight[j] = ((double)rand() / (double)RAND_MAX) / (double)SIZE;

    if(1.0 < w + vArray[0].weight[j]) { //重みの総和は1.0以下
      vArray[0].weight[j] = 0.0;
    }
    else {
      w += vArray[0].weight[j];
    }
  }

  for(i = 1; i < SIZE; i++) {
    vArray[i].isActive = 0;
    vArray[i].theta = -1.0;

    for(j = 0; j < SIZE; j++) {
      vArray[i].weight[j] = -1;
    }
  }
}


void writeMatrix(struct node* vArray) {

  int i, j;
  FILE* tfp = fopen("ThresholdMatrix.txt", "w");
  FILE* wfp = fopen("WeightMatrix.txt", "w");

  for(i = 0; i < SIZE; i++) {
    fprintf(tfp, "%lf,", vArray[i].theta);
    
    for(j = 0; j < SIZE; j++) {
      fprintf(wfp, "%lf,", vArray[i].weight[j]);
    }
    fprintf(wfp, "\n");
  }

  fclose(tfp);
  fclose(wfp);
}


int iterate(struct node* vArray) {

  int i, j, n;
  double w;

  for(i = 0, n = 0; i < SIZE && 0 < vArray[i].theta; i++) {

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

  int t, n, m, r, i, j, max;
  struct node vArray[SIZE];
  struct node nextCandidate;

  //グラフを初期化
  init(vArray);

  for(int i = 1; i < SIZE; i++) {

    j = GREEDY_ITERATION;
    max = -1; 

    // GREEDY_ITERATION 回の試行で、最終状態で最もActiveノード数が多くなる新ノードを探す
    while(0 < j--) {      
      struct node iterNode = nextNode(i);
      vArray[i] = iterNode;
      
      //何も更新されなくなるか、全てActiveになるまで時刻を進めて状態を観察する
      for(t = 0, r = 0, n = 1, m = 1; n < i + 1 && r < RETRY_LIMIT; t++) {

	// printf("時刻 t = %d, Activeノード %d / %d\n", t, n, i + 1);
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

      if(max < n) {
	// 最終状態で最もActiveノード数が多くなる新ノードを記憶
	max = n;
	nextCandidate = iterNode;
      }
    };
      
    // 探索したノードを追加
    vArray[i] = nextCandidate;
    printf("%d / %dノード決定、最終Activeノード数 = %d\n", i + 1, SIZE, max);
  }

  //閾値配列、重み行列をファイル出力
  writeMatrix(vArray);

  return 0;
}
