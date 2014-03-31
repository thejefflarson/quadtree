#include <assert.h>
#include <stdio.h>

#include "src/quadtree.h"

#define test(fn) \
        printf("\x1b[33m" # fn "\x1b[0m "); \
        test_##fn(); \
        puts("\x1b[1;32m ✓ \x1b[0m");



void descent(quadtree_node_t *node){
  if(node->bounds != NULL)
    printf("{ nw.x:%f, nw.y:%f, se.x:%f, se.y:%f }: ", node->bounds->nw->x,
      node->bounds->nw->y, node->bounds->se->x, node->bounds->se->y);
}

void ascent(quadtree_node_t *node){
  printf("\n");
}

static void
test_node(){
  quadtree_node_t *node = quadtree_node_new();
  assert(!quadtree_node_isleaf(node));
  assert(quadtree_node_isempty(node));
  assert(!quadtree_node_ispointer(node));
  free(node);
}

static void
test_bounds(){
  quadtree_bounds_t *bounds = quadtree_bounds_new();

  assert(bounds);
  assert(bounds->nw->x == INFINITY);
  assert(bounds->se->x == -INFINITY);

  quadtree_bounds_extend(bounds, 5.0, 5.0);
  assert(bounds->nw->x == 5.0);
  assert(bounds->se->x == 5.0);

  quadtree_bounds_extend(bounds, 10.0, 10.0);
  assert(bounds->nw->y == 10.0);
  assert(bounds->nw->y == 10.0);
  assert(bounds->se->y == 5.0);
  assert(bounds->se->y == 5.0);

  assert(bounds->width == 5.0);
  assert(bounds->height == 5.0);

  quadtree_bounds_free(bounds);
}


static void
test_tree(){
  int val = 10;

  quadtree_t *tree = quadtree_new(1, 1, 10, 10);
  assert(tree->root->bounds->nw->x == 1);
  assert(tree->root->bounds->nw->y == 10.0);
  assert(tree->root->bounds->se->x == 10.0);
  assert(tree->root->bounds->se->y == 1);


  assert(quadtree_insert(tree, 0, 0, &val) == 0);
  assert(quadtree_insert(tree, 110.0, 110.0, &val) == 0);

  assert(quadtree_insert(tree, 8.0, 2.0, &val) != 0);
  assert(tree->length == 1);
  assert(tree->root->point->x == 8.0);
  assert(tree->root->point->y == 2.0);

  assert(quadtree_insert(tree, 0.0, 1.0, &val) == 0); /* failed insertion */
  assert(quadtree_insert(tree, 2.0, 3.0, &val) == 1); /* normal insertion */
  assert(quadtree_insert(tree, 2.0, 3.0, &val) == 2); /* replacement insertion */
  assert(tree->length == 2);
  assert(tree->root->point == NULL);

  assert(quadtree_insert(tree, 3.0, 1.1, &val) == 1);
  assert(tree->length == 3);
  assert(quadtree_search(tree, 3.0, 1.1)->x == 3.0);
  quadtree_walk(tree->root, ascent, descent);
  quadtree_free(tree);
}

static void
test_points(){
  quadtree_point_t *point = quadtree_point_new(5, 6);
  assert(point->x == 5);
  assert(point->y == 6);
  quadtree_point_free(point);
}

int
main(int argc, const char *argv[]){
  printf("\nquadtree_t: %ld\n", sizeof(quadtree_t));
  printf("quadtree_node_t: %ld\n", sizeof(quadtree_node_t));
  printf("quadtree_bounds_t: %ld\n", sizeof(quadtree_bounds_t));
  printf("quadtree_point_t: %ld\n", sizeof(quadtree_point_t));
  test(tree);
  test(node);
  test(bounds);
  test(points);
}
