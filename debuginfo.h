#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

struct globalvar {
    int typenum;
    char* name;
    long addr;
};

struct memberinfo {
    int typenum;
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
    int bytesize;
    union {
        int saki;
        int memnum;
        int arraysize;
    };
    union {
        int pcount;
        struct memberinfo* mem;
    };
};

extern struct globalvar globalvars[10];
extern struct memberinfo person[7];
extern struct memberinfo kyuuyo[4];
extern struct memberinfo syain_dt[6];
extern struct typeinfo types[26];

#endif