#include "debuginfo.h"

struct globalvar globalvars[10] = {
    {6, "ggg", 0x601060},
    {7, "hhh", 0x601068},
    {12, "p", 0x601180},
    {24, "q", 0x601070},
    {10, "c", 0x601078},
    {25, "ca", 0x601080},
    {9, "cp", 0x601090},
    {17, "tanaka", 0x6010a0},
    {17, "ito", 0x6010e0},
    {19, "tokyo", 0x601120}
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

struct typeinfo types[26] = {
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
    {pointer, "int", 8, .saki=6, .pcount=1},
    {base, "long long int", 8},
    {base, "long long unsigned int", 8},
    {base, "int", 4},
    {base, "int", 4},
    {structure, "struct person", 40, .memnum=7, .mem=person},
    {structure, "struct kyuuyo", 32, .memnum=4, .mem=kyuuyo},
    {structure, "struct syain_dt", 88, .memnum=6, .mem=syain_dt},
    {array, "char", 10, .arraysize=10},
    {pointer, "struct person", 8, .saki=17, .pcount=1},
    {base, "double", 8},
    {array, "struct syain_dt", 1760, .arraysize=20},
    {pointer, "int", 8, .saki=12, .pcount=2},
    {array, "char", 13, .arraysize=13}
};
