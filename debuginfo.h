#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

#define _FUNCTION               0x0

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
extern struct typeinfo types[0];

#endif