#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _VOID                   0x0
#define _ADDR_T                 0x1
#define _INT                    0x2
#define _CHAR                   0x3
#define _LIST_HEAD              0x4
#define _TASK_STRUCT            0x5

enum type {
    base_type,
    pointer_type,
    array_type,
    structure_type,
    union_type,
    enumeration,
};

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