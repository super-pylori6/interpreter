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
  default:
    return "";
  }
}

struct gvarinfo gvars[1] = {
    {6, "init_task", 0xffffffff81c17480},
};

struct memberinfo task_struct[3] = {
    {1, "tasks", 0x270},
    {3, "pid", 0x2f0},
    {5, "comm", 0x4c0},
};

struct memberinfo list_head[2] = {
    {2, "next", 0},
    {2, "prev", 8},
};

struct typeinfo types[7] = {
    {base, _VOID, 8},
    {structure, _STRUCT_LIST_HEAD, 16, .memnum=2, .mem=list_head},
    {pointer, _STRUCT_LIST_HEAD, 8, .saki=1, .pcount=1},
    {base, _INT, 4},
    {base, _CHAR, 1},
    {array, _CHAR, 16, .saki=4, .arraysize=16},
    {structure, _STRUCT_TASK_STRUCT, 6784, .memnum=3, .mem=task_struct},
};

