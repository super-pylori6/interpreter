#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _FUNCTION:               return "function";
  case _VOID:                   return "void";
  default:                      return "";
  }
}

struct gvarinfo gvars[16] = {
    {57, "ggg", 0x601060},
    {5e, "hhh", 0x601068},
    {90, "p", 0x6011d0},
    {90, "q", 0x6011d8},
    {36c, "ia", 0x601070},
    {8b, "c", 0x400678},
    {3a3, "ca", 0x601080},
    {6e, "cp", 0x601090},
    {ab, "q1", 0x601098},
    {10c, "tanaka", 0x6010a0},
    {10c, "ito", 0x6010e0},
    {1aa, "tokyo", 0x601120},
    {23f, "c1", 0x601190},
    {23f, "c3", 0x6011a0},
    {23f, "c2", 0x6011b0},
    {264, "top", 0x6011c0}
};

struct typeinfo types[0] = {
};
