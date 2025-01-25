#include "quadtree.h"

/* helpers */

int
quadtree_node_ispointer(quadtree_node_t *node){
  return node->nw != NULL
      && node->ne != NULL
      && node->sw != NULL
      && node->se != NULL
      && !quadtree_node_isleaf(node);
}

int
quadtree_node_isempty(quadtree_node_t *node){
  return node->nw == NULL
      && node->ne == NULL
      && node->sw == NULL
      && node->se == NULL
      && !quadtree_node_isleaf(node);
}

int
quadtree_node_isleaf(quadtree_node_t *node){
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
  if(!(node->bounds = quadtree_bounds_new())) return NULL;
  quadtree_bounds_extend(node->bounds, maxx, maxy);
  quadtree_bounds_extend(node->bounds, minx, miny);
  return node;
}

void
quadtree_node_free(quadtree_node_t* node, void (*key_free)(void*)) {
  if(node->nw != NULL) quadtree_node_free(node->nw, key_free);
  if(node->ne != NULL) quadtree_node_free(node->ne, key_free);
  if(node->sw != NULL) quadtree_node_free(node->sw, key_free);
  if(node->se != NULL) quadtree_node_free(node->se, key_free);

  quadtree_bounds_free(node->bounds);
  quadtree_node_reset(node, key_free);
  free(node);
}
