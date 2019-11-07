#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _LONG_UNSIGNED_INT:
    return "long unsigned int";
  case _UNSIGNED_CHAR:
    return "unsigned char";
  case _SHORT_UNSIGNED_INT:
    return "short unsigned int";
  case _UNSIGNED_INT:
    return "unsigned int";
  case _SIGNED_CHAR:
    return "signed char";
  case _SHORT_INT:
    return "short int";
  case _INT:
    return "int";
  case _LONG_INT:
    return "long int";
  case _SIZETYPE:
    return "sizetype";
  case _CHAR:
    return "char";
  case _LONG_LONG_INT:
    return "long long int";
  case _LONG_LONG_UNSIGNED_INT:
    return "long long unsigned int";
  case _STRUCT_PERSON:
    return "struct person";
  case _STRUCT_KYUUYO:
    return "struct kyuuyo";
  case _STRUCT_SYAIN_DT:
    return "struct syain_dt";
  case _STRUCT_CELL:
    return "struct cell";
  case _DOUBLE:
    return "double";
  case _FLOAT:
    return "float";
  default:
    return "";
  }
}

struct gvarinfo gvars[15] = {
    {6, "ggg", 0x601060},
    {7, "hhh", 0x601068},
    {12, "p", 0x6011c0},
    {12, "q", 0x6011c8},
    {10, "c", 0x601070},
    {27, "ca", 0x601078},
    {9, "cp", 0x601088},
    {28, "ia", 0x601090},
    {17, "tanaka", 0x6010a0},
    {17, "ito", 0x6010e0},
    {19, "tokyo", 0x601120},
    {22, "c1", 0x601180},
    {22, "c3", 0x601190},
    {22, "c2", 0x6011a0},
    {23, "top", 0x6011b0}
};

struct memberinfo person[7] = {
    {9, "name", 0},
    {10, "sex", 8},
    {6, "age", 12},
    {9, "add", 16},
    {9, "job", 24},
    {15, "tmp", 32},
    {16, "zzz", 36}
};

struct memberinfo kyuuyo[4] = {
    {7, "kihon", 0},
    {7, "jyutaku", 8},
    {7, "kazoku", 16},
    {7, "sikaku", 24}
};

struct memberinfo syain_dt[6] = {
    {7, "no", 0},
    {11, "name", 8},
    {20, "yaku", 28},
    {6, "nensu", 40},
    {18, "kyu", 48},
    {21, "pp", 80}
};

struct memberinfo cell[2] = {
    {6, "val", 0},
    {23, "next", 8}
};

struct typeinfo types[29] = {
    {base, 0x1, 8},
    {base, 0x2, 1},
    {base, 0x4, 2},
    {base, 0x8, 4},
    {base, 0x10, 1},
    {base, 0x20, 2},
    {base, 0x40, 4},
    {base, 0x80, 8},
    {base, 0x100, 8},
    {pointer, 0x400, 8, .saki=10, .pcount=1},
    {base, 0x400, 1},
    {array, 0x400, 20, .saki=10, .arraysize=20},
    {pointer, 0x40, 8, .saki=6, .pcount=1},
    {base, 0x2000, 8},
    {base, 0x4000, 8},
    {base, 0x40, 4},
    {base, 0x40, 4},
    {structure, 0x20000, 40, .memnum=7, .mem=person},
    {structure, 0x40000, 32, .memnum=4, .mem=kyuuyo},
    {structure, 0x80000, 88, .memnum=6, .mem=syain_dt},
    {array, 0x400, 10, .saki=10, .arraysize=10},
    {pointer, 0x20000, 8, .saki=17, .pcount=1},
    {structure, 0x400000, 16, .memnum=2, .mem=cell},
    {pointer, 0x400000, 8, .saki=22, .pcount=1},
    {base, 0x1000000, 8},
    {base, 0x2000000, 4},
    {array, 0x80000, 1760, .saki=19, .arraysize=20},
    {array, 0x400, 13, .saki=10, .arraysize=13},
    {array, 0x40, 16, .saki=6, .arraysize=4}
};
