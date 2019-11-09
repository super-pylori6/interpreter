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
    {13, "p", 0x6011c0},
    {13, "q", 0x6011c8},
    {12, "c", 0x400678},
    {28, "ca", 0x601070},
    {9, "cp", 0x601080},
    {29, "ia", 0x601090},
    {18, "tanaka", 0x6010a0},
    {18, "ito", 0x6010e0},
    {20, "tokyo", 0x601120},
    {23, "c1", 0x601180},
    {23, "c3", 0x601190},
    {23, "c2", 0x6011a0},
    {24, "top", 0x6011b0}
};

struct memberinfo person[7] = {
    {9, "name", 0},
    {10, "sex", 8},
    {6, "age", 12},
    {9, "add", 16},
    {9, "job", 24},
    {16, "tmp", 32},
    {17, "zzz", 36}
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
    {21, "yaku", 28},
    {6, "nensu", 40},
    {19, "kyu", 48},
    {22, "pp", 80}
};

struct memberinfo cell[2] = {
    {6, "val", 0},
    {24, "next", 8}
};

struct typeinfo types[30] = {
    {base, 0x0, 8},
    {base, 0x1, 1},
    {base, 0x2, 2},
    {base, 0x3, 4},
    {base, 0x4, 1},
    {base, 0x5, 2},
    {base, 0x6, 4},
    {base, 0x7, 8},
    {base, 0x8, 8},
    {pointer, 0x9, 8, .saki=10, .pcount=1},
    {base, 0x9, 1},
    {array, 0x9, 20, .saki=10, .arraysize=20},
    {base, 0x9, 1},
    {pointer, 0x6, 8, .saki=6, .pcount=1},
    {base, 0xa, 8},
    {base, 0xb, 8},
    {base, 0x6, 4},
    {base, 0x6, 4},
    {structure, 0xc, 40, .memnum=7, .mem=person},
    {structure, 0xd, 32, .memnum=4, .mem=kyuuyo},
    {structure, 0xe, 88, .memnum=6, .mem=syain_dt},
    {array, 0x9, 10, .saki=10, .arraysize=10},
    {pointer, 0xc, 8, .saki=18, .pcount=1},
    {structure, 0xf, 16, .memnum=2, .mem=cell},
    {pointer, 0xf, 8, .saki=23, .pcount=1},
    {base, 0x10, 8},
    {base, 0x11, 4},
    {array, 0xe, 1760, .saki=20, .arraysize=20},
    {array, 0x9, 13, .saki=10, .arraysize=13},
    {array, 0x6, 16, .saki=6, .arraysize=4}
};
