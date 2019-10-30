#include "debuginfo.h"

struct globalvar globalvars[9] = {
    {6, "ggg", 0x601060},
    {7, "hhh", 0x601068},
    {20, "p", 0x601138},
    {21, "q", 0x601070},
    {10, "c", 0x601078},
    {22, "ca", 0x601080},
    {9, "cp", 0x601090},
    {14, "tanaka", 0x6010a0},
    {16, "tokyo", 0x6010e0}
};

struct memberinfo person[7] = {
    {9, "name", 0},
    {10, "sex", 8},
    {6, "age", 12},
    {9, "add", 16},
    {9, "job", 24},
    {12, "tmp", 32},
    {13, "zzz", 36}
};

struct memberinfo kyuuyo[4] = {
    {7, "kihon", 0},
    {7, "jyutaku", 8},
    {7, "kazoku", 16},
    {7, "sikaku", 24}
};

struct memberinfo syain_dt[5] = {
    {7, "no", 0},
    {11, "name", 8},
    {17, "yaku", 28},
    {6, "nensu", 40},
    {15, "kyu", 48}
};

struct typeinfo types[23] = {
    {base, "long unsigned int", 8},
    {base, "unsigned char", 1},
    {base, "short unsigned int", 2},
    {base, "unsigned int", 4},
    {base, "signed char", 1},
    {base, "short int", 2},
    {base, "int", 4},
    {base, "long int", 8},
    {base, "sizetype", 8},
    {pointer, "char", 8, .saki=10, .pcount=1},
    {base, "char", 1},
    {array, "char", 20, .arraysize=20},
    {base, "int", 4},
    {base, "int", 4},
    {structure, "struct person", 40, .memnum=7, .mem=person},
    {structure, "struct kyuuyo", 32, .memnum=4, .mem=kyuuyo},
    {structure, "struct syain_dt", 80, .memnum=5, .mem=syain_dt},
    {array, "char", 10, .arraysize=10},
    {base, "double", 8},
    {array, "struct syain_dt", 1600, .arraysize=20},
    {pointer, "int", 8, .saki=6, .pcount=1},
    {pointer, "int", 8, .saki=20, .pcount=2},
    {array, "char", 13, .arraysize=13}
};
