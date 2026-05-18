#include "quadtree.h"
#include <stdio.h>

/* private prototypes */
static int
split_node_(quadtree_t *tree, quadtree_node_t *node, int depth);

static int
insert_(quadtree_t* tree, quadtree_node_t *root, quadtree_point_t *point, void *key, int depth);

static int
node_contains_(quadtree_node_t *outer, quadtree_point_t *it);

static quadtree_node_t *
get_quadrant_(quadtree_node_t *root, quadtree_point_t *point);

/* private implementations */
static int
node_contains_(quadtree_node_t *outer, quadtree_point_t *it) {
  /* Fix(low): NaN comparisons always return false; make the rejection explicit */
  if(isnan(it->x) || isnan(it->y)) return 0;
  return outer->bounds != NULL
      && outer->bounds->nw->x <= it->x
      && outer->bounds->nw->y >= it->y
      && outer->bounds->se->x >= it->x
      && outer->bounds->se->y <= it->y;
}

static void
elision_(void* key){}

static void
reset_node_(quadtree_t *tree, quadtree_node_t *node){
  if(tree->key_free != NULL) {
    quadtree_node_reset(node, tree->key_free);
  } else {
    quadtree_node_reset(node, elision_);
  }
}

static quadtree_node_t *
get_quadrant_(quadtree_node_t *root, quadtree_point_t *point) {
  if(node_contains_(root->nw, point)) return root->nw;
  if(node_contains_(root->ne, point)) return root->ne;
  if(node_contains_(root->sw, point)) return root->sw;
  if(node_contains_(root->se, point)) return root->se;
  return NULL;
}


static int
split_node_(quadtree_t *tree, quadtree_node_t *node, int depth){
  quadtree_node_t *nw;
  quadtree_node_t *ne;
  quadtree_node_t *sw;
  quadtree_node_t *se;
  quadtree_point_t *old;
  void *key;

  double x  = node->bounds->nw->x;
  double y  = node->bounds->nw->y;
  double hw = node->bounds->width / 2;
  double hh = node->bounds->height / 2;

                                    //minx,   miny,       maxx,       maxy
  nw = quadtree_node_with_bounds(x,      y - hh,     x + hw,     y);
  ne = quadtree_node_with_bounds(x + hw, y - hh,     x + hw * 2, y);
  sw = quadtree_node_with_bounds(x,      y - hh * 2, x + hw,     y - hh);
  se = quadtree_node_with_bounds(x + hw, y - hh * 2, x + hw * 2, y - hh);

  /* Fix(medium): free already-allocated siblings on partial-OOM */
  if(!nw || !ne || !sw || !se) {
    if(nw) quadtree_node_free(nw, elision_);
    if(ne) quadtree_node_free(ne, elision_);
    if(sw) quadtree_node_free(sw, elision_);
    if(se) quadtree_node_free(se, elision_);
    return 0;
  }

  node->nw = nw;
  node->ne = ne;
  node->sw = sw;
  node->se = se;

  old = node->point;
  key   = node->key;
  node->point = NULL;
  node->key   = NULL;

  /* Fix(critical): pass depth-1 to bound mutual recursion with insert_() */
  return insert_(tree, node, old, key, depth - 1);
}


static quadtree_point_t*
find_(quadtree_node_t* node, double x, double y) {
  if(!node){
    return NULL;
  }
  if(quadtree_node_isleaf(node)){
    if(node->point->x == x && node->point->y == y)
      return node->point;
  } else if(quadtree_node_ispointer(node)){
    quadtree_point_t test;
    test.x = x;
    test.y = y;
    return find_(get_quadrant_(node, &test), x, y);
  }

  return NULL;
}

/* cribbed from the google closure library. */
static int
insert_(quadtree_t* tree, quadtree_node_t *root, quadtree_point_t *point, void *key, int depth) {
  /* Fix(critical): enforce depth limit to break unbounded mutual recursion */
  if(depth <= 0) return 0;
  if(quadtree_node_isempty(root)){
    root->point = point;
    root->key   = key;
    return 1; /* normal insertion flag */
  } else if(quadtree_node_isleaf(root)){
    if(root->point->x == point->x && root->point->y == point->y){
      reset_node_(tree, root);
      root->point = point;
      root->key   = key;
      return 2; /* replace insertion flag */
    } else {
      if(!split_node_(tree, root, depth)){
        return 0; /* failed insertion flag */
      }
      return insert_(tree, root, point, key, depth - 1);
    }
  } else if(quadtree_node_ispointer(root)){
    quadtree_node_t* quadrant = get_quadrant_(root, point);
    return quadrant == NULL ? 0 : insert_(tree, quadrant, point, key, depth - 1);
  }
  return 0;
}


/* public */
quadtree_t*
quadtree_new(double minx, double miny, double maxx, double maxy) {
  quadtree_t *tree;
  if(!(tree = malloc(sizeof(*tree))))
    return NULL;
  tree->root = quadtree_node_with_bounds(minx, miny, maxx, maxy);
  if(!(tree->root)) {
    free(tree); /* Fix(critical): avoid leaking tree struct on root alloc failure */
    return NULL;
  }
  tree->key_free = NULL;
  tree->length = 0;
  return tree;
}

int
quadtree_insert(quadtree_t *tree, double x, double y, void *key) {
  quadtree_point_t *point;
  int insert_status;

  /* Fix(high): NULL tree causes immediate crash */
  if(!tree) return 0;
  /* Fix(high): NaN/Inf coordinates silently return 0 indistinguishable from OOM */
  if(!isfinite(x) || !isfinite(y)) return 0;
  if(!(point = quadtree_point_new(x, y))) return 0;
  if(!node_contains_(tree->root, point)){
    quadtree_point_free(point);
    return 0;
  }

  if(!(insert_status = insert_(tree, tree->root, point, key, QUADTREE_MAX_DEPTH))){
    quadtree_point_free(point);
    return 0;
  }
  if(insert_status == 1) {
    /* Fix(medium): guard against silent unsigned wrap at UINT_MAX */
    if(tree->length + 1u > tree->length) {
      tree->length++;
    }
  }
  return insert_status;
}

quadtree_point_t*
quadtree_search(quadtree_t *tree, double x, double y) {
  /* Fix(high): NULL tree causes crash */
  if(!tree) return NULL;
  return find_(tree->root, x, y);
}

void
quadtree_free(quadtree_t *tree) {
  /* Fix(high): NULL tree causes crash */
  if(!tree) return;
  if(tree->key_free != NULL) {
    quadtree_node_free(tree->root, tree->key_free);
  } else {
    quadtree_node_free(tree->root, elision_);
  }
  free(tree);
}

/* Fix(critical+medium): depth-bounded internal walk to prevent stack overflow;
   NULL guards on root, and both callbacks before invoking them */
static void
quadtree_walk_depth_(quadtree_node_t *root, void (*descent)(quadtree_node_t *node),
                     void (*ascent)(quadtree_node_t *node), int depth) {
  if(root == NULL || depth <= 0) return;
  if(descent) (*descent)(root);
  if(root->nw != NULL) quadtree_walk_depth_(root->nw, descent, ascent, depth - 1);
  if(root->ne != NULL) quadtree_walk_depth_(root->ne, descent, ascent, depth - 1);
  if(root->sw != NULL) quadtree_walk_depth_(root->sw, descent, ascent, depth - 1);
  if(root->se != NULL) quadtree_walk_depth_(root->se, descent, ascent, depth - 1);
  if(ascent) (*ascent)(root);
}

void
quadtree_walk(quadtree_node_t *root, void (*descent)(quadtree_node_t *node),
                                     void (*ascent)(quadtree_node_t *node)) {
  quadtree_walk_depth_(root, descent, ascent, QUADTREE_MAX_DEPTH);
}
