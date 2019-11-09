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
NONE = 100
MODE = TAG
TYPE_GET = 0

datatype_dict = {}
struct_list_all = []
gvar_list_all = []
offset_num_dict = {}
type_bit_dict = {}
p_type_bit_dict = {}

args = sys.argv
if len(args) != 2:
    print("args error")
    sys.exit()

#READFILE = "target.debug"
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

    info_c.append("char* get_typename(int tbit){")
    info_c.append("  switch(tbit){")
        
    for k in p_type_bit_dict.keys():
        s = "  case _" + datatype_dict[k][1].upper().replace(' ', '_') + ":"
        info_c.append(s)
        s = datatype_dict[k][1]
        info_c.append("    return \"" + datatype_dict[k][1] + "\";")
        
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
        
    info_c.append("struct typeinfo types[" + str(len(datatype_dict)) + "] = {")
    for k, v in datatype_dict.items():
        if v[0] == "base":
            s = "    {" + v[0] + ", " + type_bit_dict[k] + ", " + str(v[2]) + "}"
        elif v[0] == "pointer":
            s = "    {" + v[0] + ", " + type_bit_dict[k] + ", " + str(v[2]) + ", .saki=" + str(offset_num_dict[v[3]]) + ", .pcount=" + str(v[4]) + "}"
        elif v[0] == "array":
            s = "    {" + v[0] + ", " + type_bit_dict[k] + ", " + str(v[2]) + ", .saki=" + str(offset_num_dict[v[3]]) + ", .arraysize=" + str(v[4]) + "}"
        elif v[0] == "structure":
            for l in struct_list_all:
                if l[0] == v[1].split(" ")[1]:
                    s = "    {" + v[0] + ", " + type_bit_dict[k] + ", " + str(v[2]) + ", .memnum=" + str(len(l)-1) + ", .mem=" + v[1].split(" ")[1] + "}"
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

    for k in p_type_bit_dict.keys():
        s = "#define _" + datatype_dict[k][1].upper().replace(' ', '_') + " " * (len("LONG_LONG_UNSIGNED_INT") - len(datatype_dict[k][1]) + 1)+ p_type_bit_dict[k]
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

print("make datatype_dict")

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
                datatype= None
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

        elif MODE == CONST_TYPE:
            if re.match(" *type ", line):
                datatype = gettype(line)

            if datatype is not None:
                tag = re.sub("_type", "", tag)
                datatype_dict[offset] = [tag, datatype]

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
                datatype_dict[offset] = [tag, datatype, datatype]

        elif MODE == SUBRANGE_TYPE:
            if re.match(" *upper_bound ", line):
                size = getarraysize(line)

            if size is not None and datatype is not None:
                #datatype_dict[offset].append(0)
                datatype_dict[offset].insert(2, 0)
                #datatype_dict[offset].append(size)
                datatype_dict[offset].insert(4, size)
                
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
sys.exit()


"""
print("make type_bit_dict first loop")

# 型とビットの辞書作成
cnt = 0
for k in datatype_dict.keys():
    if datatype_dict[k][0] == "base":
        #type_bit_dict[k] = p_type_bit_dict[k] = hex(2 ** i)
        type_bit_dict[k] = p_type_bit_dict[k] = hex(cnt)
        cnt = cnt + 1
    elif datatype_dict[k][0] == "structure":
        #type_bit_dict[k] = p_type_bit_dict[k] = hex(2 ** i)
        type_bit_dict[k] = p_type_bit_dict[k] = hex(cnt)
        cnt = cnt + 1

print("make type_bit_dict second loop")

ref = 0
for k in datatype_dict.keys():
    if datatype_dict[k][0] == "pointer":
        ref = datatype_dict[k][1]
        print(k, datatype_dict[k])
        while True:
            if datatype_dict[ref][0] == "base" or "structure":
                type_bit_dict[k] = type_bit_dict[ref]
                break
            elif datatype_dict[ref][0] == "pointer" or "array" or "typedef" or "const":
                ref = datatype_dict[ref][1]
                continue
    elif datatype_dict[k][0] == "array":
        ref = datatype_dict[k][1]
        print(k, datatype_dict[k])
        while True:
            if datatype_dict[ref][0] == "base" or "structure":
                type_bit_dict[k] = type_bit_dict[ref]
                break
            elif datatype_dict[ref][0] == "pointer" or "array" or "typedef" or "const":
                ref = datatype_dict[ref][1]
                continue
    elif datatype_dict[k][0] == "typedef":
        ref = datatype_dict[k][1]
        print(k, datatype_dict[k])
        while True:
            if datatype_dict[ref][0] == "base" or "structure":
                type_bit_dict[k] = type_bit_dict[ref]
                break
            elif datatype_dict[ref][0] == "pointer" or "array" or "typedef" or "const":
                ref = datatype_dict[ref][1]
                continue
    elif datatype_dict[k][0] == "const":
        ref = datatype_dict[k][1]
        print(k, datatype_dict[k])
        while True:
            if datatype_dict[ref][0] == "base" or "structure":
                type_bit_dict[k] = type_bit_dict[ref]
                break
            elif datatype_dict[ref][0] == "pointer" or "array" or "typedef" or "const":
                ref = datatype_dict[ref][1]
                continue

#pprint.pprint(type_bit_dict)
"""
print("updata datatype_dict")


# 辞書の更新
for k in datatype_dict.keys():
    if datatype_dict[k][0] == "typedef":
        datatype_dict[k] = datatype_dict[datatype_dict[k][1]]
    elif datatype_dict[k][0] == "const":
        datatype_dict[k] = datatype_dict[datatype_dict[k][1]]
    elif datatype_dict[k][0] == "pointer":
        if datatype_dict[datatype_dict[k][1]][0] == "pointer":
            print(k, datatype_dict[k])
            datatype_dict[k][4] = datatype_dict[datatype_dict[k][1]][4] + 1
        else:
            datatype_dict[k][4] += 1
        datatype_dict[k][1] = datatype_dict[datatype_dict[k][1]][1]
    elif datatype_dict[k][0] == "array":
        datatype_dict[k][2] = datatype_dict[k][4] * datatype_dict[datatype_dict[k][1]][2]
        datatype_dict[k][1] = datatype_dict[datatype_dict[k][1]][1]
#pprint.pprint(datatype_dict)

sys.exit()

print("make offset_num_dict")

# オフセットと配列の添字の対応を作成
for i, k in enumerate(datatype_dict.keys()):
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


