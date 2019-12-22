#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _VOID:                   return "void";
  case _ADDR_T:                 return "int";
  case _INT:                    return "int";
  case _CHAR:                   return "char";
  case _LIST_HEAD:              return "struct list_head";
  case _TASK_STRUCT:            return "struct task_struct";
  default: return "";
  }
}

void print_base(long data, int tbit){
  switch(tbit){
  case _ADDR_T:
    printf("0x%lx", (long int)data);
    break;
  case _VOID:
    printf("0x%lx", (long int)data);
    break;
  case _CHAR:
    printf("%c", (char)data);
    break;
  case _INT:
    printf("%d", (int)data);
    break;
  default:
    printf("[%s] not defined", __func__);
    return;
  }
}

struct gvarinfo gvars[1] = {
    {6, "init_task", 0xffffffff81c15500},
};

struct memberinfo task_struct[3] = {
    {2, "pid", 0x304},
    {7, "comm", 0x4b0},
    {4, "tasks", 0x290},
};

struct memberinfo list_head[2] = {
    {5, "next", 0x0},
    {5, "prev", 0x8},
};

struct typeinfo types[8] = {
    {base_type, _VOID, 8},
    {base_type, _ADDR_T, 8},
    {base_type, _INT, 4},
    {base_type, _CHAR, 1},
    {structure_type, _LIST_HEAD, 16, .memnum=2, .mem=list_head},
    {pointer_type, _LIST_HEAD, 8, .saki=4, .pcount=1},
    {structure_type, _TASK_STRUCT, 2216, .memnum=3, .mem=task_struct},
    {array_type, _CHAR, 16, .saki=3, .arraysize=16},
};
