#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _LONG_UNSIGNED_INT      0x0
#define _UNSIGNED_CHAR          0x1
#define _SHORT_UNSIGNED_INT     0x2
#define _UNSIGNED_INT           0x3
#define _SIGNED_CHAR            0x4
#define _SHORT_INT              0x5
#define _INT                    0x6
#define _LONG_INT               0x7
#define _SIZETYPE               0x8
#define _CHAR                   0x9
#define _LONG_LONG_INT          0xa
#define _LONG_LONG_UNSIGNED_INT 0xb
#define _STRUCT_PERSON          0xc
#define _STRUCT_KYUUYO          0xd
#define _STRUCT_SYAIN_DT        0xe
#define _STRUCT_CELL            0xf
#define _DOUBLE                 0x10
#define _FLOAT                  0x11

struct gvarinfo {
    int tidx;
    char* name;
    long addr;
};

struct memberinfo {
    int tidx;
    char* name;
    long offset;
};

enum type {
    base,
    pointer,
    array,
    structure
};

struct typeinfo {
    enum type kind;
    int tbit;
    int bytesize;
    union {
        int saki; //pointer
        int memnum; //structure
    };
    union {
        int pcount; //pointer
        int arraysize; //array
        struct memberinfo* mem; //structure
    };
};

char* get_typename(int tbit);
extern struct gvarinfo gvars[15];
extern struct memberinfo person[7];
extern struct memberinfo kyuuyo[4];
extern struct memberinfo syain_dt[6];
extern struct memberinfo cell[2];
extern struct typeinfo types[30];

#endif