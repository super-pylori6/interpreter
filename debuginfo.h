#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _LONG_UNSIGNED_INT      0x1
#define _UNSIGNED_CHAR          0x2
#define _SHORT_UNSIGNED_INT     0x4
#define _UNSIGNED_INT           0x8
#define _SIGNED_CHAR            0x10
#define _SHORT_INT              0x20
#define _INT                    0x40
#define _LONG_INT               0x80
#define _SIZETYPE               0x100
#define _CHAR                   0x400
#define _LONG_LONG_INT          0x2000
#define _LONG_LONG_UNSIGNED_INT 0x4000
#define _STRUCT_PERSON          0x20000
#define _STRUCT_KYUUYO          0x40000
#define _STRUCT_SYAIN_DT        0x80000
#define _STRUCT_CELL            0x400000
#define _DOUBLE                 0x1000000
#define _FLOAT                  0x2000000

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
    char* name;
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
extern struct typeinfo types[29];

#endif