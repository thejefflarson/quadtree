#include <stdio.h>
#include <time.h>
#include "src/quadtree.h"

static void
bench(void (*bm)(void), char *label){
  printf(" %18s", label);
  fflush(stdout);
  bm();
}

static int nodes = 10000;
static clock_t startTime;

static void
start(void) {
  startTime = clock();
}

static void
stop(void) {
  float duration = (float) (clock() - startTime) / CLOCKS_PER_SEC;
  printf(": \e[32m%.4f\e[0ms\n", duration);
}

static void
mark_insert(void){
  int fail = 0;
  int success = 0;
  int replace = 0;
  int n = nodes;
  int val = 10;
  quadtree_t *tree = quadtree_new(0, 0, 1000, 1000);
  double x;
  double y;
  start();
  while(n--){
    x = (double) (rand() % 1000);
    y = (double) (rand() % 1000);
    int status = quadtree_insert(tree, x, y, &val);
    switch(status){
      case 0: ++fail; break;
      case 1: ++success; break;
      case 2: ++replace; break;
    }
  }
  stop();
  printf("  %18s %i\n", "length:", tree->length);
  printf("  %18s %d\n", "insert success:", success);
  printf("  %18s %d\n", "insert replace:", replace);
  printf("  %18s %d\n", "insert fail:", fail);
  quadtree_free(tree);
}

int
main(int argc, const char *argv[]){
  srand(time(NULL));
  bench(mark_insert, "insertion");
  return 0;
}
