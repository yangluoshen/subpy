

#ifndef _TM_H
#define _TM_H
/** 
 *  coding standards
 *  static variable . not specified
 *  function        . camel case starts with lower case
 *  structure       . camel case starts with upper case
 *  method          . like string_xxx, list_xxx
 *  builtins        . bf_xxx
 *  macro           . like XXX_XXX
*/

/* #pragma pack(4) */
#define DEBUG_GC 0
#define PRINT_INS 0
#define PRINT_INS_CONST 0
#define EVAL_DEBUG 1
#define DEBUG_INS 1
#define LOG_LEVEL 1
#define USE_NON_PARAM 1
#define RAISE_DEBUG 0

#ifdef _WIN32
#define TM_NT 1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <time.h>
/* #include <sys/stat.h> */
#include <math.h>

#define TM_INLINE inline
typedef char instruction;

#include "object.h"
#define OBJ_SIZE sizeof(Object)

Object NUMBER_TRUE;
Object NUMBER_FALSE;
Object NONE_OBJECT;
Object UNDEF;
Object ARRAY_CHARS;

#include "instruction.h"
#include "code.h"

// gc functions
#define GC_DEBUG_LIST 0
void*       tm_malloc( size_t size);
void*       tm_realloc(void* o, size_t osize, size_t nsize);
void        tm_free(void* o, size_t size);
void        initMemory();
void        freeMemory();

void        gcInit();
Object      gcTrack(Object obj);
void        gcDestroy();
void        gcFull();
Object      bfGetMallocInfo();
void        gcMark(Object);
void        gcMarkList(TmList*);
void        gcMarkDict(TmDict*);

#if 0
    #define PRINT_OBJ_GC_INFO_START() int _gc_old = tm->allocated;
    #define PRINT_OBJ_GC_INFO_END(str, addr) \
        printf("free %s at 0x%p, %d => %d, ", str, addr, _gc_old, tm->allocated);
    #else
    #define PRINT_OBJ_GC_INFO_START()
    #define PRINT_OBJ_GC_INFO_END(str, addr)

    #define GC_LOG_START(ptr, desc)
    #define GC_LOG_END(ptr, desc)
#endif

/**
 * string functions
 */


Object        stringCharNew(int c);
Object        stringChr(int n); // get a char from charList.
Object        stringAlloc(char* s, int size);
#define       szToString(s) stringAlloc(s, -1)
#define       stringNew(s) stringAlloc(s, strlen(s))
void          stringFree(String*);
int           stringEquals(String*s0, String*s1);
Object        stringSubstring(String* str, int start, int end) ;
Object        bf_stringFormat();
Object        tmStr(Object obj);
Object        StringJoin(Object self, Object list);
void          stringMethodsInit();
DataProto*    getStringProto();
Object        stringIterNew(String* s);
Object*       stringNext(StringIterator* iterator);

static DataProto stringProto;
/* macros */
#define GET_STR(obj) (obj).value.str->value
#define GET_SZ(obj)  (obj).value.str->value
#define GET_STR_OBJ(obj) (obj).value.str
#define GET_STR_LEN(obj) (obj).value.str->len


// number functions
Object tmNumber(double v);
void   numberFormat(char* des, Object num);

/**
 * list functions
 */

void     listCheck(TmList*);
Object   listNew(int cap);
/* create a TmList which not tracked by Garbage Collector. */
TmList*  untrackedListNew(int cap);
void     listSet(TmList* list, int n, Object v);
Object   listGet(TmList* list, int n);
void     listFree(TmList* );
void     listClear(TmList* list);
void     listMethodsInit();
Object   listIterNew(TmList* list);
Object*  listNext(TmListIterator* iterator);
Object   listAdd(TmList*, TmList*);
void     listDel(TmList*list, Object key);
int      listIndex(TmList*, Object val);
void     listAppend(TmList* list, Object v);

/** 
 * list iterator prototype
 */
DataProto listIterProto = { 0 };

/* macros */
#define LIST_GET(obj, i) listGet(GET_LIST(obj), i)
#define LIST_NODES(obj) (GET_LIST(obj))->nodes
#define LIST_LEN(obj) GET_LIST(obj)->len


// dict functions
Object           dictNew();
TmDict*          dictInit();
void             dictFree(TmDict* dict);
int              dictSet0(TmDict* dict, Object key, Object val);
#define          dictSet(d, k, v) dictSet0(GET_DICT(d), k, v)
DictNode*        dictGetNode(TmDict* dict, Object key);
Object*          dictGetByStr0(TmDict* dict, char* key);
void             dictDel(TmDict* dict, Object k);
void             dictMethodsInit();
void             dictSetByStr0(TmDict* dict, char* key, Object val);
#define          dictSetByStr(dict, key, val) dictSetByStr0(GET_DICT(dict), key, val)
#define          dictGetByStr(dict, key) dictGetByStr0(GET_DICT(dict), key)
Object           dictKeys(TmDict* );

/** dict methods **/
Object           dict_keys();
Object           dict_values();

static DataProto dictIterProto;

DataProto*       getDictIterProto();
Object           dict_iterNew(TmDict* dict);
Object*          dictNext(TmDictIterator* iterator);
int              dictSetAttr(TmDict* dict, int constId, Object val);
int              dictGetAttr(TmDict* dict, int constId);



// arg functions
void    argInsert(Object arg);
String* argTakeStrPtr(const char* fnc);
void    argStart();
void    argPush(Object obj) ;
void    argSetArguments(Object* first, int len);
void    _resolveMethodSelf(TmFunction *fnc);
#define resolveMethodSelf(fnc) _resolveMethodSelf(GET_FUNCTION((fnc)))
void    printArguments();
int     hasArg();
Object  argTakeStrObj(const char* fnc);
char*   argTakeSz(const char* fnc);
Object  argTakeFuncObj(const char* fnc);
int     argTakeInt(const char* fnc);
double  argTakeDouble(const char* fnc);
TmList* argTakeListPtr(const char* fnc);
Object  argTakeListObj(const char* fnc);
Object  argTakeDictObj(const char* fnc);
Object  argTakeObj(const char* fnc);
Object  argTakeDataObj(const char* fnc);
int     getArgsCount() ;
void    tmUngetArg();



// function functions


Object           funcNew(Object mod,Object self,Object (*native_func)());
Object           getFuncAttr(TmFunction* fnc, Object key);
void             funcFree(TmFunction*);
Object           methodNew(Object _fnc, Object self);
Object           moduleNew(Object file, Object name, Object code);
void             moduleFree(TmModule*);
unsigned char*   getFunctionCode(TmFunction*);
void             func_format(char* des, TmFunction* func);
TmModule*        getFuncMod(TmFunction* func);
Object           classNew(Object dict);
Object           getFunctionGlobals(TmFunction*);
unsigned char*   func_resolve(TmFunction*, unsigned char*);
Object           getFileNameObj(Object func);
Object           getFuncNameObj(Object func);
#define GET_FUNCTION(obj) (obj.value.func)
#define GET_FUNC(obj) ((obj).value.func)
#define IS_FUNCTION(o) TM_TYPE(o)==TYPE_FUNCTION
#define GET_FUNCTIONTION_MODULE_PTR(fnc) GET_MODULE(GET_FUNCTION(fnc)->mod)
#define GET_FUNC_CONSTANTS(fnc) GET_FUNCTIONTION_MODULE_PTR(fnc)->constants
#define GET_FUNC_MOD_NAME(fnc) GET_FUNCTIONTION_MODULE_PTR(fnc)->file
#define GET_FUNC_CONSTANTS_NODES(fnc) LIST_NODES(GET_FUNC_CONSTANTS(fnc))
#define GET_FUNCTION_NAME(fnc) GET_FUNCTION(fnc)->name
#define getGlobals(func) getFunctionGlobals(GET_FUNCTION(func))
#define functionFormat(des, func) func_format(des, GET_FUNCTION(func))

/**
 * data functions
 */
static DataProto baseIterProto;

Object     dataNew(size_t size);
void       dataMark();
void       dataFree();
void       dataSet(Object, Object, Object);
Object     dataGet(Object, Object);
Object     dataStr(Object self);
void       obj_free(Object o);
Object     objNew(int type, void* value);
DataProto  defaultDataProto;
DataProto* getDefaultDataProto();
void       initDataProto(DataProto* proto);



/** ops functions
 *  general object operation
 *  some tools
 */
const char* getTypeByInt(int type);
const char* getTypeByObj(Object obj);
void        objSet(Object self, Object key, Object value);
Object      objGet(Object self, Object key);
Object      objAdd(Object a, Object b);
Object      objSub(Object a, Object b);
Object      objMul(Object a, Object b);
Object      objDiv(Object a, Object b);
Object      objMod(Object a, Object b);
Object      objNeg(Object o) ;
int         objIn(Object key, Object collection);
int         objEquals(Object a, Object b);
int         objCmp(Object a, Object b);
Object*     nextPtr(Object iterator);
Object      iterNew(Object collections);
Object      objGet(Object self, Object k);
void        objSet(Object self, Object k, Object v);
char*       objToSz(Object obj);

Object      tmStr(Object obj);
int         isTrueObj(Object v);
int         tmIter(Object self, Object *k);
void        objDel(Object self, Object k);
Object      tmCall(int lineno, Object func, int args, ...);
Object      objAppend(Object a, Object b);
Object      tmGetGlobal(Object globals, Object key);
Object      tmTakeArg();
Object      objGetlocal(int fidx, int lidx);
Object      objGetstack(int fidx, int sidx);
Object      arrayToList(int n, ...);
TmFrame*    objGetframe(int fidx);
Object      tmGetfname(Object func);
void        tmSetattr(Object a, char* key, Object value);
void        defFunc(Object globals, Object a, Object(*func)());
void        defMethod(Object dict, Object name, Object(*func)());

// vm functions
Object callModuleFunction(char* mod, char* fnc);
void   regBuiltin(char* name, Object value);
void   regModFunc(Object mod, char* name, Object(*native)());
void   regBuiltinFunc(char* name, Object (*native)());
void   regModAttr(char* modName,char* attr, Object value);
int    objEqSz(Object str, const char* value);
void   tmRaise(char*fmt , ...);

// interp functions

typedef struct _FunctionDefine {
    Object fnc;
    int len;
} FunctionDefine;

#define  TM_PUSH(x) *(++top) = (x); if(top > tm_stack_end) tmRaise("stack overflow");
#define  TM_POP() *(top--)
#define  TM_TOP() (*top)
#define  GET_CONST(i) GET_DICT(tm->constants)->nodes[i].key
Object   callUnsafe(Object fnc);
Object   callFunction(Object func);
Object   tmEval(TmFrame*);
TmFrame* pushFrame(Object fnc);
void     popFrame();

// exception functions
void tmAssertType(Object o, int type, char* msg) ;
void tmAssertInt(double value, char* msg) ;
void pushException(TmFrame* f);
void traceback();
void tmRaise(char* fmt, ...);

// builtin functions
void      tmPrint(Object v);
void      tmPrintln(Object v);
Object    tmFormatVaList(char* fmt, va_list ap, int appendln);
Object    tmFormat(char*fmt, ...);
Object    tmType(Object o);
void      tmPrintf(char* fmt, ...);
/* avoid '\0' in char array, which will be regarded as end by c lang */
/* Chars     ObjectInfo(char*,Object,int); */
Object    tmLoad(char* fname); // load the content of a file.
Object    bf_load();
Object    bf_save(); // save(fname, content);
Object    bf_int();
Object    bf_float();
Object    bf_system();
Object    bf_print();
Object    blt_AddTypeMethod();
void      builtinsInit();
Object*   getBuiltin(char* key);


// macros
#define strequals(a, b) a == b || strcmp(a,b) == 0

#define GET_VAL(obj) (obj).value
#define GET_DATA(obj) (obj).value.data
#define GET_DATA_PROTO(obj) (obj).value.data->proto
#define GET_DICT(obj) GET_VAL(obj).dict
#define GET_MODULE(obj) GET_VAL(obj).mod
#define GET_MOD(obj) GET_VAL(obj).mod
#define DICT_LEN(obj)  GET_DICT(obj)->len
#define ptr_addr(ptr) (long) (ptr) / sizeof(char*)
#define GET_NUM(obj) (obj).value.dv


#define IS_NONE(obj)   TM_TYPE(obj) == TYPE_NONE
#define IS_LIST(obj)   TM_TYPE(obj) == TYPE_LIST
#define IS_FUNC(obj)   TM_TYPE(obj) == TYPE_FUNCTION
#define IS_DICT(o)     TM_TYPE(o)==TYPE_DICT
#define IS_STR(obj)    TM_TYPE(obj) == TYPE_STR
#define IS_NUM(obj)    TM_TYPE(obj) == TYPE_NUM
#define IS_DATA(obj)   TM_TYPE(obj) == TYPE_DATA
#define IS_NATIVE(obj) GET_FUNCTION(obj)->native != NULL

#define NOT_LIST(obj) TM_TYPE(obj) != TYPE_LIST
#define NOT_DICT(obj) TM_TYPE(obj) != TYPE_DICT
#define NOT_FUNC(obj) TM_TYPE(obj) != TYPE_FUNCTION
#define NOT_STR(obj)  TM_TYPE(obj) != TYPE_STR
#define NOT_NATIVE(obj) GET_FUNCTION(obj)->native == NULL



#define ASSERT_TYPE_WITH_INFO(obj, type, info) \
    if(TM_TYPE(obj)!=type){                    \
        tmRaise(info, obj);                    \
    }
/* for instruction read */

/* gcc process ++ from right to left */
#define READ_BYTE(s) *s++
#define READ_SHORT(s) ((*s) << 8 | *(s+1)); s+= 2;
/* #define next_short( s ) (((*s++) << 8) + *(s++)); */

/* for math */
#define max(a, b) (a) > (b) ? (a) : (b)

#if LIGHT_DEBUG_GC
    #define TRACE_GC(out) ;
#else
    #define TRACE_GC(out) puts(out);
#endif

#if LIGHT_DEBUG_GC
    #define CHECK_MEM_USAGE(msg) printf("%s, press enter to continue", msg);getchar();
#else
    #define CHECK_MEM_USAGE(msg) ;
#endif

#define GET_FUNCTION_FILE(fnc) GET_MODULE(GET_FUNCTION(fnc)->mod)->file
#define GET_FUNCTION_NAME(fnc) GET_FUNCTION(fnc)->name
#define GET_FUNCTION_GLOBALS(fnc) GET_MODULE(GET_FUNCTION(fnc)->mod)->globals

/* assert macro */
#define TM_ASSERT(cond, msg) if(!cond) {tmRaise( msg );}


#define DEBUG(msg) \
    printf("--DEBUG %s\n", msg);

#define DEBUG2(msg1, msg2) \
    printf("--DEBUG %s %s\n", msg1, msg2);
    
#endif

#define LOG_INFO printf