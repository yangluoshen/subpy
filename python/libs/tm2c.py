from parse import *
import sys
from repl import *
from tokenize import *

tm_obj = "Object "
tm_const = "Object const_"

tm_pusharg = "tm_pusharg("
tm_call = "tm_call("

tm_num = "tm_number("
tm_str = "tm_string("
tm_get_glo = "tm_getglobal("
tm_define = "tm_define"

func_bool = "tm_bool"
func_add = "tm_add"
func_sub = "tm_sub"
func_mul = "tm_mul"
func_div = "tm_div"
func_mod = "tm_mod"
func_cmp = "tm_cmp"
func_not = "tm_not"
func_get = "tm_get"
func_set = "tm_set"
func_list = "tm_list"
func_method = "tm_method"

op_bool = [">", ">=", "<", "<+", "==", "!=", "and", "or", "in", "not"]

class AstNode:
    def __init__(self, type, first = None, second = None):
        self.type = type
        self.first = first
        self.second = second
        
class Scope:
    def __init__(self, prev):
        self.vars = []
        self.prev = prev
        self.temp = 0
        
class Env:
    def __init__(self, fname, prefix = None):
        self.consts = []
        self.scope = Scope(None)
        self.indent = 0
        self.fname = fname
        self.func_defines = []
        self.func_cnt = 0
        if prefix == None:
            self.prefix = fname
        else:
            self.prefix = prefix
    
    def newScope(self):
        scope = Scope(self.scope)
        self.scope = scope
        
    def exitScope(self, func_define):
        if self.scope != None:
            self.scope = self.scope.prev
        else:
            raise "fatal error"
        self.func_defines.append(func_define)
        self.func_cnt += 1
        
    def getCFuncName(self):
        return self.prefix + "F" + str(self.func_cnt)
            
    def getConst(self, val):
        if val not in self.consts:
            self.consts.append(val)
        return self.prefix+ "C" + str(self.consts.index(val))
        
    def getVarName(self, name):
        return self.prefix + "V" + name

    def getTempVarName(self):
        temp = new_temp(self)
        return self.prefix + "V" + temp

    # you must declare temp var in your scope
    def getTempPtrName(self):
        temp = new_temp(self)
        return self.prefix + "P" + temp
        
    def locals(self):
        if self.scope != None:
            return self.scope.vars
            
    def getGlobals(self):
        return self.prefix + "_g";
            
    def hasVar(self, name):
        scope = self.scope
        while scope != None:
            if name in scope.vars:
                return True
            scope = scope.prev
        return False
        
    def defVar(self, name):
        if self.hasVar(name):
            pass
        else:
            self.scope.vars.append(name)

def new_temp(env):
    c = env.scope.temp
    env.scope.temp += 1
    name = str(c)
    env.defVar(name)
    return name

def do_assign(item, env, right = None):
    left = item.first
    if istype(left, "list"):
        raise "multi-assignment not implemented"
    if right == None:
        right = do_item(item.second, env)
    if left.type == 'name':
        name = left.val
        if not env.hasVar(name):
            env.defVar(name)
        return env.getVarName(name) + "=" + right + ";"
    elif left.type == ",":
        return "// not implemented"
    elif left.type == "get":
        print("assign get")
        return sformat("%s(%s,%s,%s);", func_set, do_item(left.first, env), do_item(left.second, env), right)
    elif left.type == "attr":
        lfirst = Token("string", left.first.val)
        return sformat("%s(%s,%s,%s);", func_set, do_item(lfirst,env), do_item(left.second,env), right)

def do_const(item, env):
    val = item.val
    if val not in env.consts:
        env.consts.append(val)
    return env.getConst(val) # "const_" + str(env.consts.index(val))
    
def do_name(item, env):
    if env.hasVar(item.val):
        return env.getVarName(item.val)
    item = Token("string", item.val)
    return tm_get_glo + env.getGlobals() + "," + do_const(item, env) + ")"

def do_none(item, env):
    return "NONE_OBJECT"
    
def do_list(item, env):
    v = item.first
    cnt = 0
    if v == None:
        cnt = 0
        return sformat("%s(0)", func_list)
    newlist = []
    if istype(v, "list"):
        for item in v:
            code = do_item(item, env)
            newlist.append(code)
        cnt = len(newlist)
    else:
        raise
    if cnt == 0:
        return sformat("%s(0)", func_list)
    else:
        return sformat("%s(%s,%s)", func_list, cnt, ",".join(newlist)) 
    
def do_bool(item, env):
    if item.type in op_bool:
        return do_item(item, env)
    return func_bool + "(" + do_item(item, env) + ")"
    
def do_block(list, env, indent):
    lines = []
    for exp in list:
        if exp.type == "string": continue
        line = do_item(exp, env, indent + 2)
        if exp.type == "call": line += ";"
        lines.append(line)
    return lines
    
def do_if(item, env, indent):
    cond = do_bool(item.first, env)
    lines = do_block(item.second, env, indent)
    third = item.third
    head = sformat("if(%s) %s", cond, format_block(lines, indent))
    if third == None:
        pass
    elif gettype(third) == "list":
        return head + "  else" + format_block(do_block(third, env, 2),indent)
    else:
        # lines.append(do_item(third, env, indent))
        return head + "  else " + do_item(third, env, 2)
    return head

def do_for(item, env, indent):
    temp = env.getTempVarName()
    tempPtr = env.getTempPtrName()
    expr = item.first
    block = item.second
    iterator = do_item(expr.second, env, 0)
    _keyname = expr.first.val;
    env.defVar(_keyname)
    keyname = env.getVarName(_keyname)
    key = do_name(expr.first, env);
    init = sformat("%s = iter_new(%s);", temp, iterator)
    init += "\n" + "Object* " + tempPtr + ";";
    getNext = "%s = tm_next(%s);".format(tempPtr, temp)
    head = "while (%s != NULL)".format(tempPtr)
    head = getNext + "\n" + head
    lines = do_block(block, env, indent)
    keyAssignment = "%s = *%s;".format(keyname, tempPtr)
    lines.insert(0, keyAssignment)
    lines.append(getNext)
    return sformat("%s\n%s %s", init, head, format_block(lines, indent))

    
def do_while(item, env, indent):
    cond = do_bool(item.first, env)
    lines = do_block(item.second, env, indent)
    return sformat("while(%s) %s", cond, format_block(lines, indent))

def do_args(item, env):
    if item == None:
        return ""
    elif istype(item, "list"):
        args = "";
        for i in item:
            args += ", " + do_item(i, env)
        return args
    return "," + do_item(item, env)
    
def do_call(item, env):
    name = do_item(item.first, env)
    if item.second == None:
        n = 0
    elif istype(item.second, "list"):
        n = len(item.second)
    else:
        n = 1
    args = do_args(item.second, env)
    return tm_call + name + "," + str(n) + args + ")"
    
def format_block(lines, indent):
    text = "{\n";
    for line in lines: 
        if line != None:
            text += line + "\n"
    # return text + indent * " " + "}\n"
    return text + "}\n"
    
def do_getargs(list, env, indent):
    r = []
    for item in list:
        if item.type == "varg":
            node = AstNode("=", item.first, item.second)
            code = do_assign(node, env)
            r.append(code)
        line = do_assign(item, env, "tm_getarg()")
        r.append(line)
    return r
    
def do_def(item, env, indent=0, obj=None):
    env.newScope()
    name = item.first.val
    args = do_getargs(item.second, env, 2)
    lines = args + do_block(item.third, env, 0)
    cname = env.getCFuncName()
    if obj!=None:
        obj.name = cname
        obj.constname = env.getConst(name)
    locs = env.locals()
    vars = []
    for var in locs:
        vars.append("Object " + env.getVarName(var) + ";")
    func_define = "Object " + cname + "() " + format_block(vars+lines, 0)
    env.exitScope(func_define)
    return sformat("%s(%s, %s, %s);", tm_define, env.getGlobals(), env.getConst(name), cname)
    
def do_return(item, env, indent=0):
    return "return " + do_item(item.first, env, 0) + ";"
    
def do_class(item, env, indent=0):
    class_define = do_assign(item, env, "dict_new()")
    class_name = do_name(item.first, env)
    obj = newobj()
    lines = []
    for class_item in item.second:
        type = class_item.type
        if type == "pass": continue
        assert type == "def"
        do_def(class_item, env, indent+2, obj)
        lines.append(sformat("%s(%s,%s,%s);", func_method, class_name, obj.constname, obj.name))
    text = class_define + "\n"
    for line in lines:
        text += line + "\n"
    return text
    
def do_op(item, env, func):
    return func + "(" + do_item(item.first, env) + "," + do_item(item.second, env) + ")";

def do_or(item, env):
    return "(" + do_item(item.first, env) + "||" + do_item(item.second, env) + ")"
    
def do_and(item, env):
    return "(" + do_item(item.first, env) + "&&" + do_item(item.second, env) + ")"
    
def do_mul(item, env):
    return do_op(item, env, func_mul)
    
def do_div(item, env):
    return do_op(item, env, func_div)
    
def do_mod(item, env):
    return do_op(item, env, func_mod)
    
def do_add(item, env):
    return do_op(item, env, func_add)
    
def do_sub(item, env):
    return do_op(item, env, func_sub)
    
def do_lt(item, env):
    return do_op(item, env, func_cmp) + "<0"
    
def do_gt(item, env):
    return do_op(item, env, func_cmp) + ">0"
    
def do_le(item, env):
    return do_op(item, env, func_cmp) + "<=0"
    
def do_ge(item, env):
    return do_op(item, env, func_cmp) + ">=0"

def do_ne(item, env):
    return "(" + do_op(item, env, "!tm_equals") + ")"
    
def do_eq(item, env):
    return do_op(item, env, "tm_equals")
    
def do_not(item, env):
    return sformat("%s(%s)", func_not, do_item(item, env))
    
def do_get(item, env):
    return do_op(item, env, func_get)
    
def do_attr(item, env):
    second = item.second
    item.second = Token("string", second.val)
    item.second.pos = second.pos
    return do_op(item, env, func_get)
    
def do_inplace_op(item, env, op):
    item2 = AstNode(op, item.first, item.second)
    tk = AstNode("=", item.first, item2)
    return do_assign(tk, env)
    
def do_inplace_add(item, env):
    return do_inplace_op(item, env, "+")
    
def do_inplace_sub(item, env):
    return do_inplace_op(item, env, "-")
    
def do_inplace_mul(item, env):
    return do_inplace_op(item, env, "*")
    
def do_inplace_div(item, env):
    return do_inplace_op(item, env, "/")
    
def do_inplace_mod(item, env):
    return do_inplace_op(item, env, '%')


def do_import(item, env):
    mod = do_const(item.first, env)
    return "tmImport(%s, %s);".format(env.getGlobals(), mod)

def do_from(item, env):
    mod = do_const(item.first, env)
    return "tmImportAll(%s, %s);".format(env.getGlobals(), mod)    

_handlers = {
    "if": do_if,
    "while": do_while,
    "def": do_def,
    "return": do_return,
    "class": do_class,
    "for": do_for
}

_handlers2 = {
    "number": do_const,
    "string": do_const,
    "name": do_name,
    "None": do_none,
    "list": do_list,
    "=": do_assign,
    "+": do_add,
    "-": do_sub,
    "*": do_mul,
    "/": do_div,
    "%": do_mod,
    "<": do_lt,
    ">": do_gt,
    ">=": do_ge,
    "<=": do_le,
    "!=": do_ne,
    "==": do_eq,
    "+=": do_inplace_add,
    "-=": do_inplace_sub,
    "*=": do_inplace_mul,
    "/=": do_inplace_div,
    "%=": do_inplace_mod,
    "or": do_or,
    "and": do_and,
    "not": do_not,
    "call": do_call,
    "get": do_get,
    "attr": do_attr,
    "import": do_import,
    "from": do_from
}

def do_item(item, env, indent = 0):
    func = None
    env.token = item
    if item == None:
        return ""
    if not hasattr(item, "type"):
        replPrint(item, 1, 4)
        raise
    if item.type in _handlers:
        func = _handlers[item.type]
        code = func(item, env, indent)
    elif item.type in _handlers2:
        func = _handlers2[item.type]
        code = func(item, env)
    if func != None:
        # if indent > 0: code = " " * indent + code
        return code

# env
# consts, globals, scopes.

def do_program(tree, env, indent):
    try:
        return do_block(tree, env, 0)
    except Exception as e:
        # replPrint(tree, 2, 5)
        # printAst(tree)
        compile_error(env.fname, env.src, env.token, e)
        
def tm2c(fname, src, prefix=None):
    tree = parse(src)
    # replPrint(tree, 0, 5)
    #env = {"vars": [], "consts": [], "funcs": [], "globals":[]}
    fname = fname.replace(".", "_")
    env = Env(fname, prefix)
    env.src = src

    lines = do_program(tree, env, 0)
    _lines = []
    # assign constants
    # _lines.append("  " + env.getGlobals() + "=dict_new();");
    _lines.append(env.getGlobals() + "=dict_new();")
    for const in env.consts:
        h = env.getConst(const) + "="
        if gettype(const) == "number":
            body = tm_num + str(const) + ");"
        elif gettype(const) == "string":
            body = tm_str + '"' + escape(str(const)) + '");'
        # _lines.append("  " + h + body)
        _lines.append(h+body)
        
    lines = _lines + lines
        
    head = '''#include "include/tm.h"
#include "tm.c"
'''
    head += "/* DEFINE START */\n"
    # define constants
    for const in env.consts:
        head += "Object " + env.getConst(const) + ";\n";
    head += "Object " + env.getGlobals() + ";\n"
    # do vars
    for var in env.locals():
        head += tm_obj + env.getVarName(var) + ";\n"
    head += "/* DEFINE END */\n"
    
    # function define.
    for func_define in env.func_defines:
        head += func_define + "\n"
    
    # global 
    head += "void " + env.prefix + "_main(){\n"
    code =  head + "\n".join(lines)+"\n}\n"
    code += sformat("\nint main(int argc, char* argv[]) {\nrun_py_func(argc, argv, %s_main);\n}\n", env.prefix)
    return code
    
if __name__ == "__main__":
    name = sys.argv[1]
    # path = "../test/tm2c/" + name
    path = name
    text = load(path)
    code = tm2c(name, text, "a")
    print(code)