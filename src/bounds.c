#include "quadtree.h"

#ifndef INFINITY
  // no infinity defined? high chance we are compiling as c89
  #include <float.h>
  #include <math.h>
  #define INFINITY DBL_MAX
  /* Fix(low): plain < / > silently ignore NaN; use self-inequality trick to
     propagate NaN so callers can detect bad coordinates */
  #define fmin(a,b) ((a)!=(a)?(a):(b)!=(b)?(b):((a)<(b))?(a):(b))
  #define fmax(a,b) ((a)!=(a)?(a):(b)!=(b)?(b):((a)>(b))?(a):(b))
#endif



void
quadtree_bounds_extend(quadtree_bounds_t *bounds, double x, double y){
  bounds->nw->x = fmin(x, bounds->nw->x);
  bounds->nw->y = fmax(y, bounds->nw->y);
  bounds->se->x = fmax(x, bounds->se->x);
  bounds->se->y = fmin(y, bounds->se->y);
  bounds->width  = fabs(bounds->nw->x - bounds->se->x);
  bounds->height = fabs(bounds->nw->y - bounds->se->y);
}

void
quadtree_bounds_free(quadtree_bounds_t *bounds){
  quadtree_point_free(bounds->nw);
  quadtree_point_free(bounds->se);
  free(bounds);
}


quadtree_bounds_t*
quadtree_bounds_new(void){
  quadtree_bounds_t *bounds;
  if((bounds = malloc(sizeof(*bounds))) == NULL)
    return NULL;
  /* Fix(high): check each quadtree_point_new() return and clean up on failure */
  bounds->nw = quadtree_point_new(INFINITY, -INFINITY);
  if(!bounds->nw) {
    free(bounds);
    return NULL;
  }
  bounds->se = quadtree_point_new(-INFINITY, INFINITY);
  if(!bounds->se) {
    quadtree_point_free(bounds->nw);
    free(bounds);
    return NULL;
  }
  bounds->width  = 0;
  bounds->height = 0;
  return bounds;
}
