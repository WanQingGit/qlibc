/*
 *
 *  Created on: Apr 11, 2019
 *  Author: WanQing<1109162935@qq.com>
 *
 *  定制的string常量池比使用hashmap效率要高
 */

#include <std/string.h>
#include <stl/vector.h>
#include <string.h>
#include "std/string.h"
#include "strutils.h"
#include "mem.h"

static qstr *string_createlstr(size_t l, const char *str);

static qstr *string_get(const char *str);

static qstr *newstr(size_t l, unsigned int h,
                    const char *str);

static qstr *string_new(const char *str, size_t l);



void string_table_resize(int newsize) {
  int i;
  qstr *p, *hnext;
  stringtable *tb = &(_G->strt);
  if (newsize != tb->size) { /* grow table if needed */
    mem_realloc_vector(tb->ht, tb->size, newsize, qstr*);
  }
  int mod = newsize - 1;
  for (i = 0; i < tb->size; i++) { /* rehash */
    p = tb->ht[i];
    tb->ht[i] = NULL;
    while (p) { /* for each node in the list */
      hnext = p->hnext; /* save next */
      unsigned int h = cast(uint, p->hash & mod); /* new position */
      p->hnext = tb->ht[h]; /* chain it */
      tb->ht[h] = p;
      p = hnext;
    }
  }
  tb->size = newsize;
}
typeobj _typelstr;
Type  typeLstr=&_typelstr;
static qstr *gshrstr( const char *str, size_t l) {
  qstr *ts;
  gl_state *g = _G;
  unsigned int h = str_hash_count(str, l, g->seed);
  qstr **list = &g->strt.ht[h & (g->strt.size - 1)];
  qassert(str != NULL);
  for (ts = *list; ts; ts = ts->hnext) {
    if (l == ts->len && h == ts->hash
        && (memcmp(str, gstr(ts), l * sizeof(char)) == 0)) {
//            if (isdead(g, o2gc(ts))) /* dead (but not collected yet)? */
//                changewhite(o2gc(ts)); /* resurrect it */
      return ts;
    }
  }
  if (g->strt.nuse >= g->strt.size && g->strt.size <= UINT_MAX / 2) {
    string_table_resize(g->strt.size * 2);
    list = &g->strt.ht[h & (g->strt.size - 1)]; /* recompute with new size */
  }
  ts = newstr(l, h, str);
  incr_ref(ts);
//  GCNode *obj=o2gc(ts);
//  obj->nref--;
//  if(obj->nref<=0){
//    assert(obj->nref==0);
//    obj->type->free(obj+1);
//    qfree(obj);
//  }
  ts->hnext = *list;
  *list = ts;
  g->strt.nuse++;
  return ts;
}

static qstr *string_get(const char *str) {
  return string_new(str, strlen(str));
}

static qstr *string_new(const char *str, size_t l) {
  if (l <= MAXSHORTLEN)
    return gshrstr( str, l);
  else {
    qstr *ts = string_createlstr(l, str);
    return ts;
  }
}
static qstr *string_del(qstr *str){
  gl_state *g = _G;
  qstr *nextstr=str->hnext;
  qstr **list = &g->strt.ht[str->hash & (g->strt.size - 1)];
}

static qstr *string_createlstr(size_t l, const char *str) {
  qstr *ts = newstr(l, 0, str);
  return ts;
}

static qstr *newstr(size_t l, unsigned int h,
                    const char *str) {
  size_t totalsize = sizeof(qstr) + (l + 1) * sizeof(char);
  qstr *ts = cast(qstr*, Mem.new(typeString, totalsize));
  ts->hash = h;
  ts->len = l;
  memcpy(gstr(ts), str, l * sizeof(char));
  gstr(ts)[l] = '\0';
  return ts;
}

static size_t  strt_size() {
  return _G->strt.nuse;
}

void strt_destroy() {
  stringtable *strt = &_G->strt;
  qstr *str;
  int size = strt->size;
  for (int i = 0; i < size; i++) {
    str = strt->ht[i];
    while (str) {
      qstr *s = str;
      str = str->hnext;
      Mem.alloc(o2gc(s), sizeof(qstr) + (s->len + 1) + sizeof(GCNode),
                0);
    }
    strt->ht[i] = NULL;
  }
  strt->nuse = 0;
  Mem.alloc(strt->ht,size*sizeof(void*),0);
}
void strt_init(){
  memcpy(&STR, typeString, sizeof(struct typeobj));
  memcpy(typeLstr,typeString,sizeof(struct typeobj));
  typeLstr->free=NULL;
}

struct QString STR = {.init_env=strt_init,strt_destroy,string_new, string_get, string_table_resize,
                     strt_size};
