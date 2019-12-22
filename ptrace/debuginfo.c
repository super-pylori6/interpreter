#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _FUNCTION:
    return "function";
  case _VOID:
    return "void";
  case _LONG_LONG_UNSIGNED_INT:
    return "long long unsigned int";
  case _SIZETYPE:
    return "sizetype";
  case _STRUCT_KYUUYO:
    return "struct kyuuyo";
  case _STRUCT_SYAIN_DT:
    return "struct syain_dt";
  case _STRUCT_PERSON:
    return "struct person";
  case _UNSIGNED_INT:
    return "unsigned int";
  case _UNSIGNED_CHAR:
    return "unsigned char";
  case _LONG_INT:
    return "long int";
  case _STRUCT_CELL:
    return "struct cell";
  case _LONG_UNSIGNED_INT:
    return "long unsigned int";
  case _DOUBLE:
    return "double";
  case _ENUM_COLOR:
    return "enum color";
  case _CHAR:
    return "char";
  case _SIGNED_CHAR:
    return "signed char";
  case _INT:
    return "int";
  case _FLOAT:
    return "float";
  case _LONG_LONG_INT:
    return "long long int";
  case _SHORT_UNSIGNED_INT:
    return "short unsigned int";
  case _SHORT_INT:
    return "short int";
  case _UNION_QUIZ:
    return "union quiz";
  default:
    return "";
  }
}

struct gvarinfo gvars[16] = {
    {27, "ggg", 0x601060},
    {19, "hhh", 0x601068},
    {18, "p", 0x6011d0},
    {18, "q", 0x6011d8},
    {9, "ia", 0x601070},
    {4, "c", 0x400678},
    {35, "ca", 0x601080},
    {36, "cp", 0x601090},
    {37, "q1", 0x601098},
    {14, "tanaka", 0x6010a0},
    {14, "ito", 0x6010e0},
    {11, "tokyo", 0x601120},
    {12, "c1", 0x601190},
    {12, "c3", 0x6011a0},
    {12, "c2", 0x6011b0},
    {30, "top", 0x6011c0}
};

struct memberinfo person[7] = {
    {36, "name", 0},
    {24, "sex", 8},
    {27, "age", 12},
    {36, "add", 16},
    {36, "job", 24},
    {0, "tmp", 32},
    {25, "zzz", 36}
};

struct memberinfo kyuuyo[4] = {
    {19, "kihon", 0},
    {19, "jyutaku", 8},
    {19, "kazoku", 16},
    {19, "sikaku", 24}
};

struct memberinfo syain_dt[8] = {
    {19, "no", 0},
    {32, "name", 8},
    {17, "yaku", 28},
    {27, "nensu", 40},
    {8, "kyu", 48},
    {1, "pp", 80},
    {22, "func", 88},
    {28, "c", 96}
};

struct memberinfo cell[2] = {
    {27, "val", 0},
    {30, "next", 8}
};

struct memberinfo quiz[3] = {
    {27, "a", 0},
    {27, "b", 0},
    {27, "c", 0}
};

struct typeinfo types[38] = {
    {base, _INT, 4},
    {pointer, _STRUCT_PERSON, 8, .saki=14, .pcount=1},
    {pointer, _VOID, 8, .saki=6, .pcount=1},
    {base, _LONG_LONG_UNSIGNED_INT, 8},
    {base, _CHAR, 1},
    {base, _SIZETYPE, 8},
    {base, _VOID, 8},
    {base, _FLOAT, 4},
    {structure, _STRUCT_KYUUYO, 32, .memnum=4, .mem=kyuuyo},
    {array, _INT, 16, .saki=27, .arraysize=4},
    {pointer, _VOID, 8, .saki=15, .pcount=1},
    {structure, _STRUCT_SYAIN_DT, 104, .memnum=8, .mem=syain_dt},
    {structure, _STRUCT_CELL, 16, .memnum=2, .mem=cell},
    {base, _UNSIGNED_INT, 4},
    {structure, _STRUCT_PERSON, 40, .memnum=7, .mem=person},
    {base, _VOID, 8},
    {base, _UNSIGNED_CHAR, 1},
    {array, _CHAR, 10, .saki=24, .arraysize=10},
    {pointer, _INT, 8, .saki=27, .pcount=1},
    {base, _LONG_INT, 8},
    {base, _FUNCTION, 8},
    {base, _DOUBLE, 8},
    {pointer, _FUNCTION, 8, .saki=20, .pcount=1},
    {base, _LONG_UNSIGNED_INT, 8},
    {base, _CHAR, 1},
    {base, _INT, 4},
    {base, _SIGNED_CHAR, 1},
    {base, _INT, 4},
    {enumeration, _ENUM_COLOR, 4},
    {base, _LONG_LONG_INT, 8},
    {pointer, _STRUCT_CELL, 8, .saki=12, .pcount=1},
    {base, _SHORT_UNSIGNED_INT, 2},
    {array, _CHAR, 20, .saki=24, .arraysize=20},
    {base, _SHORT_INT, 2},
    {array, _STRUCT_SYAIN_DT, 2080, .saki=11, .arraysize=20},
    {array, _CHAR, 13, .saki=24, .arraysize=13},
    {pointer, _CHAR, 8, .saki=24, .pcount=1},
    {uni, _UNION_QUIZ, 4, .memnum=3, .mem=quiz}
};
