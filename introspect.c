#include <stdio.h>
#include <stdlib.h> // malloc()
#include <ctype.h> // isdigit(), isalpha()
#include <string.h> // strchr()
#include <sysexits.h> // exit(EXIT_SUCCESS)
#include <stdbool.h>
#include <stdio.h>
#include "debuginfo.h"

#include <glib.h>
GHashTable *gtbl;

//#define PTRACE_ON
#define LIBVMI_ON

#ifdef PTRACE_ON
#include <sys/ptrace.h> // ptrace()
#endif

#ifdef PTRACE_ON
pid_t pid;
#define SIZE 8
#endif


#ifdef LIBVMI_ON
#include <libvmi/libvmi.h>
#endif

#ifdef LIBVMI_ON
vmi_instance_t vmi;
char* vmname;
#define SIZE 8
#endif


#define SYMBOL_MAX_LEN 100

enum {
  INT,
  STR,
  SYM,
  PRIM,
  LIST,
  FUNC,
  ENV,
  GVAR,
  RES,
  RPAREN,
  TTRUE,
  NIL,
  BREAK,
};

struct obj;

// 基本関数用の関数ポインタ
typedef struct obj *primitive(struct obj *env, struct obj *args);

typedef struct obj {
  int type;
  union {
    // 数値
    long value;
    // 文字列
    char* string;
    // シンボル
    char* symbol;
    // 基本関数
    primitive* prim;
    // 関数
    struct {
      struct obj *params;
      struct obj *body;
      struct obj *env;
    };
    // リスト
    struct {
      struct obj* car;
      struct obj* cdr;
    };
    // 環境
    struct {
      struct obj* vars;
      struct obj* up;
    };
    // グローバル変数及び結果
    struct {
      int tidx;
      long data;
    };
  };
} obj;

obj* Rparen = &(obj){.type=RPAREN};
obj* True   = &(obj){.type=TTRUE};
obj* Nil    = &(obj){.type=NIL};
obj* Break  = &(obj){.type=BREAK};

// envで一括管理すれば消せそう
obj *symbol_table;

#define TYPE_ON  1
#define TYPE_OFF 0

obj* get_gvar(obj* o);
void print(obj* o, int tflag);
void print_gvar(obj* o, int tflag);

//================
// constructor
//================

// 数値を格納するobjを定義する
obj* make_int(long value){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = INT;
  tmp->value = value;
  return tmp;
}

// 文字列を格納するobjを定義する
obj* make_str(char* string){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = STR;
  tmp->string = string;
  return tmp;
}


// シンボルを格納するobjを定義する
obj* make_sym(char* symbol){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->symbol = (char*)malloc(strlen(symbol) + 1);
  tmp->type = SYM;
  strcpy(tmp->symbol, symbol);
  return tmp;
}

// リストを格納するobjを定義する
obj* make_list(obj* car, obj* cdr){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = LIST;
  tmp->car = car;
  tmp->cdr = cdr;
  return tmp;
}

// 基本関数を格納するobjを定義する
obj* make_prim(primitive* prim){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = PRIM;
  tmp->prim = prim;
  return tmp;
}

obj* make_function(obj *env, obj *params, obj *body) {
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = FUNC;
  tmp->params = params;
  tmp->body = body;
  tmp->env = env;
  return tmp;
}

// 環境を表すobjを定義する
obj* make_env(obj* vars, obj* up){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = ENV;
  tmp->vars = vars;
  tmp->up = up;
  return tmp;
}

// 対象のグローバル変数情報を格納するobjを定義する
obj* make_gvar(int tidx, long data){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = GVAR;
  tmp->tidx = tidx;
  tmp->data = data;
  return tmp;
}

// 対象のグローバル変数情報の取得結果を格納するobjを定義する
obj* make_res(int tidx, long data){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = RES;
  tmp->tidx = tidx;
  tmp->data = data;
  return tmp;
}

// Returns ((x . y) . a)
obj* acons(obj *x, obj *y, obj *a) {
    obj* cell = make_list(x, y);
    return make_list(cell, a);
}


//================
// read
//================

obj* read_main(FILE *stream);

// １文字先読みする
// ungetc()でストリームに１文字返す
int peek(FILE *stream) {
  int c = getc(stream);
  ungetc(c, stream);
  return c;
}

// リストを反転する
obj* reverse(obj* o){
  obj* ret = Nil;
  while(o != Nil){
    obj* head = o;
    o = o->cdr;
    head->cdr = ret;
    ret = head;
  }
  return ret;
}

// リストを読み込む
// 入力に左括弧が見つかると呼ばれる
// 右括弧が見つかると終了
obj* read_list(FILE *stream){
  obj* head = Nil;
  for(;;){
    obj* tmp = read_main(stream);
    if(!tmp){
      printf("[read_list] error \n");
      return NULL;
    }
    else if(tmp == Rparen){
      return reverse(head); //リストを反転して返す
    }
    head = make_list(tmp, head); // 先頭に挿入する
  }
}

// 数値を読み込む
// 後続文字が数字である限り数値を計算する
obj* read_num(int value, FILE* stream){
  while(isdigit(peek(stream))){
    value = value * 10 + getc(stdin) - '0';
  }
  return make_int((long)value);
}

// すでに同じ名前のシンボルがあるか確認する
obj* check_symbol(char* symbol){
  for(obj *o = symbol_table; o != Nil; o = o->cdr){
    if(strcmp(symbol, o->car->symbol) == 0){
      return o->car;
    }
  }
  obj* sym = make_sym(symbol);
  symbol_table = make_list(sym, symbol_table);
  return sym;
}

const char symbol_chars[] = ".~!@#$%^&*-_=+:/?<>";

// シンボルを読み込む
// 後続文字がある限り読み込みを続ける
obj* read_symbol(char c, FILE* stream){
  char symbol[SYMBOL_MAX_LEN + 1];
  symbol[0] = c;
  int len = 1;
  while(isalnum(peek(stream)) || strchr(symbol_chars, peek(stream))){
    if(SYMBOL_MAX_LEN <= len){
      printf("[read_symbol] too long symbol name");
      return NULL;
    }
    symbol[len++] = getc(stdin);
  }
  symbol[len] = '\0';
  
  return check_symbol(symbol);
}

obj* read_string(void){
  char str[SYMBOL_MAX_LEN + 1];
  for(int i=0; i<SYMBOL_MAX_LEN+1; i++){
    str[i] = getc(stdin);
    if(str[i] == '"'){
      str[i] = '\0';
      return make_str(str);
    }
  }

  printf("[%s] error", __func__);
  return NULL;
}

void quit(void){
#ifdef PTRACE_ON
  int ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);

  if(ret != 0){
    printf("[main] DETACH error\n");
    exit(EXIT_FAILURE);
  }
#endif

#ifdef LIBVMI_ON
  vmi_resume_vm(vmi);
  vmi_destroy(vmi);
#endif
  
  exit(EXIT_SUCCESS);
}

void init(void){
  int ret;
  
#ifdef PTRACE_ON
  ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);

  if(ret != 0){
    printf("[main] ATTACH error\n");
    exit(EXIT_FAILURE);
  }
#endif

#ifdef LIBVMI_ON
  ret = vmi_init_complete(&vmi, "u1", VMI_INIT_DOMAINNAME, NULL,
			  VMI_CONFIG_GLOBAL_FILE_ENTRY, NULL, NULL);
  if(ret == VMI_FAILURE){
    printf("[init] Failed to init LibVMI library.\n");
    exit(EXIT_FAILURE);
  }

  ret = vmi_pause_vm(vmi);
  
  if (ret != VMI_SUCCESS) {
    printf("[init] Failed to pause VM\n");
    quit();
  } 
#endif
}

// 文字列の前後の空白を削除する
int trim(char *s) {
    int i;
    int count = 0;

    /* 空ポインタか? */
    if ( s == NULL ) { /* yes */
      return -1;
    }
    
    /* 文字列長を取得する */
    i = strlen(s);
    
    /* 末尾から順に空白でない位置を探す */
    while ( --i >= 0 && s[i] == ' ' ) count++;
    
    /* 終端ナル文字を付加する */
    s[i+1] = '\0';
    
    /* 先頭から順に空白でない位置を探す */
    i = 0;
    while ( s[i] != '\0' && s[i] == ' ' ) i++;
    strcpy(s, &s[i]);
    
    return i + count;
}


obj* read_file(void){
  char filename[256];
  gets(filename);

  if(trim(filename) < 0){
    printf("[%s] trim error", __func__);
    return NULL;
  }

  FILE *fp = fopen(filename, "r");
  if(fp == NULL){
    printf("[%s] fopen error", __func__);
    return NULL;
  }
    
  //obj* f = read_main(fp);
  
  //fclose(fp);
  
  return read_main(fp);
}

obj* read_command(void){
  int c2 = getc(stdin);
  if(c2 == 'q'){
    quit();
  }
  else if(c2 == 'l'){
    return read_file();
  }
  return make_sym(" ");
}

obj* read_s(FILE *stream){
  for(;;){
    int c = getc(stream);
    if(c == ' ' || c == '\n' || c == '\r' || c == '\t'){
      continue;
    }
    else if(c == ':'){
      return read_command();
    }
    else{
      ungetc(c, stdin);
      return read_main(stream);
    }
  }
}

// 読み込み
// 空白は飛ばす
obj* read_main(FILE *stream){
  for(;;){
    int c = getc(stream);
    if(c == ' ' || c == '\n' || c == '\r' || c == '\t'){
      continue;
    }
    else if(c == ':'){
      return read_command();
    }
    else if(c == '('){
      return read_list(stream);
    }
    else if(c == ')'){
      return Rparen;
    }
    else if(c == '"'){
      return read_string();
    }
    else if(isdigit(c)){
      return read_num(c - '0', stream);
    }
    else if (isalpha(c) || strchr(symbol_chars, c)){
      return read_symbol(c, stream);
    }
    else{
      printf("[read] error");
      return NULL;
    }
  }
}




//================
// eval
//================

obj* eval(obj* env, obj* o);

// 環境を調べて変数を検索する
// 未定義ならNULLを返す
obj* lookup(obj* env, obj* sym){
  for(obj* e = env; e != Nil; e = e->up){
    for(obj* list = e->vars; list != Nil; list = list->cdr){
      obj* tmp = list->car;
      if(sym == tmp->car){
	return tmp;
      }
    }
  }
  return Nil;
}

// 環境を新規作成し返す
obj* push_env(obj *env, obj *vars, obj *vals) {
    obj* map = Nil;
    obj* sym = Nil;
    obj* val = Nil;
    for(; vars->type == LIST; vars = vars->cdr, vals = vals->cdr){
      if (vals->type != LIST){
	printf("[%s] Cannot apply function: number of argument does not match", __func__);
      }
      sym = vars->car;
      val = vals->car;
      map = acons(sym, val, map);
    }
    if (vars != Nil)
      map = acons(vars, vals, map);
    return make_env(map, env);
}

// listの要素をすべて評価する
obj* progn(obj *env, obj *list) {
  obj* r = Nil;
  for (obj* lp = list; lp != Nil; lp = lp->cdr) {
    r = lp->car;
    r = eval(env, r);
  }
  return r;
}

obj* eval_list(obj* env, obj* list){
  if(list->type != LIST){
    return eval(env, list);
  }
  obj* head = Nil;
  obj* o = Nil;
  obj* result = Nil;  
  for(obj* lp = list; lp != Nil; lp = lp->cdr){
    o = lp->car;
    result = eval(env, o);
    head = make_list(result, head);
  }
  return reverse(head);
}

obj* apply_func(obj *fn, obj *args) {
    obj* params = fn->params;
    obj* newenv = push_env(fn->env, params, args);
    obj* body   = fn->body;
    return progn(newenv, body);
}

obj* apply(obj* env, obj* fn, obj* args){
  if(args->type != LIST && args != Nil){
    printf("[apply] obj->type error");
    return Nil;
  }
  if(fn->type == PRIM){
    return fn->prim(env, args);
  }
  else if(fn->type == FUNC){
    obj* eargs = eval_list(env, args);
    return apply_func(fn, eargs);
  }
  return Nil;
}

obj* eval(obj* env, obj* o){
  if(!o){
    printf("[eval] no obj");
    return NULL;
  }
  
  switch(o->type){
  case INT:
  case STR:
  case PRIM:
  case FUNC:
  case GVAR:
  case RES:
  case TTRUE:
  case NIL:
  case BREAK:
    return o;
  case SYM:{
    obj* list_sym = lookup(env, o);
    if(list_sym == Nil){
      int* gidx = g_hash_table_lookup(gtbl, o->symbol);
      if(gidx){
	return get_gvar(make_gvar(gvars[*gidx].tidx, gvars[*gidx].addr));
      }
      printf("[eval] no symbol");
      return Nil;
    }
    else{
      return list_sym->cdr;
    }
  }
  case LIST:{
    obj* fn = eval(env, o->car);
    obj* args = o->cdr;
    if(fn->type != PRIM && fn->type != FUNC){
      printf("[eval] fn->type error");
      return Nil;
    }
    return apply(env, fn, args);
  }
  default:
    printf("[eval] obj->type error");
    return Nil;
  }
}




//================
// introspect
//================

long read_mem(int size, long addr){
#ifdef PTRACE_ON
  return ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
#endif

#ifdef LIBVMI_ON
  long data;
  size_t data_size;
  int ret = vmi_read_va(vmi, addr, 0, size, &data, &data_size);

  if (ret == VMI_FAILURE) {
    printf("Failed to read memory\n");
    quit();
  }
  return data;
#endif
  
}

obj* get_base(obj* o){
  return make_res(o->tidx, read_mem(SIZE, o->data));
}

obj* get_pointer(obj* o){
  return make_res(o->tidx, read_mem(SIZE, o->data));
}

obj* get_array(obj* o){
  return make_res(o->tidx, o->data);
}

obj* get_struct(obj* o){
  return make_res(o->tidx, o->data);
}

obj* get_union(obj* o){
  return make_res(o->tidx, o->data);
}

obj* get_enumeration(obj* o){
  return make_res(o->tidx, read_mem(SIZE, o->data));
}

obj* get_gvar(obj* o){
  int tidx = o->tidx;
  if(types[tidx].kind == base_type){
    return get_base(o);
  }
  else if(types[tidx].kind == pointer_type){
    return get_pointer(o);
  }
  else if(types[tidx].kind == array_type){
    return get_array(o);
  }
  else if(types[tidx].kind == structure_type){
    return get_struct(o);
  }
  else if(types[tidx].kind == union_type){
    return get_union(o);
  }
  else if(types[tidx].kind == enumeration){
    return get_enumeration(o);
  }
  else{
    printf("[get_gvar] not defined");
    return NULL;
  }
}

int search_memb(char* memb, int tidx){
  for(int i=0;i<types[tidx].memnum;i++){
    if(strcmp(memb, types[tidx].mem[i].name) == 0){
      return i;
    }
  }
  return -1;
}






//======================================================================
// primitive
//======================================================================

void add_variable(obj* env, obj* sym, obj* val);

int length(obj *list) {
    int len = 0;
    for(; list->type == LIST; list = list->cdr){
      len++;
    }
    return list ? len : -1;
}

// (+ x y)
obj* prim_add(obj* env, obj* list){
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    printf("[%s] malformed +", __func__);
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;

  if(x->type == INT && y->type == INT)
    return make_int(x->value + y->value);
  else if((x->type == GVAR || x->type == RES) &&
	  y->type == INT){
    if(types[x->tidx].kind == pointer_type)
      return make_res(x->tidx, x->data + y->value);
    else
      return make_res(0, x->data + y->value);
  }
  else if(x->type == INT &&
	  (y->type == GVAR || y->type == RES)){
    if(types[y->tidx].kind == pointer_type)
      return make_res(y->tidx, x->value - y->data);
    else
      return make_res(0, x->value + y->data);
  }
  else if((x->type == GVAR || x->type == RES) &&
	  (y->type == GVAR || y->type == RES)){
    if(types[x->tidx].kind == structure_type &&
       types[y->tidx].kind == structure_type){
      return make_res(1, x->data + y->data);
    }
    else if(types[x->tidx].kind == pointer_type &&
	    types[y->tidx].kind == pointer_type ){
      return make_res(1, x->data + y->data);
    }
    else if(types[x->tidx].kind == pointer_type &&
	    types[y->tidx].kind != pointer_type){
      return make_res(x->tidx, x->data + y->data);      
    }
    else if(types[x->tidx].kind != pointer_type &&
	    types[y->tidx].kind == pointer_type){
      return make_res(y->tidx, x->data + y->data);      
    }
    else{
      return make_res(0, x->data + y->data);
    }
  }
  printf("[%s] + takes only numbers", __func__);  
  return Nil;
}

// (- 1 2)
obj *prim_sub(obj *env, obj *list) {
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    printf("[%s] malformed -", __func__);
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;
  
  if(x->type == INT && y->type == INT)
    return make_int(x->value - y->value);
  else if((x->type == GVAR || x->type == RES) &&
	  y->type == INT){
    if(types[x->tidx].kind == pointer_type)
      return make_res(x->tidx, x->data - y->value);
    else{
      return make_res(0, x->data - y->value);
    }
  }
  else if(x->type == INT &&
	  (y->type == GVAR || y->type == RES)){
    if(types[y->tidx].kind == pointer_type){
      return make_res(y->tidx, x->value - y->data);
    }
    else{
      return make_res(0, x->value - y->data);
    }
  }
  else if((x->type == GVAR || x->type == RES) &&
	  (y->type == GVAR || y->type == RES) ){
    if(types[x->tidx].kind == structure_type &&
       types[y->tidx].kind == structure_type){
      return make_res(1, x->data - y->data);
    }
    else if(types[x->tidx].kind == pointer_type &&
	    types[y->tidx].kind == pointer_type ){
      return make_res(1, x->data - y->data);
    }
    else if(types[x->tidx].kind == pointer_type &&
	    types[y->tidx].kind != pointer_type){
      return make_res(x->tidx, x->data - y->data);      
    }
    else if(types[x->tidx].kind != pointer_type &&
	    types[y->tidx].kind == pointer_type){
      return make_res(y->tidx, x->data - y->data);      
    }
    else{
      return make_res(0, x->data - y->data);
    }
  }
  
  
  printf("[%s] + takes only numbers", __func__);  
  return Nil;
}

// (< <value> <value>)
obj* prim_lt(obj *env, obj *list) {
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    printf("[%s] malformed <", __func__);
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;
  if(x->type != INT || y->type != INT){
    printf("[prim_lt] < takes only numbers");
  }
  return x->value < y->value ? True : Nil;
}

// (> <value> <value>)
obj* prim_gt(obj *env, obj *list) {
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    printf("[prim_gt] malformed >");
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;
  if(x->type != INT || y->type != INT){
    printf("[prim_gt] < takes only numbers");
  }
  return x->value > y->value ? True : Nil;
}

// (= <value> <value>)
obj* prim_eq(obj *env, obj *list) {
  if (length(list) != 2){
    printf("[prim_num_eq] Malformed =");
    return Nil;
  }
  obj* values = eval_list(env, list);
  obj* x = values->car;
  obj* y = values->cdr->car;

  if (x->type == INT && y->type == INT){
    return x->value == y->value ? True : Nil;
  }
  else if((x->type == GVAR || x->type == RES) && (y->type == GVAR || y->type == RES)){
    return x->data == y->data ? True : Nil;
  }
  else if(x->type == INT && (y->type == GVAR || y->type == RES)){
    return x->value == y->data ? True : Nil;
  }
  else if((x->type == GVAR || x->type == RES) && y->type == INT){
    return x->data == y->value ? True : Nil;
  }
  else{
    printf("[prim_num_eq] = only takes numbers");
    return Nil;
  }
}

// (!= <value> <value>)
obj* prim_not_eq(obj *env, obj *list) {
  if (length(list) != 2){
    printf("[prim_num_eq] Malformed =");
    return Nil;
  }
  obj* values = eval_list(env, list);
  obj* x = values->car;
  obj* y = values->cdr->car;

  if (x->type == INT && y->type == INT){
    return x->value != y->value ? True : Nil;
  }
  else if((x->type == GVAR || x->type == RES) && (y->type == GVAR || y->type == RES)){
    return x->data != y->data ? True : Nil;
  }
  else if(x->type == INT && (y->type == GVAR || y->type == RES)){
    return x->value != y->data ? True : Nil;
  }
  else if((x->type == GVAR || x->type == RES) && y->type == INT){
    return x->data != y->value ? True : Nil;
  }
  else{
    printf("[prim_num_eq] = only takes numbers");
    return Nil;
  }
}

// (if expr expr expr ...)
obj* prim_if(obj *env, obj *list) {
  if (length(list) < 2){
    printf("[prim_if] Malformed if");
    return Nil;
  }
  obj* cond = list->car;
  cond = eval(env, cond);
  if (cond != Nil) {
    obj* then = list->cdr->car;
    return eval(env, then);
  }
  obj* els = list->cdr->cdr;
  return els == Nil ? Nil : progn(env, els);
}

// リストがBreakを含むか判定
int break_in(obj* o){
  if(o->type != LIST){
    printf("[get_end] not list");
    return 0;
  }
  while(o != Nil){
    if(o->car == Break){
      return 1;
    }
    o = o->cdr;
  }
  return 0;
}

// (while cond expr ...)
obj* prim_while(obj *env, obj *list) {
  if (length(list) < 2){
    printf("[prim_while] Malformed while");
    return Nil;
  }
  obj* cond = list->car;
  obj* exprs = Nil;
  while (eval(env, cond) != Nil) {
    exprs = list->cdr;
    obj* ret = eval_list(env, exprs);
    if(break_in(ret)){
      break;
    }
  }
  return Nil;
}

// (define <symbol> expr)
obj* prim_define(obj* env, obj* list){
  if (length(list) != 2 || list->car->type != SYM){
    printf("[prim_define] Malformed define");
    return Nil;
  }
  
  if(list->car->symbol[0] != '$'){
    printf("[prim_define] initial of variable takes only $");
    return Nil;
  };
  
  obj* sym = list->car;
  obj* value = eval(env, list->cdr->car);
  add_variable(env, sym, value);
  return value;
}

// (set <symbol> expr)
obj* prim_set(obj* env, obj* list){
  if (length(list) != 2 || list->car->type != SYM){
    printf("[prim_define] Malformed define");
    return Nil;
  }
  
  if(list->car->symbol[0] != '$'){
    printf("[prim_define] initial of variable takes only $");
    return Nil;
  };
  
  obj* sym = list->car;
  obj* value = eval(env, list->cdr->car);
  add_variable(env, sym, value);
  return value;
}

// (deref <pointer var>)
obj* prim_deref(obj* env, obj* list){
  obj* o = eval(env, list->car);
  
  if(!o){
    printf("[%s] no pointer var", __func__);
    return Nil;
  }
  
  int tidx = o->tidx;
  if(types[tidx].kind != pointer_type){
    printf("[%s] not pointer", __func__);
    return Nil;
  }
  
  return make_res(types[tidx].saki, o->data);
}

obj* get_member_direct(obj* stru, obj* memb){
  int tidx = stru->tidx;

  //指定の構造体が指定のメンバを持つか検索
  int i = search_memb(memb->symbol, tidx);
  if(i<0){
    printf("[%s] no member", __func__);
    return Nil;
  }

  int membtidx = types[tidx].mem[i].tidx;
  long membaddr = stru->data + types[tidx].mem[i].offset;

  return get_gvar(make_gvar(membtidx, membaddr));
}

obj* get_member_indirect(obj* poin, obj* memb){
  int tidx = types[poin->tidx].saki;
  //long data = read_mem(SIZE, poin->data);
  long data = poin->data;

  return get_member_direct(make_gvar(tidx, data), memb);
}

// (. <struct> <member>)
obj* prim_member_direct(obj* env, obj* list){
  obj* stru = eval(env, list->car);
  
  if(!stru){
    printf("[%s] no obj", __func__);
    return Nil;
  }
  
  if(stru->type != GVAR && stru->type != RES){
    printf("[%s] not gvar", __func__);
    return Nil;
  }

  int tidx = stru->tidx;
  if(types[tidx].kind != structure_type){
    printf("[%s] not struct", __func__);
    return Nil;
  }
  
  return get_member_direct(stru, list->cdr->car);
}

// (-> <struct pointer> <member>)
obj* prim_member_indirect(obj* env, obj* list){
  obj* stru = eval(env, list->car);
  
  if(!stru){
    printf("[%s] no obj", __func__);
    return Nil;
  }

  if(stru->type != GVAR && stru->type != RES){
    printf("[%s] not gvar", __func__);
    return Nil;
  }
  
  int tidx = stru->tidx;
  if(types[tidx].kind != pointer_type || types[types[tidx].saki].kind != structure_type){
    printf("[%s] not struct pointer", __func__);
    return Nil;
  }

  return get_member_indirect(stru, list->cdr->car);
}

// addrからbytesizeだけblockへコピーする
void read_mem_block(int bytesize, long addr, void* block){
  int i;
  long data;
  for(i=0;i<bytesize;i+=SIZE){
    data = read_mem(SIZE, addr+i);
    memcpy(block+i, &data, SIZE);
  }
}

obj* printstringp(obj* o){
  int tidx = o->tidx;
  int i, j, size=256;
  long data, addr = o->data;
  char *str = (char*)malloc(sizeof(char)*size);
  
  for(i=1;i<types[tidx].pcount;i++){
    addr = read_mem(SIZE, addr);
  }
  
  for(i=0;i<size;i+=SIZE){
    data = read_mem(SIZE, addr+i);
    memcpy(str+i, &data, SIZE);
    for(j=0;j<SIZE;j++){
      if(str[i+j] == '\0'){
	printf("%s ", str);
	return Nil;
      }
    }
  }
  return Nil;
}

obj* printstringa(obj* o){
  int tidx = o->tidx;
  long addr = o->data;
  char* str = (char*)malloc(types[tidx].bytesize);
  
  read_mem_block(types[tidx].bytesize, addr, (void*)str);
  printf("%s ", (char*)str);
  return Nil;
}

// (printstring <char*>)
// (printstring <char[]>)
obj* prim_printstring(obj* env, obj* list){
  obj* o = eval(env, list->car);

  if(o->type != GVAR && o->type != RES){
    printf("[%s] malformed printstring", __func__);
    return Nil;
  }
  
  int tidx = o->tidx;
  if(types[tidx].kind == pointer_type && (types[tidx].tbit & _CHAR)){
    return printstringp(o);
  }
  else if(types[tidx].kind == array_type && (types[tidx].tbit & _CHAR)){
    return printstringa(o);
  }
  else{
    printf("[%s] takes only pointer or array", __func__);
    return Nil;
  }
}

// (print expr ...)
// 型を表示しない
obj *prim_print(obj *env, obj *list){
  for(obj* args = eval_list(env, list); args != Nil; args = args->cdr) {
    if(args->car->type == GVAR || args->car->type == RES){
      int tidx = args->car->tidx;
      if(types[tidx].kind == pointer_type && (types[tidx].tbit & _CHAR)){
	printstringp(args->car);
	continue;
      }
      else if(types[tidx].kind == array_type && (types[tidx].tbit & _CHAR)){
	printstringa(args->car);
	continue;
      }
    }
    print(args->car, TYPE_OFF);
    printf(" ");
  }
  printf("\n");
  return Nil;
}

// (printt expr ...)
// 型を表示する
obj *prim_printt(obj *env, obj *list){
  for(obj* args = eval_list(env, list); args != Nil; args = args->cdr) {
    if(args->car->type == GVAR || args->car->type == RES){
      int tidx = args->car->tidx;
      if(types[tidx].kind == pointer_type && (types[tidx].tbit & _CHAR)){
	printstringp(args->car);
	continue;
      }
      else if(types[tidx].kind == array_type && (types[tidx].tbit & _CHAR)){
	printstringa(args->car);
	continue;
      }
    }
    print(args->car, TYPE_ON);
    printf(" ");
  }
  return Nil;
}


void printa(int tbit, int arraysize, long* array){
  for(int i=0;i<arraysize;i++){
    print_base(array[i], tbit);
    printf(" :: %s\n", get_typename(tbit));
  }
}

// (printarray <array var>)
// (printarray <array var> <size>)
obj* prim_printarray(obj* env, obj* list){
  if(length(list) != 1 && length(list) != 2){
    printf("[%s] malformed printarray", __func__);
    return Nil;
  }
  
  obj* o = eval(env, list->car);

  if(o->type != GVAR && o->type != RES){
    printf("[%s] malformed printstring", __func__);
    return Nil;
  }
  
  int tidx = o->tidx;
  long addr = o->data;

  
  if(types[tidx].kind != array_type){
    printf("[%s] not struct", __func__);
    return Nil;
  }
  
  int arraysize=0;
  long* array=NULL;

  if(length(list) == 1){
    arraysize = types[tidx].arraysize;
    array = (long*)malloc(types[tidx].bytesize);
  }
  else if(length(list) == 2){
    obj* sizeo = eval(env, list->cdr->car);
    arraysize = sizeo->value;
    array = (long*)malloc(types[types[tidx].saki].bytesize*sizeo->value);    
  }
  
  read_mem_block(types[tidx].bytesize, addr, array);
  printa(types[tidx].tbit, arraysize, array);

  return Nil; 
}

// (printarray <struct var>)
obj* prim_printstruct(obj* env, obj* list){
    obj* stru = eval(env, list->car);
  
  if(!stru){
    printf("[%s] no obj", __func__);
    return Nil;
  }
  
  if(stru->type != GVAR && stru->type != RES){
    printf("[%s] not gvar", __func__);
    return Nil;
  }

  int tidx = stru->tidx;
  if(types[tidx].kind != structure_type){
    printf("[%s] not struct", __func__);
    return Nil;
  }

  for(int i=0;i<types[tidx].memnum;i++){
    int membtidx = types[tidx].mem[i].tidx;
    long membaddr = stru->data + types[tidx].mem[i].offset;
    obj* memb = get_gvar(make_gvar(membtidx, membaddr));
    
    printf("%s = ", types[tidx].mem[i].name);
    print_gvar(memb, TYPE_ON);
    printf("\n");
  }
  
  return Nil;
}

// (cast <gvar> <gvar>)
obj *prim_cast(obj *env, obj *list){
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    printf("[%s] malformed -", __func__);
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;

  if((x->type != GVAR && x->type != RES) ||
     (y->type != GVAR && y->type != RES)){
    printf("[%s] cast takes only gvar", __func__);
    return Nil;
  }

  y->tidx = x->tidx;
  
  return y;
}

bool is_list(obj *o) {
    return o == Nil || o->type == LIST;
}

obj* param_check(obj *env, obj *list){
  if (list->type != LIST  ||
      !is_list(list->car) ||
      list->cdr->type != LIST ){
    printf("[%s] Malformed lambda", __func__);
    return Nil;
  }
  obj* p = list->car;
  for (; p->type == LIST; p = p->cdr)
    if (p->car->type != SYM){
      printf("[%s] Parameter must be a symbol", __func__);
      return Nil;
    }
  if (p != Nil && p->type != SYM){
    printf("[%s] Parameter must be a symbol", __func__);
    return Nil;
  }
  obj* params = list->car;
  obj* body   = list->cdr;
  return make_function(env, params, body);
}

// (defun <symbol> (<symbol> ...) expr ...)
obj *prim_defun(obj *env, obj *list) {
  if (list->car->type != SYM || list->cdr->type != LIST){
    printf("[%s] Malformed defun", __func__);
    return Nil;
  }
  obj* sym  = list->car;
  obj* rest = list->cdr;
  obj* fn   = param_check(env, rest);
  add_variable(env, sym, fn);
  return fn;
}

void add_variable(obj* env, obj* sym, obj* val) {
    obj* vars = env->vars;
    obj* tmp  = acons(sym, val, vars);
    env->vars = tmp;
}

void add_primitive(obj* env, char* sym, primitive* prim){
  obj* s = check_symbol(sym);
  obj* p = make_prim(prim);
  add_variable(env, s, p);
}

// 基本関数定義
void define_primitive(obj* env){
  add_primitive(env, "+",           prim_add);
  add_primitive(env, "-",           prim_sub);
  add_primitive(env, "<",           prim_lt);
  add_primitive(env, ">",           prim_gt);
  add_primitive(env, "=",           prim_eq);
  add_primitive(env, "!=",          prim_not_eq);
  add_primitive(env, "if",          prim_if);
  add_primitive(env, "while",       prim_while);
  add_primitive(env, "define",      prim_define);
  add_primitive(env, "set",         prim_set);
  add_primitive(env, "deref",       prim_deref);
  add_primitive(env, ".",           prim_member_direct);
  add_primitive(env, "->",          prim_member_indirect);
  add_primitive(env, "print",       prim_print);
  add_primitive(env, "printt",      prim_printt);
  add_primitive(env, "printstring", prim_printstring);
  add_primitive(env, "printarray",  prim_printarray);
  add_primitive(env, "printstruct", prim_printstruct);
  add_primitive(env, "cast",        prim_cast);
  add_primitive(env, "defun",       prim_defun);
}

void define_gvar(void){
  gtbl = g_hash_table_new(g_str_hash, g_str_equal);
  for(int i=0;i<sizeof(gvars)/sizeof(gvars[0]);i++){
    int* gidx = g_new(int, 1);
    *gidx = i;
    g_hash_table_insert(gtbl, gvars[i].name, gidx);
  }
}

void define_t(obj* env){
  obj* s = check_symbol("t");
  add_variable(env, s, True);
}

void define_nil(obj* env){
  obj* s = check_symbol("nil");
  add_variable(env, s, Nil);
}

void define_break(obj* env){
  obj* s = check_symbol("break");
  add_variable(env, s, Break);
}

//================
// print
//================

#define  PR_FMT(f,x)  printf("%"#f, (x))

// リストを表示する
void print_list(obj* o, int tflag){
  for(;;){
    print(o->car, tflag);
    if(o->cdr == Nil){
      break;
    }
    printf(" ");
    o = o->cdr;
  }
}

void print_addr(long data){
  printf("0x%lx", data);
}

void print_gvar(obj* o, int tflag){
  int kind = types[o->tidx].kind;
  int tbit = types[o->tidx].tbit;
  
  if(kind == base_type){
    print_base(o->data, tbit);
    if(tflag) printf(" :: %s", get_typename(tbit));
  }
  else if(kind == pointer_type   ||
	  kind == array_type     ||
	  kind == structure_type ||
	  kind == union_type){
    print_addr(o->data);
    if(tflag) printf(" :: %s", get_typename(tbit));
    if(tflag && kind == pointer_type) printf("*");
    else if(tflag && kind == array_type) printf("[]");
  }
  else if(kind == enumeration){
    print_base(o->data, tbit);
    if(tflag) printf(" :: %s", get_typename(tbit));
  }
  else{
    printf("[%s] not defined", __func__);
  }
}

// objの中身を表示する
// リストの場合は辿ってすべて表示する
void print(obj* o, int tflag){
  if(!o){
    printf("[%s] no obj", __func__);
    return;
  }
  
  switch(o->type){
  case INT:
    printf("%ld", o->value);
    break;
  case STR:
    printf("%s", o->string);
    break;
  case SYM:
    printf("%s", o->symbol);
    break;
  case PRIM:
    printf("PRIM");
    break;
  case FUNC:
    printf("FUNC");
    break;
  case LIST:
    printf("(");
    print_list(o, tflag);
    printf(")");
    break;
  case ENV:
    printf("ENV");
    break;
  case NIL:
    //printf("NIL");
    return;
  case GVAR:
    print_gvar(o, tflag);
    break;
  case RES:
    print_gvar(o, tflag);
    break;
  case TTRUE:
    printf("TRUE");
    break;
  case BREAK:
    printf("BREAK");
    break;
  default:
    printf("[%s] not defined", __func__);
    break;
  }
}

int main(int argc, char **argv){  
  if(argc != 2){
    printf("[main] argc error");
    return 0;
  }

#ifdef PTRACE_ON
  pid = atoi(argv[1]);
#endif

#ifdef LIBVMI_ON
  vmname = argv[1];
#endif

  symbol_table = Nil;
  obj* env = make_env(Nil, Nil);
  define_primitive(env);
  define_gvar();
  define_t(env);
  define_nil(env);
  define_break(env);

  init();

  for(;;){
    printf("> ");
    print(eval(env, read_main(stdin)), TYPE_ON);
    printf("\n");
  }
  
  quit();

  return 0;
}
