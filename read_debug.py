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
unionl_all = []
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
    return addr_m.group()

def getgvarname(line):
    gvarname_m = re.search("\<.*\>", line.rstrip('\n'))
    gvarname = re.sub("[\<\>]", "", gvarname_m.group())
    return gvarname

def print_gvar():
    print("struct gvar gvars[" + str(len(gvarl_all)) + "] = {")
    for l in gvarl_all:
        print("    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}", end="")
        if l != gvarl_all[-1]: print(",")
        else: print("\n};\n")

def print_memberinfo():
    for l in structl_all:
        print("struct memberinfo " + l[0] + "[" + str(len(l)-1) + "] = {")
        for m in l[1:]:
            print("    {" + str(m[0])  + ", " + "\"" + m[1] + "\"" + ", " +  str(m[2]) + "}", end="")
            if m != l[-1]: print(",")
            else: print("")
        print("};\n")

def print_structinfo():
    print("struct structinfo {")
    print("    char* name;")
    print("    int membernum;")
    print("    struct memberinfo* member;")
    print("};\n")

    print("struct structinfo structinfos[] = {")
    for l in structl_all:
        print("    {" + "\"" + l[0] + "\""  + ", " + str(len(l)-1) + ", " + l[0] + "}", end="")
        if l != structl_all[-1]: print(",")
        else: print("\n};\n")

def print_typeinfo():
    print("struct typeinfo types[" + str(len(typeD)) + "] = {")
    for k, v in typeD.items():
        print("    {", end="")
        if v[0] == "base":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + "}", end="")
        elif v[0] == "pointer":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .saki=" + str(numD[v[3]]) + "}", end="")
        elif v[0] == "array":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .arraysize=" + str(v[3]) + "}", end="")
        elif v[0] == "structure":
            for l in structl_all:
                if l[0] == v[1].split(" ")[1]:
                    print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .memnum=" + str(len(l)-1) + ", .mem=" + v[1].split(" ")[1] + "}", end="")
        if k != list(typeD.keys())[-1]: print(",")
    print("\n};\n")

def write_infoc():
    info_c = []
    info_c.append("#include \"" + WRITEFILE2 +"\"") 
    info_c.append("")

    info_c.append("char* get_typename(int tbit){")
    info_c.append("  switch(tbit){")

    info_c.append("  case _FUNCTION:")
    info_c.append("    return \"function\";")
    info_c.append("  case _VOID:")
    info_c.append("    return \"void\";")
    
    for k in p_bitD.keys():
        if typeD[k][1] == "function" or typeD[k][1] == "void":
            continue
        s = "  case _" + typeD[k][1].upper().replace(' ', '_') + ":"
        info_c.append(s)
        s = typeD[k][1]
        info_c.append("    return \"" + typeD[k][1] + "\";")
        
    info_c.append("  default:")
    info_c.append("    return \"\";")
    info_c.append("  }")
    info_c.append("}")
    info_c.append("")
        
    info_c.append("struct gvarinfo gvars[" + str(len(gvarl_all)) + "] = {")
    for l in gvarl_all:
        s = "    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}"
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
            for l in structl_all:
                if l[0] == v[1].split(" ")[1]:
                    s = "    {" + v[0] + ", "
                    s = s + "_" + v[1].upper().replace(' ', '_') + ", "
                    s = s + str(v[2]) + ", "
                    s = s + ".memnum=" + str(len(l)-1) + ", "
                    s = s + ".mem=" + v[1].split(" ")[1] + "}"
                    
        elif v[0] == "union":
            for l in unionl_all:
                if l[0] == v[1].split(" ")[1]:
                    s = "    {" + v[0][0:3] + ", "
                    s = s + "_" + v[1].upper().replace(' ', '_') + ", "
                    s = s + str(v[2]) + ", "
                    s = s + ".memnum=" + str(len(l)-1) + ", "
                    s = s + ".mem=" + v[1].split(" ")[1] + "}"

        elif v[0] == "enumeration":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + "}"
                    
        elif v[0] == "subroutine":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + "}"
        if k != list(typeD.keys())[-1]:
            info_c.append(s + ",")
        else:
            info_c.append(s)
    info_c.append("};")
    info_c.append("")


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
    info_h.append("char* get_typename(int tbit);")
    info_h.append("extern struct gvarinfo gvars[" + str(len(gvarl_all)) + "];")
    for l in structl_all:
        info_h.append("extern struct memberinfo " + l[0] + "[" + str(len(l)-1) + "];")
    info_h.append("extern struct typeinfo types[" + str(len(typeD)) + "];")
    info_h.append("")
    info_h.append("#endif")

    with open(WRITEFILE2, mode="w") as f:
        f.writelines("\n".join(info_h))

"""

.debug :: デバッグ情報が記述されたファイル

[base_type]
================================================================
 [    57]    base_type
             byte_size            (data1) 4
             encoding             (data1) signed (5)
             name                 (string) "int"
================================================================

[pointer_type]
================================================================
 [    6c]    pointer_type
             byte_size            (data1) 8
             type                 (ref4) [    72]
 [    72]    base_type
             byte_size            (data1) 1
             encoding             (data1) signed_char (6)
             name                 (strp) "char"
================================================================

[array_type]
================================================================
 [    79]    array_type
             type                 (ref4) [    72]
             sibling              (ref4) [    89]
================================================================

[subrange_type]
================================================================
 [    82]      subrange_type
               type                 (ref4) [    65]
               upper_bound          (data1) 19
================================================================

[const_type]
================================================================
 [    89]    const_type
             type                 (ref4) [    72]
================================================================

[typedef]
================================================================
 [    a2]    typedef
             name                 (strp) "newint"
             decl_file            (data1) 1
             decl_line            (data1) 20
             type                 (ref4) [    57]
 [    ad]    typedef
             name                 (strp) "newnewint"
             decl_file            (data1) 1
             decl_line            (data1) 21
             type                 (ref4) [    a2]
================================================================

[structure_type]
================================================================
 [    b8]    structure_type
             name                 (strp) "person"
             byte_size            (data1) 40
             decl_file            (data1) 1
             decl_line            (data1) 23
             sibling              (ref4) [   119]
================================================================

"""


print("make typeD")

typeD["0"] = ["base", "void", 8]
refD["0"]  = 0

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
                typeD[offset] = ["base", "function", 8]
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
                typeD[offset] = [tag, datatype]
                refD[offset] = 1

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

"""

typeD :: 辞書型 
- DIEオフセット : DIE情報

base_type [タグ, 型, バイトサイズ]
================================================================
 '50': ['base', 'short int', 2],
 '57': ['base', 'int', 4],
================================================================

pointer_type [タグ, 型の参照, バイトサイズ, 型の参照, ポインターの数]
================================================================
 '6c': ['pointer', '72', 8, '72', 0]
 '72': ['base', 'char', 1],
================================================================

array_type [タグ, 型の参照, バイトサイズ, 型の参照, 配列のサイズ]
================================================================
 '79': ['array', '72', 0, '72', 20],
================================================================

const_type [タグ, 型の参照]
================================================================
 '89': ['const', '72'],
================================================================

typedef [タグ, 型の参照]
================================================================
 'a2': ['typedef', '57'],
 'ad': ['typedef', 'a2'],
================================================================

structure_type [タグ, 型, バイトサイズ]
================================================================
 'b8': ['structure', 'struct person', 40]}
================================================================

"""

"""

refD :: 辞書型
- DIEオフセット : typeDの要素のリストに型の参照があるかのフラグ
- 参照がある場合1, ない場合0

================================================================
 '6c': 1,
 '72': 0,
 '79': 1,
================================================================



"""


print("make bitD")

# 型とビットの辞書作成
cnt = 2
for k in typeD.keys():
    if typeD[k][0] == "base":
        if typeD[k][1] == "function":
            bitD[k] = p_bitD[k] = hex(0)
        elif typeD[k][1] == "void":
            bitD[k] = p_bitD[k] = hex(1)
        else:
            bitD[k] = p_bitD[k] = hex(cnt)
            cnt = cnt + 1
    elif typeD[k][0] == "structure":
        bitD[k] = p_bitD[k] = hex(cnt)
        cnt = cnt + 1
    elif typeD[k][0] == "union":
        bitD[k] = p_bitD[k] = hex(cnt)
        cnt = cnt + 1
    elif typeD[k][0] == "enumeration":
        bitD[k] = p_bitD[k] = hex(cnt)
        cnt = cnt + 1
    elif typeD[k][0] == "subroutine":
        bitD[k] = p_bitD[k] = hex(0)



        
print("updata typeD")

# 辞書の更新
for k in typeD.keys():
    while refD[k]:
        if refD[typeD[k][1]]:
            typeD[k][1] = typeD[typeD[k][1]][1]
            continue

        bitD[k] = bitD[typeD[k][1]]
        
        if typeD[k][0] == "typedef":
            typeD[k] = typeD[typeD[k][1]]
        elif typeD[k][0] == "const":
            typeD[k] = typeD[typeD[k][1]]
        elif typeD[k][0] == "volatile":
            typeD[k] = typeD[typeD[k][1]]
        elif typeD[k][0] == "pointer":
            if typeD[typeD[k][1]][0] == "pointer":
                #print(k, typeD[k])
                typeD[k][4] = typeD[typeD[k][1]][4] + 1
            else:
                typeD[k][4] += 1
            typeD[k][1] = typeD[typeD[k][1]][1]
        elif typeD[k][0] == "array":
            typeD[k][2] = typeD[k][4] * typeD[typeD[k][1]][2]
            typeD[k][1] = typeD[typeD[k][1]][1]
        refD[k] = 0

#pprint.pprint(typeD)

#sys.exit()

"""

typeD :: 辞書型

2つ目の要素は型に更新されており，4つ目の要素は

================================================================
 '6c': ['pointer', 'char', 8, '72', 1],
 '72': ['base', 'char', 1],
 '79': ['array', 'char', 20, '72', 20],
================================================================


"""

print("make numD")

# オフセットと配列の添字の対応を作成
#numD["0"] = 0
for i, k in enumerate(typeD.keys()):
    numD[k] = i
#pprint.pprint(numD)


print("make structl and unionl")

# 構造体情報の取得
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "structure_type":
                MODE = TSTRUCT
                offset = getoffset(line)
            elif tag == "union_type":
                MODE = TUNION
                offset = getoffset(line)
            elif tag == "member" and (MODE == TSTRUCT or MODEM == TSMEM):
                MODE = TMEM
                MODEM = TSMEM
                structl.append([0, 0, "0"])
            elif tag == "member" and (MODE == TUNION or MODEM == TUMEM):
                MODE = TMEM
                MODEM = TUMEM
                unionl.append([0, 0, "0"])
            elif tag == "subprogram":
                break
            else:
                MODE = NONE
            continue

        elif MODE == TSTRUCT:
            if re.match(" *name ", line):
                structname = getname(line)
                structl = [structname]
                structl_all.append(structl)
            continue

        elif MODE == TUNION:
            if re.match(" *name ", line):
                unionname = getname(line)
                unionl = [unionname]
                unionl_all.append(unionl)
            continue

        elif MODE == TMEM and MODEM == TSMEM:
            if re.match(" *name ", line):
                mname = getname(line)
                structl[-1][1] = mname
            elif re.match(" *type ", line):
                mtype = gettype(line)
            elif re.match(" *data_member_location ", line):
                moffset_hex = re.sub(".*\(.*?\) ", "", line.rstrip('\n'))
                moffset = int(moffset_hex, 16)
                structl[-1][0] = numD[mtype]
                structl[-1][2] = moffset
                #print(structl)
                
        elif MODE == TMEM and MODEM == TUMEM:
            if re.match(" *name ", line):
                mname = getname(line)
                unionl[-1][1] = mname
            elif re.match(" *type ", line):
                mtype = gettype(line)
                unionl[-1][0] = numD[mtype]
                
#pprint.pprint(structl_all)
#pprint.pprint(unionl_all)

print("make gvarl")

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
                vartype_offset = gettype(line)
                TYPE_GET = 1
                continue

            elif re.match(" *\[.*\] addr", line) and TYPE_GET:
                addr = getaddr(line)
                gvarname = getgvarname(line)
                gvarl = [numD[vartype_offset], gvarname, addr]
                gvarl_all.append(gvarl)
                TYPE_GET = 0
                continue

#print_gvar()
#print_memberinfo()
#print_typeinfo()

print("start write_infoh")

write_infoh()
#with open("info.h") as f: print(f.read())

print("start write_infoc")

write_infoc()
#with open("info.c") as f: print(f.read())

print(READFILE)
print("├─>" + WRITEFILE1)
print("└─>" + WRITEFILE2)
#pprint.pprint(structl_all)
#pprint.pprint(gvarl_all)


