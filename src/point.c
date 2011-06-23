#include "quadtree.h"

quadtree_point_t*
quadtree_point_new(double x, double y) {
  quadtree_point_t* point;
  if(!(point = malloc(sizeof(*point))))
    return NULL;
  point->x = x;
  point->y = y;
  return point;
}

void
quadtree_point_free(quadtree_point_t *point){
  free(point);
}
