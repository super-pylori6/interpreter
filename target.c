#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // malloc()


int ggg  = 4;
long hhh = 6;
int* p   = NULL;
int* q   = NULL;
int ia[] = {0, 1, 2, 3};

const char c = 'x';
char ca[]    = "hello world!";
char* cp     = "hello world!";

void add(void* x, const void* y){
  return;
}

union quiz {
  int a;
  int b;
  int c;
};

union quiz q1 = {.a=1};

typedef int newint;
typedef newint newnewint;

struct person {
  char *name;
  char sex;
  int age;
  char *add;
  char *job;
  newint tmp;
  newnewint zzz;
};

struct person tanaka = {
  "T.Tanaka",'m',30,"Tokyo","teacher",1,1
};

struct person ito = {
  "K.Ito",'f',25,"Osaka","teacher",2,3
};

struct kyuuyo{
  long kihon;
  long jyutaku;
  long kazoku;
  long sikaku;
};

struct syain_dt{
  long no;
  char name[20];
  char yaku[10];
  int nensu;
  struct kyuuyo kyu;
  struct person* pp;
  void (*func)(void* x, const void *y);
};

struct syain_dt tokyo = {
  1,
  "sato",
  "Manager",
  10,
  {20, 5, 2, 1},
  &tanaka,
  add
};

struct cell {
  int val;
  struct cell* next;
};

struct cell c1;

struct cell c3 = {
  3, &c1
};

struct cell c2 = {
  2, &c3
};

struct cell c1 = {
  1, &c2
};

struct cell* top = &c1;

int main(void){
  int iii = 12;
  double ddd = 34.5;
  float fff = 6.7;
  struct syain_dt syomu[20];

  while(1){
    printf("running:0\n");
    sleep(1);
    printf("running:1\n");
    sleep(1);
  }
  return 0;
}
