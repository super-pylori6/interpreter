#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _VOID:
    return "void";
  case _INT:
    return "int";
  case _CHAR:
    return "char";
  case _STRUCT_TASK_STRUCT:
    return "struct task_struct";
  case _STRUCT_LIST_HEAD:
    return "struct list_head";
  case _ADDR_T:
    return "int";    
  default:
    return "";
  }
}

void print_base(long data, int tbit){
  switch(tbit){
  case _VOID:
    printf("0x%lx", (long int)data);
    break;    
  case _ADDR_T:
    printf("0x%lx", (long int)data);
    break;    
  case _LONG_UNSIGNED_INT:
    printf("%lu", (long unsigned int)data);
    break;
  case _UNSIGNED_CHAR:
    printf("%hhu", (unsigned char)data);
    break;
  case _SHORT_UNSIGNED_INT:
    printf("%hu", (short unsigned int)data);
    break;
  case _UNSIGNED_INT:
    printf("%u", (unsigned int)data);
    break;
  case _SIGNED_CHAR:
    printf("%hhd", (signed char)data);
    break;
  case _SHORT_INT:
    printf("%d", (short int)data);
    break;
  case _INT:
    printf("%d", (int)data);
    break;
  case _LONG_INT:
    printf("%ld", (long int)data);
    break;
  case _FLOAT:
    printf("%f", (float)data);
    break;
  case _DOUBLE:
    printf("%lf", (double)data);
    break; 
  case _CHAR:
    printf("%c", (char)data);
    break;
  default:
    printf("[%s] not defined", __func__);
    return;
  }
}

struct gvarinfo gvars[1] = {
    {7, "init_task", 0xffffffff81c15440},
};

struct memberinfo task_struct[3] = {
    {2, "tasks", 0x260},
    {4, "pid", 0x2d4},
    {6, "comm", 0x480},
};

struct memberinfo list_head[2] = {
    {3, "next", 0x0},
    {3, "prev", 0x8},
};

struct typeinfo types[8] = {
    {base, _VOID, 8},
    {base, _ADDR_T, 8},
    {structure, _STRUCT_LIST_HEAD, 16, .memnum=2, .mem=list_head},
    {pointer, _STRUCT_LIST_HEAD, 8, .saki=2, .pcount=1},
    {base, _INT, 4},
    {base, _CHAR, 1},
    {array, _CHAR, 16, .saki=5, .arraysize=16},
    {structure, _STRUCT_TASK_STRUCT, 6784, .memnum=3, .mem=task_struct},
};
