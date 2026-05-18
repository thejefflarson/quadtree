#include "quadtree.h"

/* helpers */

int
quadtree_node_ispointer(quadtree_node_t *node){
  /* Fix(medium): NULL node must not be dereferenced */
  if(node == NULL) return 0;
  return node->nw != NULL
      && node->ne != NULL
      && node->sw != NULL
      && node->se != NULL
      && !quadtree_node_isleaf(node);
}

int
quadtree_node_isempty(quadtree_node_t *node){
  /* Fix(medium): NULL node must not be dereferenced */
  if(node == NULL) return 0;
  return node->nw == NULL
      && node->ne == NULL
      && node->sw == NULL
      && node->se == NULL
      && !quadtree_node_isleaf(node);
}

int
quadtree_node_isleaf(quadtree_node_t *node){
  /* Fix(medium): NULL node must not be dereferenced */
  if(node == NULL) return 0;
  return node->point != NULL;
}

void
quadtree_node_reset(quadtree_node_t* node, void (*key_free)(void*)) {
  quadtree_point_free(node->point);
  (*key_free)(node->key);
}

/* api */
quadtree_node_t*
quadtree_node_new(void) {
  quadtree_node_t *node;
  if(!(node = malloc(sizeof(*node))))
    return NULL;
  node->ne     = NULL;
  node->nw     = NULL;
  node->se     = NULL;
  node->sw     = NULL;
  node->point  = NULL;
  node->bounds = NULL;
  node->key    = NULL;
  return node;
}

quadtree_node_t*
quadtree_node_with_bounds(double minx, double miny, double maxx, double maxy){
  quadtree_node_t* node;
  if(!(node = quadtree_node_new())) return NULL;
  node->bounds = quadtree_bounds_new();
  if(!(node->bounds)) {
    free(node); /* Fix(high): free node to prevent leak on bounds alloc failure */
    return NULL;
  }
  quadtree_bounds_extend(node->bounds, maxx, maxy);
  quadtree_bounds_extend(node->bounds, minx, miny);
  return node;
}

/* Fix(high): depth-bounded recursion to prevent stack overflow on deep trees;
   each child level decrements depth — at QUADTREE_MAX_DEPTH we stop descending */
static void
quadtree_node_free_depth_(quadtree_node_t* node, void (*key_free)(void*), int depth) {
  if(node == NULL) return;
  if(depth > 0) {
    if(node->nw != NULL) quadtree_node_free_depth_(node->nw, key_free, depth - 1);
    if(node->ne != NULL) quadtree_node_free_depth_(node->ne, key_free, depth - 1);
    if(node->sw != NULL) quadtree_node_free_depth_(node->sw, key_free, depth - 1);
    if(node->se != NULL) quadtree_node_free_depth_(node->se, key_free, depth - 1);
  }
  quadtree_bounds_free(node->bounds);
  quadtree_node_reset(node, key_free);
  free(node);
}

void
quadtree_node_free(quadtree_node_t* node, void (*key_free)(void*)) {
  quadtree_node_free_depth_(node, key_free, QUADTREE_MAX_DEPTH);
}
