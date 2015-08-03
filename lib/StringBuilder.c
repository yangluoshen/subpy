#include "StringBuilder.h"

DataProto* getStringBuilderProto() {
	if(!stringBuilderProto.init) {
		initDataProto(&stringBuilderProto);
		stringBuilderProto.get = StringBuilderGet;
		stringBuilderProto.str = StringBuilderToStr;
		stringBuilderProto.dataSize = sizeof(StringBuilder);
		stringBuilderProto.free = StringBuilderDel;
	}
	return &stringBuilderProto;
}

StringBuilder* StringBuilderNew(){
	Object sb = StringBuilderObjNew();
	return (StringBuilder*)GET_DATA(sb);
}

Object StringBuilderObjNew() {
	Object data = dataNew(sizeof(StringBuilder));
	StringBuilder* sb = (StringBuilder*)GET_DATA(data);
	sb->len = 0;
	sb->cap = 10;
	sb->value = tmMalloc(sb->cap);
	sb->proto = getStringBuilderProto();
	return data;
}

void StringBuilderDel(StringBuilder* sb){
    tmFree((sb)->value, (sb)->cap);
    tmFree(sb, sizeof(StringBuilder));
}

void _sbcheck(StringBuilder* sb, int n){
    if(sb->len + n >= sb->cap){
        int ncap = sb->cap + n + sb->cap / 2;
        sb->value = tmRealloc(sb->value, sb->cap, ncap);
        sb->cap = ncap;
    }
}

void StringBuilderAppend(StringBuilder* sb, char c){
    _sbcheck(sb, 1);
    sb->value[sb->len] = c;
    sb->len++;
}

void StringBuilderAppendCharsWithLen(StringBuilder* sb, char* str, int len){
    _sbcheck(sb, len);
    memcpy(sb->value + sb->len, str, len);
    sb->len+=len;
}

void StringBuilderAppendChars(StringBuilder* sb, char* str){
    StringBuilderAppendCharsWithLen(sb, str, strlen(str));
}

void StringBuilderAppendObj(StringBuilder* sb, Object obj){
    obj = tmStr(obj);
    StringBuilderAppendCharsWithLen(sb, GET_STR(obj), GET_STR_LEN(obj));
}

void StringBuilderAppendInt(StringBuilder* sb, int value){
    static char s[20];
    sprintf(s, "%d", value);
    StringBuilderAppendChars(sb, s);
}

void StringBuilderAppendPointer( StringBuilder* sb, void* ptr){
    static char s[20];
    sprintf(s, "%p", ptr);
    StringBuilderAppendChars(sb, s);
}

void StringBuilderAppendDouble( StringBuilder* sb, double value){
    char s[20];
    if(fabs(value) < 0.0000001 ) {
        sprintf(s, "%ld", (long)value);
    }else{
        sprintf(s, "%lf", value);
    }
    StringBuilderAppendChars(sb, s);
}

void StringBuilderAppendChar(StringBuilder* sb, char c){
    StringBuilderAppend(sb, c);
}

Object StringBuilderToStr(StringBuilder*sb){
    Object str = newString0(sb->value, sb->len);
    return str;
}

char* StringBuilderToChars(StringBuilder* sb) {
	sb->value[sb->len] = '\0';
	return sb->value;
}

Object StringBuilderGet(StringBuilder* sb, Object key){
  if(objEqSz(key,"len")) return newNumber(sb->len);
  else if(objEqSz(key, "cap"))return newNumber(sb->cap);
  tmRaise("StringBuilderGet():no attribute %o", key);
  return NONE_OBJECT;
}
