#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H

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

extern struct gvarinfo gvars[15];
extern struct memberinfo person[7];
extern struct memberinfo kyuuyo[4];
extern struct memberinfo syain_dt[6];
extern struct memberinfo cell[2];
extern struct typeinfo types[28];

#endif