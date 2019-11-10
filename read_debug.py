# -*- coding: utf-8 -*-

import re # 正規表現
import pprint # リストや辞書を見やすく表示
import sys # コマンドライン引数

TAG = 0
BASE_TYPE = 1
CONST_TYPE = 2
POINTER_TYPE = 3
ARRAY_TYPE = 4
SUBRANGE_TYPE = 5
TYPEDEF = 6
STRUCTURE_TYPE = 7
MEMBER = 8
VARIABLE = 9
SUBROUTINE_TYPE = 10
NONE = 100
MODE = TAG
TYPE_GET = 0


typeD = {} # 辞書：.debugのオフセットと
refD = {}
struct_list_all = []
gvar_list_all = []
offset_num_dict = {}
type_bit_dict = {}
p_type_bit_dict = {}

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
    print("struct gvar gvars[" + str(len(gvar_list_all)) + "] = {")
    for l in gvar_list_all:
        print("    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}", end="")
        if l != gvar_list_all[-1]: print(",")
        else: print("\n};\n")

def print_memberinfo():
    for l in struct_list_all:
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
    for l in struct_list_all:
        print("    {" + "\"" + l[0] + "\""  + ", " + str(len(l)-1) + ", " + l[0] + "}", end="")
        if l != struct_list_all[-1]: print(",")
        else: print("\n};\n")

def print_typeinfo():
    print("struct typeinfo types[" + str(len(typeD)) + "] = {")
    for k, v in typeD.items():
        print("    {", end="")
        if v[0] == "base":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + "}", end="")
        elif v[0] == "pointer":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .saki=" + str(offset_num_dict[v[3]]) + "}", end="")
        elif v[0] == "array":
            print(v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .arraysize=" + str(v[3]) + "}", end="")
        elif v[0] == "structure":
            for l in struct_list_all:
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
    
    for k in p_type_bit_dict.keys():
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
        
    info_c.append("struct gvarinfo gvars[" + str(len(gvar_list_all)) + "] = {")
    for l in gvar_list_all:
        s = "    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}"
        if l != gvar_list_all[-1]:
            info_c.append(s + ",")
        else:
            info_c.append(s)
    info_c.append("};")
    info_c.append("")
    
    for l in struct_list_all:
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
            s = s + ".saki=" + str(offset_num_dict[v[3]]) + ", "
            s = s + ".pcount=" + str(v[4]) + "}"
        elif v[0] == "array":
            s = "    {" + v[0] + ", "
            s = s + "_" + v[1].upper().replace(' ', '_') + ", "
            s = s + str(v[2]) + ", "
            s = s + ".saki=" + str(offset_num_dict[v[3]]) + ", "
            s = s + ".arraysize=" + str(v[4]) + "}"
        elif v[0] == "structure":
            for l in struct_list_all:
                if l[0] == v[1].split(" ")[1]:
                    s = "    {" + v[0] + ", "
                    s = s + "_" + v[1].upper().replace(' ', '_') + ", "
                    s = s + str(v[2]) + ", "
                    s = s + ".memnum=" + str(len(l)-1) + ", "
                    s = s + ".mem=" + v[1].split(" ")[1] + "}"
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
    info_h.append("#define _VOID" + " " * (len("LONG_LONG_UNSIGNED_INT") - len("VOID") + 1) + "0x1")
    for k in p_type_bit_dict.keys():
        s = "#define _" + typeD[k][1].upper().replace(' ', '_') + " " * (len("LONG_LONG_UNSIGNED_INT") - len(typeD[k][1]) + 1) + p_type_bit_dict[k]
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
    info_h.append("    structure")
    info_h.append("};")
    info_h.append("")
    info_h.append("struct typeinfo {")
    info_h.append("    enum type kind;")
    info_h.append("    int tbit;")
    info_h.append("    int bytesize;")
    info_h.append("    union {")
    info_h.append("        int saki; //pointer")
    info_h.append("        int memnum; //structure")
    info_h.append("    };")
    info_h.append("    union {")
    info_h.append("        int pcount; //pointer")
    info_h.append("        int arraysize; //array")
    info_h.append("        struct memberinfo* mem; //structure")
    info_h.append("    };")
    info_h.append("};")
    info_h.append("")
    info_h.append("char* get_typename(int tbit);")
    info_h.append("extern struct gvarinfo gvars[" + str(len(gvar_list_all)) + "];")
    for l in struct_list_all:
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
refD["0"]   = 0
# 型情報の辞書の作成
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "base_type":
                MODE = BASE_TYPE
                offset = getoffset(line)
                size = name = None
            elif tag == "const_type":
                MODE = CONST_TYPE
                offset = getoffset(line)
                tag = re.sub("_type", "", tag)
                typeD[offset] = ["base", "void", 8]
                refD[offset] = 0
                #if offset == "b49": MODE = NONE
            elif tag == "pointer_type":
                MODE = POINTER_TYPE
                offset = getoffset(line)
                datatype = "void"
                size = 8
                tag = re.sub("_type", "", tag)
                pcnt = 0
                typeD[offset] = [tag, datatype, size, "0", pcnt]
                refD[offset] = 0
            elif tag == "array_type":
                MODE = ARRAY_TYPE
                offset = getoffset(line)
                size = datatype = None
            elif tag == "subrange_type" :
                MODE = SUBRANGE_TYPE
            elif tag == "typedef" :
                MODE = TYPEDEF
                offset = getoffset(line)
                datatype = None
            elif tag == "structure_type":
                MODE = STRUCTURE_TYPE
                offset = getoffset(line)
            elif tag == "subroutine_type":
                MODE = SUBROUTINE_TYPE
                offset = getoffset(line)
                typeD[offset] = ["base", "function", 8]
                refD[offset] = 0

            else:
                MODE = NONE

        elif MODE == BASE_TYPE:
            if re.match(" *byte_size ", line):
                size = getsize(line)
            elif re.match(" *name ", line):
                name = getname(line)

            if size is not None and name is not None:
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, name, size]
                refD[offset] = 0
                #print(typeD[offset])

        elif MODE == CONST_TYPE:
            if re.match(" *type ", line):
                datatype = gettype(line)
                refD[offset] = 1
                typeD[offset] = [tag, datatype]
                
        elif MODE == POINTER_TYPE:
            if re.match(" *byte_size ", line):
                size = getsize(line)
                typeD[offset][2] = size
            elif re.match(" *type ", line):
                datatype = gettype(line)
                typeD[offset][1] = typeD[offset][3] = datatype
                refD[offset] = 1

        elif MODE == ARRAY_TYPE:
            if re.match(" *type ", line):
                datatype = gettype(line)

            if datatype is not None:
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, datatype, datatype]
                refD[offset] = 1

        elif MODE == SUBRANGE_TYPE:
            if re.match(" *upper_bound ", line):
                size = getarraysize(line)

            if size is not None and datatype is not None:
                #typeD[offset].append(0)
                typeD[offset].insert(2, 0)
                #typeD[offset].append(size)
                typeD[offset].insert(4, size)
                
        elif MODE == TYPEDEF:
            if re.match(" *type ", line):
                datatype = gettype(line)

            if datatype is not None:
                typeD[offset] = [tag, datatype]
                refD[offset] = 1

        elif MODE == STRUCTURE_TYPE:
            if re.match(" *name ", line):
                name = getname(line)
            elif re.match(" *byte_size ", line):
                size = getsize(line)

            if name is not None and size is not None:
                tag = re.sub("_type", "", tag)
                typeD[offset] = [tag, "struct " + name, size]
                refD[offset] = 0

        elif MODE == SUBROUTINE_TYPE:
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


print("make type_bit_dict")

# 型とビットの辞書作成
cnt = 2
for k in typeD.keys():
    if typeD[k][0] == "base":
        if typeD[k][1] == "function":
            type_bit_dict[k] = p_type_bit_dict[k] = hex(0)
        elif typeD[k][1] == "void":
            type_bit_dict[k] = p_type_bit_dict[k] = hex(1)
        else:
            type_bit_dict[k] = p_type_bit_dict[k] = hex(cnt)
            cnt = cnt + 1
    elif typeD[k][0] == "structure":
        type_bit_dict[k] = p_type_bit_dict[k] = hex(cnt)
        cnt = cnt + 1
    elif typeD[k][0] == "subroutine":
        type_bit_dict[k] = p_type_bit_dict[k] = hex(0)



        
print("updata typeD")

# 辞書の更新
for k in typeD.keys():
    while refD[k]:
        if refD[typeD[k][1]]:
            typeD[k][1] = typeD[typeD[k][1]][1]
            continue

        type_bit_dict[k] = type_bit_dict[typeD[k][1]]
        
        if typeD[k][0] == "typedef":
            typeD[k] = typeD[typeD[k][1]]
        elif typeD[k][0] == "const":
            typeD[k] = typeD[typeD[k][1]]
        elif typeD[k][0] == "pointer":
            if typeD[typeD[k][1]][0] == "pointer":
                #print(k, typeD[k])
                typeD[k][4] = typeD[typeD[k][1]][4] + 1
            else:
                typeD[k][4] += 1
            typeD[k][1] = typeD[typeD[k][1]][1]
        elif typeD[k][0] == "array":
            print(k, typeD[k])
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

print("make offset_num_dict")

# オフセットと配列の添字の対応を作成
offset_num_dict["0"] = 0
for i, k in enumerate(typeD.keys()):
    offset_num_dict[k] = i
#pprint.pprint(offset_num_dict)


print("make struct_list")

# 構造体情報の取得
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "structure_type":
                MODE = STRUCTURE_TYPE
                offset = getoffset(line)
            elif tag == "member":
                MODE = MEMBER
            elif tag == "subprogram":
                break
            else:
                MODE = NONE
            continue

        elif MODE == STRUCTURE_TYPE:
            if re.match(" *name ", line):
                structname = getname(line)
                struct_list = [structname]
                struct_list_all.append(struct_list)
            continue

        elif MODE == MEMBER:
            if re.match(" *name ", line):
                membername = getname(line)

            elif re.match(" *type ", line):
                membertype_offset = gettype(line)

            elif re.match(" *data_member_location ", line):
                member_offset_hex = re.sub(".*\(.*?\) ", "", line.rstrip('\n'))
                member_offset = int(member_offset_hex, 16)
                struct_list.append([offset_num_dict[membertype_offset], membername, member_offset])

print("make gvar_list")

# グローバル変数情報の取得
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)

            # タグに応じたモードを設定して次の行へ
            if tag == "variable":
                MODE = VARIABLE
            else:
                MODE = NONE
            continue

        elif MODE == VARIABLE:
            if re.match(" *type ", line):
                vartype_offset = gettype(line)
                TYPE_GET = 1
                continue

            elif re.match(" *\[.*\] addr", line) and TYPE_GET:
                addr = getaddr(line)
                gvarname = getgvarname(line)
                gvar_list = [offset_num_dict[vartype_offset], gvarname, addr]
                gvar_list_all.append(gvar_list)
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
#pprint.pprint(struct_list_all)
#pprint.pprint(gvar_list_all)


