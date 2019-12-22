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
#define _UNION_QUIZ             0xe
#define _ENUM_COLOR             0xf
#define _STRUCT_PERSON          0x10
#define _STRUCT_KYUUYO          0x11
#define _STRUCT_SYAIN_DT        0x12
#define _STRUCT_CELL            0x13
#define _DOUBLE                 0x14
#define _FLOAT                  0x15

#define _TASK_STRUCT            0x16
#define _LIST_HEAD              0x17

#define _ADDR_T                 0x18



#define GVARS_SIZE 1

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
    base_type,
    pointer_type,
    array_type,
    structure_type,
    union_type,
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

void print_base(long data, int tbit);
char* get_typename(int tbit);
extern struct gvarinfo gvars[1];
extern struct memberinfo task_struct[3];
extern struct memberinfo list_head[2];
extern struct typeinfo types[8];

#endif
