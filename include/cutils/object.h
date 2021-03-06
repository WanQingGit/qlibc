/*
 * object.h
 *
 *  Created on: 2019年2月21日
 *      Author: WanQing
 */

#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/base.h"
#include "cutils/conf.h"
#include "setjmp.h"

typedef enum {
    ERR_NO, ERR_MEM, ERR_RUN, ERR_ASSERT
} errcode;

/*type of variable*/
typedef enum {
    V_NIL,
    V_STR,
    V_NUMFLT,
    V_NUMINT,
    V_BOOL,
    V_TABLE,
    V_LIST,
    V_ARRAY,
    V_RBTREE,
    V_TYPE,
} qtype;
#define HASHMASK (~((size_t)0))
typedef struct QHashMap qmap;
typedef struct qobj qobj;
typedef struct qvector *qvec;
typedef struct qstr qstr;
typedef struct qbytes qbytes;
#define OBJ(v, t) {{cast(void*,v)},t}

#define o2gc(o) (cast(GCNode*,o)-1)
#define incr_ref(o) o2gc(o)->nref++

typedef void *(*errfun)(void *u, errcode code, char *msg);

typedef size_t (*hashfun)(const qobj *o);

typedef void (*serialfun)(qbytes *l, void *o);

typedef int (*deserialfun)(byte *l, intptr_t *o);

typedef size_t (*comparefun)(void *a, void *b);
typedef size_t (*assignfunc)(void *dst, void *src);

typedef char *(*o2strfun)(qbytes *l, qobj *o);

typedef void (*freefun)(qobj *o);

#define sizearr(x) (1<<(x))
typedef struct typeobj {
    comparefun compare; //同类型比较函数
    hashfun hash;
    serialfun serialize;
    deserialfun deserial;
    o2strfun toString;
    freefun free;
    assignfunc assign;
    size_t id;
    size_t size;
    char name[16];
} typeobj, *Type;
//extern const  uint HASHMASK = -1;
#define TYPEDEFINE \
 const Type typeType, typeInt, typeFloat, typeBool, typeString, typeMap,\
        typeList, typeCFun, typeRBTree,typeNULL;
extern TYPEDEFINE


typedef struct GCNode {
    struct GCNode *next;
    struct GCNode *prev;
    ssize_t size;
    ssize_t gcref;
    ssize_t nref;
    typeobj *type;
} GCNode;
//    struct qstr *hnext;
struct qstr {
    size_t hash;
    int len;
    char val[];
};

struct qbytes {
    int length;
    int capacity;
    byte *data;
    int datasize;
};


typedef struct gc {
    GCNode *allgc; /* list of all collectable objects */
    ptrdiff_t GCdebt; /* bytes allocated not yet compensated by the collector */
} GC;
struct longjmp {
    struct longjmp *prev;
    jmp_buf b;
    volatile errcode status;
};
typedef struct RBTree RBTree;
typedef struct glstate {
    GC gc;
    void* strt;
    RBTree *typeinfos;
    errfun errf;
    uint seed;
} gl_state;
typedef struct qstate {
    gl_state *g;
    struct qstate *up;
    struct longjmp *errorJmp;
    errfun errf;
} State;

typedef union qval {
    void *p;
    qmap *t;
    qvec arr;
    qstr *s;
    INT i;
    FLT flt;
    qobj *obj;
    signed int32: 32;
    float flt32;
} qval;
typedef struct qobj {
    typeobj *type;
    qval val;
} qobj;

extern qobj nilobj;

typedef int (*qfunc)(State *S, int argc, int resc);



typedef enum {
    LOG_LAZY = 0, LOG_ACTIVE = 1 << 2, LOG_NORMAL = 1 << 3, LOG_CONSOLE = 1 << 4
} LogPolicy;


extern State *_S;
extern gl_state *_G;

Type createType(char *name, comparefun compare, hashfun hash,
                serialfun serialize, deserialfun deserial, o2strfun toString, freefun free,
                size_t baseType);

bool destroyType(qstr *name);

#define typeCompare(compare) createType(NULL, cast(comparefun,compare), NULL, NULL, NULL, NULL, NULL, V_NIL)
#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_OBJECT_H_ */
