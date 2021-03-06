#include "include/tm.h"

void argStart() {
    tm->arguments = tm->internalArgStack;
    tm->arg_loaded = 0;
    tm->arg_cnt = 0;
}

void argPush(Object obj) {
    tm->arguments = tm->internalArgStack;
    tm->arg_loaded = 0;
    tm->arguments[tm->arg_cnt] = obj;
    tm->arg_cnt += 1;
    if(tm->arg_cnt > MAX_ARG_COUNT) {
        tmRaise("argPush(): too many arguments! over %d", MAX_ARG_COUNT);
    }
}

void argSetArguments(Object* first, int len) {
    tm->arguments = first;
    tm->arg_cnt = len;
    tm->arg_loaded = 0;
}

void argInsert(Object first) {
    int i;
    for (i = tm->arg_cnt; i > 0; i--) {
        tm->arguments[i] = tm->arguments[i-1];
    }
    #if 0
        tmPrintf("Self = %o\n", first);
    #endif
    tm->arguments[0] = first;
    tm->arg_cnt += 1;
}

void _resolveMethodSelf(TmFunction *fnc) {
    if(IS_NONE(fnc->self)) {
        return;
    }
    argInsert(fnc->self);
}

Object getArgFromVM0(const char* fnc) {
    if (tm->arg_loaded >= tm->arg_cnt)
        tmRaise("%s :no argument! total %d, current %d",
                fnc, tm->arg_cnt, tm->arg_loaded);
    tm->arg_loaded += 1;
    return tm->arguments[tm->arg_loaded - 1];
}


int hasArg() {
    return tm->arg_loaded < tm->arg_cnt;
}

Object getArgFromVM2(int type, const char* fnc) {
    Object value = getArgFromVM0(fnc);
    if (type == -1)
        return value;
    if (type != value.type)
        tmRaise("%s:expect type %s, but see %s", fnc, getTypeByInt(type),
                getTypeByObj(value));
    return value;
}


Object argTakeStrObj(const char* fnc) {
    Object value = getArgFromVM0(fnc);
    if (NOT_STR(value)) {
        tmRaise("%s: expect string but see %s", fnc, getTypeByObj(value));
    }
    return value;
}

String* argTakeStrPtr(const char* fnc) {
    Object value = getArgFromVM0(fnc);
    if (NOT_STR(value)) {
        tmRaise("%s: expect string but see %s", fnc, getTypeByObj(value));
    }
    return value.value.str;
}

char* argTakeSz(const char* fnc) {
    Object value = getArgFromVM0(fnc);
    if (NOT_STR(value)) {
        tmRaise("%s: expect string but see %s", fnc, getTypeByObj(value));
    }
    return GET_STR(value);
}

Object argTakeFuncObj(const char* fnc) {
    Object value = getArgFromVM0(fnc);
    if (NOT_FUNC(value)) {
        tmRaise("%s: expect function but see %s", fnc, getTypeByObj(value));
    }
    return value;
}

Object argTakeListObj(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (NOT_LIST(v)) {
        tmRaise("%s: expect list but see %s", fnc, getTypeByObj(v));
    }
    return v;
}

Object argTakeDictObj(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (NOT_DICT(v)) {
        tmRaise("%s: expect dict but see %s", fnc, getTypeByObj(v));
    }
    return v;
}

TmList* argTakeListPtr(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (NOT_LIST(v)) {
        tmRaise("%s: expect list but see %s", fnc, getTypeByObj(v));
    }
    return GET_LIST(v);
}

int argTakeInt(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (TM_TYPE(v) != TYPE_NUM) {
        tmRaise("%s: expect number but see %s", fnc, getTypeByObj(v));
    }
    return (int) GET_NUM(v);
}

double argTakeDouble(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (TM_TYPE(v) != TYPE_NUM) {
        tmRaise("%s: expect number but see %s", fnc, getTypeByObj(v));
    }
    return GET_NUM(v);
}

Object argTakeObj(const char* fnc) {
    return getArgFromVM0(fnc);
}

Object argTakeDataObj(const char* fnc) {
    Object v = getArgFromVM0(fnc);
    if (TM_TYPE(v) != TYPE_DATA) {
        tmRaise("%s: expect data but see %s", fnc, getTypeByObj(v));
    }
    return v;
}
int getArgsCount() {
    return tm->arg_cnt;
}
