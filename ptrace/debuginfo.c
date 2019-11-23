#include "debuginfo.h"

char* get_typename(int tbit){
  switch(tbit){
  case _FUNCTION:
    return "function";
  case _VOID:
    return "void";
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
  case _UNION_QUIZ:
    return "union quiz";
  case _ENUM_COLOR:
    return "enum color";
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

struct gvarinfo gvars[16] = {
    {7, "ggg", 0x601060},
    {8, "hhh", 0x601068},
    {15, "p", 0x6011d0},
    {15, "q", 0x6011d8},
    {36, "ia", 0x601070},
    {14, "c", 0x400678},
    {37, "ca", 0x601080},
    {11, "cp", 0x601090},
    {20, "q1", 0x601098},
    {24, "tanaka", 0x6010a0},
    {24, "ito", 0x6010e0},
    {26, "tokyo", 0x601120},
    {31, "c1", 0x601190},
    {31, "c3", 0x6011a0},
    {31, "c2", 0x6011b0},
    {32, "top", 0x6011c0}
};

struct memberinfo person[7] = {
    {11, "name", 0},
    {12, "sex", 8},
    {7, "age", 12},
    {11, "add", 16},
    {11, "job", 24},
    {22, "tmp", 32},
    {23, "zzz", 36}
};

struct memberinfo kyuuyo[4] = {
    {8, "kihon", 0},
    {8, "jyutaku", 8},
    {8, "kazoku", 16},
    {8, "sikaku", 24}
};

struct memberinfo syain_dt[8] = {
    {8, "no", 0},
    {13, "name", 8},
    {27, "yaku", 28},
    {7, "nensu", 40},
    {25, "kyu", 48},
    {28, "pp", 80},
    {30, "func", 88},
    {21, "c", 96}
};

struct memberinfo cell[2] = {
    {7, "val", 0},
    {32, "next", 8}
};

struct memberinfo quiz[3] = {
    {7, "a", 0},
    {7, "b", 0},
    {7, "c", 0}
};

struct typeinfo types[38] = {
    {base, _VOID, 8},
    {base, _LONG_UNSIGNED_INT, 8},
    {base, _UNSIGNED_CHAR, 1},
    {base, _SHORT_UNSIGNED_INT, 2},
    {base, _UNSIGNED_INT, 4},
    {base, _SIGNED_CHAR, 1},
    {base, _SHORT_INT, 2},
    {base, _INT, 4},
    {base, _LONG_INT, 8},
    {base, _SIZETYPE, 8},
    {pointer, _VOID, 8, .saki=0, .pcount=1},
    {pointer, _CHAR, 8, .saki=12, .pcount=1},
    {base, _CHAR, 1},
    {array, _CHAR, 20, .saki=12, .arraysize=20},
    {base, _CHAR, 1},
    {pointer, _INT, 8, .saki=7, .pcount=1},
    {base, _LONG_LONG_INT, 8},
    {base, _LONG_LONG_UNSIGNED_INT, 8},
    {pointer, _VOID, 8, .saki=19, .pcount=1},
    {base, _VOID, 8},
    {uni, _UNION_QUIZ, 4, .memnum=3, .mem=quiz},
    {enumeration, _ENUM_COLOR, 4},
    {base, _INT, 4},
    {base, _INT, 4},
    {structure, _STRUCT_PERSON, 40, .memnum=7, .mem=person},
    {structure, _STRUCT_KYUUYO, 32, .memnum=4, .mem=kyuuyo},
    {structure, _STRUCT_SYAIN_DT, 104, .memnum=8, .mem=syain_dt},
    {array, _CHAR, 10, .saki=12, .arraysize=10},
    {pointer, _STRUCT_PERSON, 8, .saki=24, .pcount=1},
    {base, _FUNCTION, 8},
    {pointer, _FUNCTION, 8, .saki=29, .pcount=1},
    {structure, _STRUCT_CELL, 16, .memnum=2, .mem=cell},
    {pointer, _STRUCT_CELL, 8, .saki=31, .pcount=1},
    {base, _DOUBLE, 8},
    {base, _FLOAT, 4},
    {array, _STRUCT_SYAIN_DT, 2080, .saki=26, .arraysize=20},
    {array, _INT, 16, .saki=7, .arraysize=4},
    {array, _CHAR, 13, .saki=12, .arraysize=13}
};
