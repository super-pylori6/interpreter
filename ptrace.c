#include <stdio.h>
#include <stdlib.h> // malloc()
#include <ctype.h> // isdigit(), isalpha()
#include <string.h> // strchr()
#include <assert.h> // assert()
#include <sys/ptrace.h> // ptrace()
#include <sys/wait.h> // waitpid()
#include <sysexits.h> // exit(EXIT_SUCCESS)
#include "debuginfo.h"

#define SYMBOL_MAX_LEN 100

enum {
  INT,
  STR,
  SYM,
  PRIM,
  LIST,
  NIL,
  RPAREN,
  FUNC,
  ENV,
  GVAR,
  TRUE
};


struct obj;

// 基本関数用の関数ポインタ
typedef struct obj *primitive(struct obj *env, struct obj *args);

typedef struct obj {
  int type;
  union {
    // 数値
    int integer;
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
      int typenum;
      long val;
    };
  };
} obj;

obj* True = &(obj){TRUE};
obj* Nil = &(obj){NIL};

obj *symbol_table;

pid_t pid;

obj* get_gvar(obj* o);


/*
 * constructor
 */

// 数値を格納するobjを定義する
obj* make_int(int integer){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = INT;
  tmp->integer = integer;
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

// NIL定数を表すobjを定義する
obj* make_nil(void){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = NIL;
  return tmp;
}

// 右括弧を表すobjを定義する
obj* make_rparen(void){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = RPAREN;
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

// 対象プロセスのグローバル変数情報を格納するobjを定義する
obj* make_gvar(int typenum, long addr){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = GVAR;
  tmp->typenum = typenum;
  tmp->val = addr;
  return tmp;
}

// 真を表すobjを定義する
obj* make_true(void){
  obj* tmp = (obj*)malloc(sizeof(obj));
  tmp->type = TRUE;
  return tmp;
}


/*
 * read
 */

obj* read(void);
void print(obj* o);


// １文字先読みする
int peek(void) {
  int c = getchar();
  ungetc(c, stdin);
  return c;
}

// リストを反転する
obj* reverse(obj* o){
  obj* ret = Nil;
  while(o->type != NIL){
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
    if(!tmp || tmp->type == NIL){
      printf("右括弧がありません\n");
      return NULL;
    }
    else if(tmp->type == RPAREN){
      return reverse(head); //リストを反転して返す
    }
    head = make_list(tmp, head); // 先頭に挿入する
  }
}

obj* read_rparen(void){
  return make_rparen();
}

// 数値を読み込む
// 後続文字が数字である限り数値を計算する
obj* read_num(int integer){
  while(isdigit(peek())){
    integer = integer * 10 + getchar() - '0';
  }
  return make_int(integer);
}

// すでに同じ名前のシンボルがあるか確認する
obj* check_symbol(char* symbol){
  for(obj *o = symbol_table; o->type != NIL; o = o->cdr){
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
      printf("シンボル名が長過ぎます\n");
      return NULL;
    }
    symbol[len++] = getchar();
  }
  symbol[len] = '\0';
  
  return check_symbol(symbol);
}

void quit(void){
  int ret;
  
  ret = ptrace(PTRACE_CONT, pid, NULL, NULL);
  if(ret != 0){
    perror("[main] PTRACE_CONT error\n");
    return;
  }
  kill(pid, SIGSTOP);
  waitpid(pid, NULL, 0);
  
  ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
  if(ret != 0){
    perror("[main] PTRACE_DETACH error\n");
    return;
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

/*
obj* read_s(void) {
  int c1 = getchar();
  int c2 = getchar();
  
  if(c1 == ':' && c2 == 'q'){
    quit();
  }
  else{
    ungetc(c2, stdin);
    ungetc(c1, stdin);
  }

  return read();
}
*/

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
      return read_rparen();
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

/*
 * eval
 */

obj* eval(obj* env, obj* o);

// 環境を調べて変数を検索する
// 未定義ならNULLを返す
obj* lookup(obj* env, obj* sym){
  for(obj* e = env; e->type != NIL; e = e->up){
    for(obj* cell = e->vars; cell->type != NIL; cell = cell->cdr){
      obj* tmp = cell->car;
      if(sym == tmp->car){
	return tmp;
      }
    }
  }
  return Nil;
}

obj* progn(obj *env, obj *list) {
  obj* r = Nil;
  for (obj* lp = list; lp->type != NIL; lp = lp->cdr) {
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
  for(obj* lp = list; lp->type != NIL; lp = lp->cdr){
    o = lp->car;
    result = eval(env, o);
    head = make_list(result, head);
  }
  return reverse(head);
}

obj* apply(obj* env, obj* fn, obj* args){
  if(args->type != LIST && args->type != NIL){
    perror("[apply] obj->type error");
  }
  if(fn->type != PRIM){
    perror("[apply] fn->type error");
  }
  return fn->prim(env, args);
}

obj* eval(obj* env, obj* o){
  switch(o->type){
  case INT:
  case STR:
  case PRIM:
  case NIL:
    return o;
  case SYM:{
    obj* list_sym = lookup(env, o);
    if(list_sym->type == NIL){
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

/*
 * ptrace処理
 */

// メンバ取得処理を作成する

long read_mem(int size, long addr){
  //printf("%lx\n", addr);
  return ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
}

obj* get_base(obj* o){
  int typenum = o->typenum;
  long val = read_mem(8, o->val);
  return make_gvar(typenum, val);
}

obj* get_pointer(obj* o){
  int typenum = o->typenum;
  return make_gvar(typenum, o->val);
}

obj* get_array(obj* o){
  return get_pointer(o);
}


obj* get_struct(obj* o){
  return get_pointer(o);
}

obj* get_gvar(obj* o){
  int typenum = o->typenum;
  if(types[typenum].kind == base){
    return get_base(o);
  }
  else if(types[typenum].kind == pointer){
    return get_pointer(o);
  }
  else if(types[typenum].kind == array){
    return get_array(o);
  }
  else if(types[typenum].kind == structure){
    return get_struct(o);
  }
  else{
    perror("[get_gvar] not defined");
  }
}

int search_memb(char* memb, int typenum){
  for(int i=0;types[typenum].memnum;i++){
    if(strcmp(memb, types[typenum].mem[i].name) == 0){
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
  for(obj* args = eval_list(env, list); args->type != NIL; args = args->cdr) {
    if(args->car->type != INT){
      perror("[prim_add] args type error");
    }
    sum += args->car->integer;
  }
  return make_int(sum);
}

// (define <symbol> expr)
// グローバル変数も上書きできてしまう
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
  int typenum = stru->typenum;

  //指定の構造体が指定のメンバを持つか検索
  int i = search_memb(memb->symbol, typenum);
  if(i<0){
    perror("[get_member_direct] no member");
    return Nil;
  }

  int membtypenum = types[typenum].mem[i].typenum;
  long membaddr = stru->val + types[typenum].mem[i].offset;

  return get_gvar(make_gvar(membtypenum, membaddr));
}

obj* get_member_indirect(obj* poin, obj* memb){
  int typenum = types[poin->typenum].saki;
  long val = read_mem(8, poin->val);

  return get_member_direct(make_gvar(typenum, val), memb);
}

// (. <struct> <member>)
obj* prim_member_direct(obj* env, obj* list){
  obj* stru = eval(env, list->car);
  
  if(!stru){
    perror("[prim_member_direct] no obj");
    return Nil;
  }
  
  if(stru->type != GVAR){
    perror("[prim_member_direct] not gvar");
    return Nil;
  }

  int typenum = stru->typenum;
  if(types[typenum].kind != structure){
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

  if(stru->type != GVAR){
    perror("[prim_member_direct] not gvar");
    return Nil;
  }
  
  int typenum = stru->typenum;
  if(types[typenum].kind != pointer || types[types[typenum].saki].kind != structure){
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

  int typenum = o->typenum;
  if(types[typenum].kind != pointer){
    perror("[prim_printp] not pointer");
    return Nil;
  }
  
  int i, j;
  long addr = o->val;
  long val;
  int pcount = types[typenum].pcount;

  if(strcmp(types[typenum].name, "char") == 0){
    int size=256;
    char *str = (char*)malloc(sizeof(char)*size);
    for(i=0;i<pcount;i++){
      addr = read_mem(8, addr);
    }
    for(i=0;i<size;i+=sizeof(long)){
      val = read_mem(8, addr+i);
      memcpy(str+i, &val, sizeof(long));
      for(j=0;j<sizeof(long);j++){
	if(str[i+j] == '\0'){
	  printf("%s\n", str);
	  //return make_gvar(typenum, (long)str);
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

  int typenum = o->typenum;
  if(types[typenum].kind != array){
    perror("[prim_printp] not pointer");
    return Nil;
  }
  
  int i;
  long val;
  long addr = o->val;
  int arraysize = types[typenum].arraysize;
  
  if(strcmp(types[typenum].name, "char") == 0){
    char* str = (char*)malloc(sizeof(char)*types[typenum].arraysize);
    for(i=0;i<arraysize;i+=sizeof(long)){
      val = read_mem(8, addr+i);
      memcpy(str+i, &val, sizeof(long));
    }
    printf("%s\n", str);
    //return make_gvar(typenum, (long)str);
    return Nil;
  }
  else{
    perror("[prim_printa] not defined");
    return Nil;
  }
}

// (< <integer> <integer>)
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
  return x->integer < y->integer ? True : Nil;
}

// (if expr expr expr ...)
obj* prim_if(obj *env, obj *list) {
  if (length(list) < 2){
    perror("[prim_if] Malformed if");
    return Nil;
  }
  obj* cond = list->car;
  cond = eval(env, cond);
  if (cond->type != NIL) {
    obj* then = list->cdr->car;
    return eval(env, then);
  }
  obj* els = list->cdr->cdr;
  return els->type == NIL ? Nil : progn(env, els);
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
    eval_list(env, exprs);
  }
  //printf("while\n");
  return Nil;
}

// (= <integer> <integer>)
obj* prim_num_eq(obj *env, obj *list) {
  if (length(list) != 2){
    perror("[prim_num_eq] Malformed =");
    return Nil;
  }
  obj* values = eval_list(env, list);
  obj* x = values->car;
  obj* y = values->cdr->car;
  if (x->type != INT || y->type != INT){
    perror("[prim_num_eq] = only takes numbers");
  }
  return x->integer == y->integer ? True : Nil;
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
}


void define_globalvar(obj* env){
  for(int i=0;i<sizeof(globalvars)/sizeof(globalvars[0]);i++){
    obj* s = check_symbol(globalvars[i].name);
    obj* gvar = make_gvar(globalvars[i].typenum, globalvars[i].addr);
    add_variable(env, s, gvar);
  }
}

// リストを表示する
void print_list(obj* o){
  for(;;){
    print(o->car);
    if(o->cdr->type == NIL){
      break;
    }
    printf(" ");
    o = o->cdr;
  }
}

void print_base(obj* o){
  if(strcmp(types[o->typenum].name, "long unsigned int") == 0){
    printf("%lu : %s", (long unsigned int)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "short unsigned int") == 0){
    printf("%u : %s", (short unsigned int)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "unsigned int") == 0){
    printf("%u : %s", (unsigned int)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "short int") == 0){
    printf("%d : %s", (short int)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "int") == 0){
    printf("%d : %s", (int)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "long int") == 0){
    printf("%ld : %s", o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "float") == 0){
    printf("%f : %s", (float)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "double") == 0){
    printf("%lf : %s", (double)o->val, types[o->typenum].name);
  }
  else if(strcmp(types[o->typenum].name, "char") == 0){
    printf("%c : %s", (char)o->val, types[o->typenum].name);
  }
  else{
    perror("[print_base] not defined");
  }
}

void print_pointer(obj* o){
  printf("0x%lx : %s*", o->val, types[o->typenum].name);
}

void print_array(obj* o){
  print_pointer(o);
}

void print_struct(obj* o){
  print_pointer(o);
}

void print_gvar(obj* o){
  if(types[o->typenum].kind == base){
    print_base(o);
  }
  else if(types[o->typenum].kind == pointer){
    print_pointer(o);
  }
  else if(types[o->typenum].kind == array){
    print_array(o);
  }
  else if(types[o->typenum].kind == structure){
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
    printf("%d", o->integer);
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
  case TRUE:
    printf("TRUE");
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
  pid = atoi(argv[1]);
    
  symbol_table = Nil;
  obj* nil = Nil;
  obj* env = make_env(nil, nil);
  define_primitive(env);
  define_globalvar(env);

  ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
  if(ret != 0){
    perror("[main] PTRACE_ATTACH error\n");
    return 0;
  }
  
  for(;;){
    print(eval(env, read_s()));
  }
  
  ret = ptrace(PTRACE_CONT, pid, NULL, NULL);
  if(ret != 0){
    perror("[main] PTRACE_CONT error\n");
    return 0;
  }
  kill(pid, SIGSTOP);
  waitpid(pid, NULL, 0);
    
  ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
  if(ret != 0){
    perror("[main] PTRACE_DETACH error\n");
    return 0;
  }

  return 0;
}
