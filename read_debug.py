# -*- coding: utf-8 -*-

import re # 正規表現
import pprint # リストや辞書を見やすく表示
import sys # コマンドライン引数

TAG = 0
TBASE = 1
TCONST = 2
TPOINTER = 3
TARRAY = 4
TSUBRANGE = 5
TTYPEDEF = 6
TSTRUCT = 7
TUNION = 8
TMEM = 9
TSMEM = 10
TUMEM = 11
TVAR = 12
TSUBROUTINE = 13
TVOLATILE = 14
TENUM = 15
NONE = 100
MODE = TAG

TYPE_GET = 0

MODEM = TAG

typeD = {} # 辞書：.debugのオフセットと
refD = {}
structl_all = []
structl = []
unionl_all = []
unionl = []
gvarl_all = []
numD = {}
bitD = {}
p_bitD = {}

args = sys.argv
if len(args) != 2:
    print("args error")
    sys.exit()

READFILE = args[1]
WRITEFILE1 = "debuginfo.c"
WRITEFILE2 = "debuginfo.h"

def getname(line):
    name_m = re.search("\".*\"", line.rstrip('\n'))
    name = re.sub("\"", "", name_m.group())
    return name

def gettype(line):
    typeoffset_m = re.search("\[.*?\]", line.rstrip('\n'))
    typeoffset = re.sub("[ \[\]]", "", typeoffset_m.group())
    return typeoffset

def getsize(line):
    return int(re.sub(".*\(.*?\) ", "", line.rstrip('\n')))

def getarraysize(line):
    size = int(re.sub(".*\(.*?\) ", "", line.rstrip('\n')))
    return size + 1

def gettag(line):
    return re.sub(" \[ *[0-9a-f]*\] *", "", line.rstrip('\n'))

def getoffset(line):
    return re.sub("[ \[\]]", "", offset_m.group())

def getaddr(line):
    addr_m = re.search("0x[0-9a-f]*", line.rstrip('\n'))
    if addr_m:
        return addr_m.group()
    else:
        return "0"

def getgvarname(line):
    gvarname_m = re.search("\<.*\>", line.rstrip('\n'))
    gvarname = re.sub("[\<\>]", "", gvarname_m.group())
    return gvarname

def write_infoc():
    info_c = []
    info_c.append("#include <stdio.h>") 
    info_c.append("#include \"" + WRITEFILE2 +"\"") 
    info_c.append("")

    info_c.append("char* get_typename(int tbit){")
    info_c.append("  switch(tbit){")

    s = "  case _FUNCTION:"
    s = s + " " * (len("LONG_LONG_UNSIGNED_INT") - len("function") + 1)
    s = s + "return \"function\";"
    info_c.append(s)

    s = "  case _VOID:"
    s = s + " " * (len("LONG_LONG_UNSIGNED_INT") - len("void") + 1)
    s = s + "return \"void\";"
    info_c.append(s)
    
    for k in p_bitD.keys():
        if typeD[k][1] == "function" or typeD[k][1] == "void":
            continue
        s = "  case _" + typeD[k][1].upper().replace(' ', '_') + ":"
        #info_c.append(s)
        s = s + " " * (len("LONG_LONG_UNSIGNED_INT") - len(typeD[k][1]) + 1) + "return \"" + typeD[k][1] + "\";"
        info_c.append(s)

    s = "  default:"
    s = s + " " * len("LONG_LONG_UNSIGNED_INT")
    s = s + "return \"\";"
    info_c.append(s)
    info_c.append("  }")
    info_c.append("}")
    info_c.append("")

    info_c.append("char* print_base(int tbit){")
    
    info_c.append("#define GVARS_SIZE " + str(len(gvarl_all)))
    info_c.append("")

    info_c.append("struct gvarinfo gvars[" + str(len(gvarl_all)) + "] = {")
    for l in gvarl_all:
        s = "    {" + str(l[0]) + ", "
        s = s + "\"" + l[1] + "\"" + ", "
        s = s + l[2] + "}"
        if l != gvarl_all[-1]:
            info_c.append(s + ",")
        else:
            info_c.append(s)
    info_c.append("};")
    info_c.append("")
    
    for l in structl_all:
        info_c.append("struct memberinfo " + l[0] + "[" + str(len(l)-1) + "] = {")
        for m in l[1:]:
            s = "    {" + str(m[0])  + ", " + "\"" + m[1] + "\"" + ", " +  str(m[2]) + "}"
            if m != l[-1]:
                info_c.append(s + ",")
            else:
                info_c.append(s)
        info_c.append("};")
        info_c.append("")

    for l in unionl_all:
        info_c.append("struct memberinfo " + l[0] + "[" + str(len(l)-1) + "] = {")
        for m in l[1:]:
            s = "    {" + str(m[0])  + ", " + "\"" + m[1] + "\"" + ", " +  str(m[2]) + "}"
            if m != l[-1]:
                info_c.append(s + ",")
            else:
                info_c.append(s)
        info_c.append("};")
        info_c.append("")

    info_c.append("struct typeinfo types[" + str(len(typeD)) + "] = {")
    for k, v in typeD.items():
        if v[0] == "base":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + "}"
            
        elif v[0] == "pointer":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + ", "
            s = s + ".saki=" + str(numD[v[3]]) + ", "
            s = s + ".pcount=" + str(v[4]) + "}"
            
        elif v[0] == "array":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + ", "
            s = s + ".saki=" + str(numD[v[3]]) + ", "
            s = s + ".arraysize=" + str(v[4]) + "}"
            
        elif v[0] == "structure":
            for i, l in enumerate(structl_all):
                if " " in v[1] and l[0] != v[1].split(" ")[1]:
                        continue
                elif " " not in v[1] and l[0] != v[1]:
                        continue
                    
                s = "    {" + v[0] + ", "
                s = s + "_" + v[1].upper().replace(' ', '_') + ", "
                s = s + str(v[2]) + ", "
                s = s + ".memnum=" + str(len(l)-1) + ", "
                s = s + ".mem=" + v[1].split(" ")[1] + "}"
                break
                
        elif v[0] == "union":
            for i, l in enumerate(unionl_all):
                if " " in v[1] and l[0] != v[1].split(" ")[1]:
                        continue
                elif " " not in v[1] and l[0] != v[1]:
                        continue

                s = "    {" + v[0][0:3] + ", "
                s = s + "_" + v[1].upper().replace(' ', '_') + ", "
                s = s + str(v[2]) + ", "
                s = s + ".memnum=" + str(len(l)-1) + ", "
                s = s + ".mem=" + v[1].split(" ")[1] + "}"
                break
                
        elif v[0] in {"enumeration", "subroutine"}:
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + "}"
                                
        if k != list(typeD.keys())[-1]:
            info_c.append(s + ",")
        else:
            info_c.append(s)
    info_c.append("};")
    info_c.append("")

    print("write " + WRITEFILE1)
    with open(WRITEFILE1, mode="w") as f:
        f.writelines("\n".join(info_c))

def write_infoh():
    info_h = []
    info_h.append("#ifndef _" + WRITEFILE2.replace(".", "_").upper())
    info_h.append("#define _" + WRITEFILE2.replace(".", "_").upper())
    info_h.append("")

    info_h.append("#define _FUNCTION" + " " * (len("LONG_LONG_UNSIGNED_INT") - len("FUNCTION") + 1) +"0x0")
    for k in p_bitD.keys():
        s = "#define "
        s = s + "_" + typeD[k][1].upper().replace(' ', '_') + " " * (len("LONG_LONG_UNSIGNED_INT") - len(typeD[k][1]) + 1)
        s = s + p_bitD[k]
        info_h.append(s)
    info_h.append("")
    
    info_h.append("struct gvarinfo {")
    info_h.append("    int tidx;")
    info_h.append("    char* name;")
    info_h.append("    long addr;")
    info_h.append("};")
    info_h.append("")
    info_h.append("struct memberinfo {")
    info_h.append("    int tidx;")
    info_h.append("    char* name;")
    info_h.append("    long offset;")
    info_h.append("};")
    info_h.append("")
    info_h.append("enum type {")
    info_h.append("    base,")
    info_h.append("    pointer,")
    info_h.append("    array,")
    info_h.append("    structure,")
    info_h.append("    uni,")
    info_h.append("    enumeration,")
    info_h.append("};")
    info_h.append("")
    info_h.append("struct typeinfo {")
    info_h.append("    enum type kind;")
    info_h.append("    int tbit;")
    info_h.append("    int bytesize;")
    info_h.append("    union {")
    info_h.append("        int saki; //pointer")
    info_h.append("        int memnum; //structure, union")
    info_h.append("    };")
    info_h.append("    union {")
    info_h.append("        int pcount; //pointer")
    info_h.append("        int arraysize; //array")
    info_h.append("        struct memberinfo* mem; //structure, union")
    info_h.append("    };")
    info_h.append("};")
    info_h.append("")
    info_h.append("void print_base(long data, int tbit);")
    info_h.append("char* get_typename(int tbit);")
    info_h.append("extern struct gvarinfo gvars[" + str(len(gvarl_all)) + "];")
    for l in structl_all:
        info_h.append("extern struct memberinfo " + l[0] + "[" + str(len(l)-1) + "];")
    info_h.append("extern struct typeinfo types[" + str(len(typeD)) + "];")
    info_h.append("")
    info_h.append("#endif")

    with open(WRITEFILE2, mode="w") as f:
        f.writelines("\n".join(info_h))



#print("make typeD")


typeD = {"0" : ["base", "void", 8]}
refD  = {"0" : 0}

# 型情報の辞書の作成

# 型情報の辞書の作成
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "base_type":
                MODE = TBASE
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, "void", 8]
                refD[offset] = 0
            elif tag == "const_type":
                MODE = TCONST
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, "0", 8]
                refD[offset] = 1
            elif tag == "volatile_type":
                MODE = TVOLATILE
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, "0", 8]
                refD[offset] = 1
            elif tag == "typedef" :
                MODE = TTYPEDEF
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, "0", 8]
                refD[offset] = 1
            elif tag == "pointer_type":
                MODE = TPOINTER
                offset = getoffset(line)
                datatype = "void"
                size = 8
                tag = re.sub("_type", "", tag)
                pcnt = 0
                typeD[offset] = [tag, "0", size, "0", pcnt]
                refD[offset] = 1
            elif tag == "array_type":
                MODE = TARRAY
                offset = getoffset(line)
                size = 0
                tag = re.sub("_type", "", tag)
                datatype = "void"
                typeD[offset] = [tag, "0", size, "0", size]
                refD[offset] = 1
            elif tag == "subrange_type" :
                MODE = TSUBRANGE
            elif tag == "structure_type":
                MODE = TSTRUCT
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                size = 0
                typeD[offset] = [tag, "void", size]
                refD[offset] = 0
            elif tag == "union_type":
                MODE = TUNION
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                size = 0
                typeD[offset] = [tag, "void", size]
                refD[offset] = 0
            elif tag == "enumeration_type":
                MODE = TENUM
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                size = 4
                typeD[offset] = [tag, "void", size]
                refD[offset] = 0
            elif tag == "subroutine_type":
                MODE = TSUBROUTINE
                offset = getoffset(line)
                typeD[offset] = ["subroutine", "function", 8]
                refD[offset] = 0

            else:
                MODE = NONE

        elif MODE == TBASE:
            if re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size
            elif re.match(" *name ", line):
                name = getname(line)
                typeD[offset][1] = name

        elif MODE in {TCONST, TVOLATILE, TTYPEDEF}:
            if re.match(" *type ", line):
                datatype = gettype(line)
                typeD[offset][1] = datatype
                refD[offset] = 1
            elif re.match(" *name ", line):
                name = getname(line)
                typeD[offset][2] = name

        elif MODE == TPOINTER:
            if re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size
            elif re.match(" *type ", line):
                datatype = gettype(line)
                typeD[offset][1] = typeD[offset][3] = datatype
                refD[offset] = 1

        elif MODE == TARRAY:
            if re.match(" *type ", line):
                datatype = gettype(line)
                typeD[offset][1] = typeD[offset][3] = datatype
                refD[offset] = 1

        elif MODE == TSUBRANGE:
            if re.match(" *upper_bound ", line):
                arraysize = getarraysize(line)
                typeD[offset][4] = arraysize
                
        elif MODE == TSTRUCT:
            if re.match(" *name ", line):
                name = getname(line)
                typeD[offset][1] = "struct " + name
            elif re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size

        elif MODE == TUNION:
            if re.match(" *name ", line):
                name = getname(line)
                typeD[offset][1] = "union " + name
            elif re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size

        elif MODE == TENUM:
            if re.match(" *name ", line):
                name = getname(line)
                typeD[offset][1] = "enum " + name
            elif re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size
                
        elif MODE == TSUBROUTINE:
            continue
        else:
            continue

#pprint.pprint(typeD)
#pprint.pprint(refD)
#sys.exit()


     
#print("updata typeD")

# 辞書の更新
for k in typeD.keys():
    while refD[k]:
        ref = typeD[k][1]
        if refD[ref]:
            typeD[k][1] = typeD[ref][1]
            continue
        
        if typeD[k][0] == "typedef":
            if typeD[ref][0] in {"structure", "union"}:
                #print(typeD[k])
                if typeD[ref][1] == "void":
                    typeD[ref][1] = typeD[k][2]
            typeD[k] = typeD[ref]
        elif typeD[k][0] in {"const", "volatile"}:
            typeD[k] = typeD[ref]
        elif typeD[k][0] == "pointer":
            if typeD[ref][0] == "pointer":
                #print(k, typeD[k])
                typeD[k][4] = typeD[ref][4] + 1
            else:
                typeD[k][4] += 1
            typeD[k][1] = typeD[ref][1]
        elif typeD[k][0] == "array":
            typeD[k][2] = typeD[k][4] * typeD[ref][2]
            typeD[k][1] = typeD[ref][1]
        refD[k] = 0

#pprint.pprint(typeD)

#sys.exit()


#print("make gvarl")

# グローバル変数情報の取得
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "variable":
                MODE = TVAR
            else:
                MODE = NONE
            continue

        elif MODE == TVAR:
            if re.match(" *type ", line):
                datatype = gettype(line)
                TYPE_GET = 1
                continue

            elif re.match(" *\[.*\] addr", line) and TYPE_GET:
                addr = getaddr(line)
                name = getgvarname(line)
                if addr == '0': continue
                gvarl = [typeD[datatype][1], name, addr]
                #print(gvarl)
                gvarl_all.append(gvarl)
                TYPE_GET = 0
                continue



#print(gvarl)
#print_gvar()
#print_memberinfo()
#print_typeinfo()

#print("start write_infoc")

#write_infoc()
#with open("info.c") as f: print(f.read())

#print("start write_infoh")

#write_infoh()
#with open("info.h") as f: print(f.read())

#print(READFILE)
#print("├─>" + WRITEFILE1)
#print("└─>" + WRITEFILE2)
#pprint.pprint(structl_all)
#pprint.pprint(gvarl_all)




