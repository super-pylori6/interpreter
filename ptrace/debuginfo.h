#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _FUNCTION               0x0
#define _LONG_LONG_UNSIGNED_INT 0x2
#define _SIZETYPE               0x3
#define _STRUCT_KYUUYO          0x5
#define _STRUCT_SYAIN_DT        0x6
#define _VOID                   0x1
#define _STRUCT_PERSON          0x9
#define _UNSIGNED_INT           0x8
#define _UNSIGNED_CHAR          0xa
#define _LONG_INT               0xb
#define _STRUCT_CELL            0x7
#define _LONG_UNSIGNED_INT      0xd
#define _DOUBLE                 0xc
#define _ENUM_COLOR             0x11
#define _CHAR                   0xe
#define _SIGNED_CHAR            0xf
#define _INT                    0x10
#define _FLOAT                  0x4
#define _LONG_LONG_INT          0x12
#define _SHORT_UNSIGNED_INT     0x13
#define _SHORT_INT              0x14
#define _FUNCTION               0x0
#define _UNION_QUIZ             0x15

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
    uni,
    enumeration,
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
extern struct memberinfo syain_dt[8];
extern struct memberinfo cell[2];
extern struct typeinfo types[38];

#endif