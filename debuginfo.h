#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _FUNCTION               0x0
#define _VOID                   0x1
#define _LONG_UNSIGNED_INT      0x2
#define _UNSIGNED_CHAR          0x3
#define _SHORT_UNSIGNED_INT     0x4
#define _UNSIGNED_INT           0x5
#define _SIGNED_CHAR            0x6
#define _SHORT_INT              0x7
#define _INT                    0x8
#define _LONG_INT               0x9
#define _SIZETYPE               0xa
#define _CHAR                   0xb
#define _LONG_LONG_INT          0xc
#define _LONG_LONG_UNSIGNED_INT 0xd
#define _VOID                   0x1
#define _UNION_QUIZ             0xe
#define _STRUCT_PERSON          0xf
#define _STRUCT_KYUUYO          0x10
#define _STRUCT_SYAIN_DT        0x11
#define _FUNCTION               0x0
#define _STRUCT_CELL            0x12
#define _DOUBLE                 0x13
#define _FLOAT                  0x14

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
    structure,
    uni
};

struct typeinfo {
    enum type kind;
    int tbit;
    int bytesize;
    union {
        int saki; //pointer
        int memnum; //structure, union
    };
    union {
        int pcount; //pointer
        int arraysize; //array
        struct memberinfo* mem; //structure, union
    };
};

char* get_typename(int tbit);
extern struct gvarinfo gvars[16];
extern struct memberinfo person[7];
extern struct memberinfo kyuuyo[4];
extern struct memberinfo syain_dt[7];
extern struct memberinfo cell[2];
extern struct typeinfo types[37];

#endif