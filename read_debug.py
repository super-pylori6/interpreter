# -*- coding: utf-8 -*-

import re # 正規表現
import pprint # リストや辞書を見やすく表示

TAG = 0
BASE_TYPE = 1
POINTER_TYPE = 2
ARRAY_TYPE = 3
SUBRANGE_TYPE = 4
TYPEDEF = 5
STRUCTURE_TYPE = 6
MEMBER = 7
VARIABLE = 8
NONE = 100
MODE = TAG
TYPE_GET = 0

datatype_dict = {}
struct_list_all = []
globalvar_list_all = []
offset_num_dict = {}

READFILE = "target.debug"
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

def getglobalvarname(line):
    globalvarname_m = re.search("\<.*\>", line.rstrip('\n'))
    globalvarname = re.sub("[\<\>]", "", globalvarname_m.group())
    return globalvarname


def print_globalvar():
    print("struct globalvar globalvars[" + str(len(globalvar_list_all)) + "] = {")
    for l in globalvar_list_all:
        print("    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}", end="")
        if l != globalvar_list_all[-1]: print(",")
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
    print("struct typeinfo types[" + str(len(datatype_dict)) + "] = {")
    for k, v in datatype_dict.items():
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
        if k != list(datatype_dict.keys())[-1]: print(",")
    print("\n};\n")

def write_infoc():
    info_c = []
    info_c.append("#include \"" + WRITEFILE2 +"\"") 
    info_c.append("") 
    info_c.append("struct globalvar globalvars[" + str(len(globalvar_list_all)) + "] = {")
    for l in globalvar_list_all:
        s = "    {" + str(l[0]) + ", " + "\"" + l[1] + "\"" + ", " + l[2] + "}"
        if l != globalvar_list_all[-1]:
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
        
    info_c.append("struct typeinfo types[" + str(len(datatype_dict)) + "] = {")
    for k, v in datatype_dict.items():
        if v[0] == "base":
            s = "    {" + v[0] + ", \"" + v[1] + "\", " + str(v[2]) + "}"
        elif v[0] == "pointer":
            s = "    {" + v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .saki=" + str(offset_num_dict[v[3]]) + ", .pcount=" + str(v[4]) + "}"
        elif v[0] == "array":
            s = "    {" + v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .arraysize=" + str(v[3]) + "}"
        elif v[0] == "structure":
            for l in struct_list_all:
                if l[0] == v[1].split(" ")[1]:
                    s = "    {" + v[0] + ", \"" + v[1] + "\", " + str(v[2]) + ", .memnum=" + str(len(l)-1) + ", .mem=" + v[1].split(" ")[1] + "}"
        if k != list(datatype_dict.keys())[-1]:
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
    info_h.append("struct globalvar {")
    info_h.append("    int typenum;")
    info_h.append("    char* name;")
    info_h.append("    long addr;")
    info_h.append("};")
    info_h.append("")
    info_h.append("struct memberinfo {")
    info_h.append("    int typenum;")
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
    info_h.append("    char* name;")
    info_h.append("    int bytesize;")
    info_h.append("    union {")
    info_h.append("        int saki;")
    info_h.append("        int memnum;")
    info_h.append("        int arraysize;")
    info_h.append("    };")
    info_h.append("    union {")
    info_h.append("        int pcount;")
    info_h.append("        struct memberinfo* mem;")
    info_h.append("    };")
    info_h.append("};")
    info_h.append("")
    info_h.append("extern struct globalvar globalvars[" + str(len(globalvar_list_all)) + "];")
    for l in struct_list_all:
        info_h.append("extern struct memberinfo " + l[0] + "[" + str(len(l)-1) + "];")
    info_h.append("extern struct typeinfo types[" + str(len(datatype_dict)) + "];")
    info_h.append("")
    info_h.append("#endif")

    with open(WRITEFILE2, mode="w") as f:
        f.writelines("\n".join(info_h))
    
"""
# .debugファイルをそのまま表示
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        print line.rstrip('\n')
"""


# 型情報の辞書の作成
with open(READFILE, "r", encoding="utf-8_sig") as debug_info:
    for line in debug_info:
        offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
        if offset_m:
            tag = gettag(line)
            #print addr, tag

            # タグに応じたモードを設定して次の行へ
            if tag == "base_type":
                MODE = BASE_TYPE
                offset = getoffset(line)
                size = name = None
            elif tag == "pointer_type":
                MODE = POINTER_TYPE
                offset = getoffset(line)
                size = datatype = None
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
            else:
                MODE = None
            continue

        elif MODE == BASE_TYPE:
            if re.match(" *byte_size ", line):
                size = getsize(line)
            elif re.match(" *name ", line):
                name = getname(line)

            if size is not None and name is not None:
                tag = re.sub("_type", "", tag)
                datatype_dict[offset] = [tag, name, size]
                #print(datatype_dict[offset])

        elif MODE == POINTER_TYPE:
            if re.match(" *byte_size ", line):
                size = getsize(line)
            elif re.match(" *type ", line):
                datatype = gettype(line)

            if size is not None and datatype is not None:
                tag = re.sub("_type", "", tag)
                pcount = 0
                datatype_dict[offset] = [tag, datatype, size, datatype, pcount]

        elif MODE == ARRAY_TYPE:
            if re.match(" *type ", line):
                datatype = gettype(line)

            if datatype is not None:
                tag = re.sub("_type", "", tag)
                datatype_dict[offset] = [tag, datatype]

        elif MODE == SUBRANGE_TYPE:
            if re.match(" *upper_bound ", line):
                size = getarraysize(line)

            if size is not None and datatype is not None:
                datatype_dict[offset].append(0)
                datatype_dict[offset].append(size)

        elif MODE == TYPEDEF:
            if re.match(" *type ", line):
                datatype = gettype(line)

            if datatype is not None:
                datatype_dict[offset] = [tag, datatype]

        elif MODE == STRUCTURE_TYPE:
            if re.match(" *name ", line):
                name = getname(line)
            elif re.match(" *byte_size ", line):
                size = getsize(line)

            if name is not None and size is not None:
                tag = re.sub("_type", "", tag)
                datatype_dict[offset] = [tag, "struct " + name, size]
        else:
            continue
#pprint.pprint(datatype_dict)

# 辞書の更新
for k in datatype_dict.keys():
    if datatype_dict[k][0] == "typedef":
        datatype_dict[k] = datatype_dict[datatype_dict[k][1]]
    elif datatype_dict[k][0] == "pointer":
        if datatype_dict[datatype_dict[k][1]][0] == "pointer":
            datatype_dict[k][4] = datatype_dict[datatype_dict[k][1]][4] + 1
        else:
            datatype_dict[k][4] += 1
        datatype_dict[k][1] = datatype_dict[datatype_dict[k][1]][1]
    elif datatype_dict[k][0] == "array":
        datatype_dict[k][2] = datatype_dict[k][3] * datatype_dict[datatype_dict[k][1]][2]
        datatype_dict[k][1] = datatype_dict[datatype_dict[k][1]][1]
        
#pprint.pprint(datatype_dict)


# オフセットと配列の添字の対応を作成
for i, k in enumerate(datatype_dict.keys()):
    offset_num_dict[k] = i
#pprint.pprint(offset_num_dict)


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
                globalvarname = getglobalvarname(line)
                globalvar_list = [offset_num_dict[vartype_offset], globalvarname, addr]
                globalvar_list_all.append(globalvar_list)
                TYPE_GET = 0
                continue

#print_globalvar()
#print_memberinfo()
#print_typeinfo()

write_infoh()
#with open("info.h") as f: print(f.read())

write_infoc()
#with open("info.c") as f: print(f.read())

print(READFILE)
print("├─>" + WRITEFILE1)
print("└─>" + WRITEFILE2)
#pprint.pprint(struct_list_all)
#pprint.pprint(globalvar_list_all)


