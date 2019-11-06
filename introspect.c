#include <stdio.h>
#include <stdlib.h> // malloc()
#include <ctype.h> // isdigit(), isalpha()
#include <string.h> // strchr()
#include <sysexits.h> // exit(EXIT_SUCCESS)
#include "debuginfo.h"

#define PTRACE_ON

#ifdef PTRACE_ON
#include <sys/ptrace.h> // ptrace()
#include <sys/wait.h> // waitpid()
#endif

#ifdef PTRACE_ON
pid_t pid;
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
  TRUE,
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
    int value;
    // 文字列
    char* string;
    // シンボル
    char* symbol;
    // 基本関数
    primitive* prim;
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

obj* Rparen = &(obj){RPAREN};
obj* True = &(obj){TRUE};
obj* Nil = &(obj){NIL};
obj* Break = &(obj){BREAK};

// envで一括管理すれば消せそう
obj *symbol_table;

obj* get_gvar(obj* o);
void print(obj* o);



//================
// constructor
//================

// 数値を格納するobjを定義する
obj* make_int(int value){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = INT;
  tmp->value = value;
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


//================
// read
//================

obj* read(void);

// １文字先読みする
// ungetc()でストリームに１文字返す
int peek(void) {
  int c = getchar();
  ungetc(c, stdin);
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
obj* read_list(void){
  obj* head = Nil;
  for(;;){
    obj* tmp = read();
    if(!tmp || tmp == Nil){
      perror("[read_list] no )\n");
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
obj* read_num(int value){
  while(isdigit(peek())){
    value = value * 10 + getchar() - '0';
  }
  return make_int(value);
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
obj* read_symbol(char c){
  char symbol[SYMBOL_MAX_LEN + 1];
  symbol[0] = c;
  int len = 1;
  while(isalnum(peek()) || strchr(symbol_chars, peek())){
    if(SYMBOL_MAX_LEN <= len){
      perror("[read_symbol] too long symbol name");
      return NULL;
    }
    symbol[len++] = getchar();
  }
  symbol[len] = '\0';
  
  return check_symbol(symbol);
}

void init(void){
  int ret;
  
#ifdef PTRACE_ON
  ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
#endif
  
  if(ret != 0){
    perror("[main] ATTACH error\n");
    exit(EXIT_FAILURE);
  }
}

void quit(void){
  int ret;

#ifdef PTRACE_ON
  ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
#endif
  
  if(ret != 0){
    perror("[main] DETACH error\n");
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}

obj* read_s(void){
  for(;;){
    int c = getchar();
    if(c == ' ' || c == '\n' || c == '\r' || c == '\t'){
      continue;
    }
    else if(c == ':'){
      if(getchar() == 'q'){
	quit();
      }
      return make_sym(" ");
    }
    else{
      ungetc(c, stdin);
      return read();
    }
  }
}

// 読み込み
// 空白は飛ばす
obj* read(void){
  for(;;){
    int c = getchar();
    if(c == ' ' || c == '\n' || c == '\r' || c == '\t'){
      continue;
    }
    else if(c == '('){
      return read_list();
    }
    else if(c == ')'){
      return Rparen;
    }
    else if(isdigit(c)){
      return read_num(c - '0');
    }
    else if (isalpha(c) || strchr(symbol_chars, c)){
      return read_symbol(c);
    }
    else{
      perror("[read] error");
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

obj* apply(obj* env, obj* fn, obj* args){
  if(args->type != LIST && args != Nil){
    perror("[apply] obj->type error");
  }
  if(fn->type != PRIM){
    perror("[apply] fn->type error");
  }
  return fn->prim(env, args);
}

obj* eval(obj* env, obj* o){
  if(!o){
    perror("[eval] no obj");
    return NULL;
  }
  
  switch(o->type){
  case INT:
  case STR:
  case PRIM:
  case GVAR:
  case RES:
  case TRUE:
  case NIL:
    return o;
  case SYM:{
    obj* list_sym = lookup(env, o);
    if(list_sym == Nil){
      perror("[eval] no symbol");
      return Nil;
    }
    else if(list_sym->cdr->type == GVAR){
      return get_gvar(list_sym->cdr);
    }
    else{
      return list_sym->cdr;
    }
  }
  case LIST:{
    obj* fn = eval(env, o->car);
    obj* args = o->cdr;
    if(fn->type != PRIM && fn->type != FUNC){
      perror("[eval] fn->type error");
      return Nil;
    }
    return apply(env, fn, args);
  }
  default:
    perror("[eval] obj->type error");
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
}

obj* get_base(obj* o){
  return make_res(o->tidx, read_mem(8, o->data));
}

obj* get_pointer(obj* o){
  return make_res(o->tidx, read_mem(8, o->data));
}

obj* get_array(obj* o){
  return make_res(o->tidx, read_mem(8, o->data));
}

obj* get_struct(obj* o){
  return make_res(o->tidx, o->data);
}

obj* get_gvar(obj* o){
  int tidx = o->tidx;
  if(types[tidx].kind == base){
    return get_base(o);
  }
  else if(types[tidx].kind == pointer){
    return get_pointer(o);
  }
  else if(types[tidx].kind == array){
    return get_array(o);
  }
  else if(types[tidx].kind == structure){
    return get_struct(o);
  }
  else{
    perror("[get_gvar] not defined");
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

/*
 * primitive
 */

void add_variable(obj* env, obj* sym, obj* val);

int length(obj *list) {
    int len = 0;
    for(; list->type == LIST; list = list->cdr){
      len++;
    }
    return list ? len : -1;
}

// (+ 1 2 ...)
obj* prim_add(obj* env, obj* list){
  int sum = 0;
  for(obj* args = eval_list(env, list); args != Nil; args = args->cdr) {
    if(args->car->type != INT){
      perror("[prim_add] args type error");
    }
    sum += args->car->value;
  }
  return make_int(sum);
}

// (define <symbol> expr)
obj* prim_define(obj* env, obj* list){
  if (length(list) != 2 || list->car->type != SYM){
    perror("[prim_define] Malformed define");
    return Nil;
  }
  
  if(list->car->symbol[0] != '$'){
    perror("[prim_define] initial of variable takes only $");
    return Nil;
  };
  
  obj* sym = list->car;
  obj* value = eval(env, list->cdr->car);
  add_variable(env, sym, value);
  return value;
}

obj* get_member_direct(obj* stru, obj* memb){
  int tidx = stru->tidx;

  //指定の構造体が指定のメンバを持つか検索
  int i = search_memb(memb->symbol, tidx);
  if(i<0){
    perror("[get_member_direct] no member");
    return Nil;
  }

  int membtidx = types[tidx].mem[i].tidx;
  long membaddr = stru->data + types[tidx].mem[i].offset;

  return get_gvar(make_gvar(membtidx, membaddr));
}

obj* get_member_indirect(obj* poin, obj* memb){
  int tidx = types[poin->tidx].saki;
  //long data = read_mem(8, poin->data);
  long data = poin->data;

  return get_member_direct(make_gvar(tidx, data), memb);
}

// (. <struct> <member>)
obj* prim_member_direct(obj* env, obj* list){
  obj* stru = eval(env, list->car);
  
  if(!stru){
    perror("[prim_member_direct] no obj");
    return Nil;
  }
  
  if(stru->type != GVAR && stru->type != RES){
    perror("[prim_member_direct] not gvar");
    return Nil;
  }

  int tidx = stru->tidx;
  if(types[tidx].kind != structure){
    perror("[prim_member_direct] not struct");
    return Nil;
  }
  
  return get_member_direct(stru, list->cdr->car);
}

// (-> <struct pointer> <member>)
obj* prim_member_indirect(obj* env, obj* list){
  obj* stru = eval(env, list->car);
  
  if(!stru){
    perror("[prim_member_direct] no obj");
    return Nil;
  }

  if(stru->type != GVAR && stru->type != RES){
    perror("[prim_member_direct] not gvar");
    return Nil;
  }
  
  int tidx = stru->tidx;
  if(types[tidx].kind != pointer || types[types[tidx].saki].kind != structure){
    perror("[prim_member_direct] not struct pointer");
    return Nil;
  }

  return get_member_indirect(stru, list->cdr->car);
}


obj* prim_printp(obj* env, obj* list){
  obj* o = eval(env, list->car);

  if(!o){
    perror("[prim_printp] no pointer var");
    return Nil;
  }

  int tidx = o->tidx;
  if(types[tidx].kind != pointer){
    perror("[prim_printp] not pointer");
    return Nil;
  }
  
  int i, j;
  long addr = o->data;
  long data;
  int pcount = types[tidx].pcount;

  if(strcmp(types[tidx].name, "char") == 0){
    int size=256;
    char *str = (char*)malloc(sizeof(char)*size);
    for(i=1;i<pcount;i++){
      addr = read_mem(8, addr);
    }
    for(i=0;i<size;i+=sizeof(long)){
      data = read_mem(8, addr+i);
      memcpy(str+i, &data, sizeof(long));
      for(j=0;j<sizeof(long);j++){
	if(str[i+j] == '\0'){
	  printf("%s\n", str);
	  //return make_gvar(tidx, (long)str);
	  return Nil;
	}
      }
    } 
  }
  else{
    perror("[prim_printp] not defined");
    return Nil;
  }
}

// (printp <array var>)
obj* prim_printa(obj* env, obj* list){
  obj* o = eval(env, list->car);

  if(!o){
    perror("[prim_printp] no pointer var");
    return Nil;
  }

  int tidx = o->tidx;
  if(types[tidx].kind != array){
    perror("[prim_printp] not pointer");
    return Nil;
  }
  
  int i;
  long data;
  long addr = o->data;
  int arraysize = types[tidx].arraysize;
  
  if(strcmp(types[tidx].name, "char") == 0){
    char* str = (char*)malloc(sizeof(char)*types[tidx].arraysize);
    for(i=0;i<arraysize;i+=sizeof(long)){
      data = read_mem(8, addr+i);
      memcpy(str+i, &data, sizeof(long));
    }
    printf("%s\n", str);
    //return make_gvar(tidx, (long)str);
    return Nil;
  }
  else{
    perror("[prim_printa] not defined");
    return Nil;
  }
}

// (< <value> <value>)
obj* prim_lt(obj *env, obj *list) {
  obj *args = eval_list(env, list);
  if(length(args) != 2){
    perror("[prim_lt] malformed <");
    return Nil;
  }
  obj *x = args->car;
  obj *y = args->cdr->car;
  if(x->type != INT || y->type != INT){
    perror("[prim_lt] < takes only numbers");
  }
  return x->value < y->value ? True : Nil;
}

// (if expr expr expr ...)
obj* prim_if(obj *env, obj *list) {
  if (length(list) < 2){
    perror("[prim_if] Malformed if");
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
    perror("[get_end] not list");
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
    perror("[prim_while] Malformed while");
    return Nil;
  }
  obj* cond = list->car;
  obj* exprs = Nil;
  while (eval(env, cond) != Nil) {
    exprs = list->cdr;
    if(break_in(eval_list(env, exprs))){
      break;
    }
  }
  return Nil;
}

// (= <value> <value>)
obj* prim_num_eq(obj *env, obj *list) {
  if (length(list) != 2){
    perror("[prim_num_eq] Malformed =");
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
  else{
    perror("[prim_num_eq] = only takes numbers");
    return Nil;    
  }
}

// (eq expr expr)
obj *prim_eq(obj *env, obj *list) {
  if (length(list) != 2){
    perror("[prim_eq] Malformed eq");
    return Nil;
  }
  obj *values = eval_list(env, list);
  return values->car == values->cdr->car ? True : Nil;
}

obj* prim_deref(obj* env, obj* list){
  obj* o = eval(env, list->car);
  
  if(!o){
    perror("[prim_deref] no pointer var");
    return Nil;
  }
  
  int tidx = o->tidx;
  if(types[tidx].kind != pointer){
    perror("[prim_deref] not pointer");
    return Nil;
  }
  
  return make_res(types[tidx].saki, read_mem(8, o->data));
}

// (break)
obj* prim_break(obj* env, obj* list){
  return Break;
}

void add_variable(obj* env, obj* sym, obj* val) {
    obj* vars = env->vars;
    obj* tmp = make_list(make_list(sym, val), vars);
    env->vars = tmp;
}

void add_primitive(obj* env, char* sym, primitive* prim){
  obj* s = check_symbol(sym);
  obj* p = make_prim(prim);
  add_variable(env, s, p);
}

// 基本関数定義
void define_primitive(obj* env){
  add_primitive(env, "+", prim_add);
  add_primitive(env, "define", prim_define);
  add_primitive(env, ".", prim_member_direct);
  add_primitive(env, "->", prim_member_indirect);
  add_primitive(env, "printp", prim_printp);
  add_primitive(env, "printa", prim_printa);
  add_primitive(env, "<", prim_lt);
  add_primitive(env, "if", prim_if);
  add_primitive(env, "while", prim_while);
  add_primitive(env, "=", prim_num_eq);
  add_primitive(env, "eq", prim_eq);
  add_primitive(env, "deref", prim_deref);
  add_primitive(env, "break", prim_break);
}


void define_gvar(obj* env){
  for(int i=0;i<sizeof(gvars)/sizeof(gvars[0]);i++){
    obj* s = check_symbol(gvars[i].name);
    obj* gvar = make_gvar(gvars[i].tidx, gvars[i].addr);
    add_variable(env, s, gvar);
  }
}

//================
// print
//================

void print(obj* o);

// リストを表示する
void print_list(obj* o){
  for(;;){
    print(o->car);
    if(o->cdr == Nil){
      break;
    }
    printf(" ");
    o = o->cdr;
  }
}

void print_base(obj* o){
  if(strcmp(types[o->tidx].name, "long unsigned int") == 0){
    printf("%lu : %s", (long unsigned int)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "short unsigned int") == 0){
    printf("%u : %s", (short unsigned int)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "unsigned int") == 0){
    printf("%u : %s", (unsigned int)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "short int") == 0){
    printf("%d : %s", (short int)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "int") == 0){
    printf("%d : %s", (int)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "long int") == 0){
    printf("%ld : %s", o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "float") == 0){
    printf("%f : %s", (float)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "double") == 0){
    printf("%lf : %s", (double)o->data, types[o->tidx].name);
  }
  else if(strcmp(types[o->tidx].name, "char") == 0){
    printf("%c : %s", (char)o->data, types[o->tidx].name);
  }
  else{
    perror("[print_base] not defined");
  }
}

void print_pointer(obj* o){
  printf("0x%lx : %s*", o->data, types[o->tidx].name);
}

void print_array(obj* o){
  printf("0x%lx : %s[]", o->data, types[o->tidx].name);
}

void print_struct(obj* o){
  printf("0x%lx : %s", o->data, types[o->tidx].name);
}

void print_gvar(obj* o){
  if(types[o->tidx].kind == base){
    print_base(o);
  }
  else if(types[o->tidx].kind == pointer){
    print_pointer(o);
  }
  else if(types[o->tidx].kind == array){
    print_array(o);
  }
  else if(types[o->tidx].kind == structure){
    print_struct(o);
  }
  else{
    perror("[print_gvar] not defined");
  }
}

// objの中身を表示する
// リストの場合は辿ってすべて表示する
void print(obj* o){
  if(!o){
    perror("[print] no obj");
    return;
  }
  
  switch(o->type){
  case INT:
    printf("%d", o->value);
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
  case LIST:
    printf("(");
    print_list(o);
    printf(")");
    break;
  case ENV:
    printf("ENV");
    break;
  case NIL:
    //printf("NIL");
    return;
  case GVAR:
    print_gvar(o);
    break;
  case RES:
    print_gvar(o);
    break;
  case TRUE:
    printf("TRUE");
    break;
  case BREAK:
    printf("BREAK");
    break;
  default:
    printf("[print] not defined");
    break;
  }
  printf("\n");
}


int main(int argc, char **argv){
  int ret = 0;
  
  if(argc != 2){
    perror("[main] argc error");
    return 0;
  }
#ifdef PTRACE_ON
  pid = atoi(argv[1]);
#endif
  
  symbol_table = Nil;
  obj* env = make_env(Nil, Nil);
  define_primitive(env);
  define_gvar(env);
  
  init();
  
  for(;;){
    print(eval(env, read_s()));
  }
  
  quit();

  return 0;
}
