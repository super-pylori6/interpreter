# -*- coding: utf-8 -*-

import re # 正規表現
import pprint # リストや辞書を見やすく表示
import sys # コマンドライン引数
import linecache # ファイルの指定行を読み込む

TDEFO = 0
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

TYPE_GET = 0

MODEM = TDEFO

args = sys.argv
if len(args) != 2:
    print("args error")
    sys.exit()

READFILE = args[1]
WRITEFILE1 = "debuginfo.c"
WRITEFILE2 = "debuginfo.h"

def storename(line):
    name_m = re.search("\".*\"", line.rstrip('\n'))
    name = re.sub("\"", "", name_m.group())
    return name

def storetype(line):
    typeoffset_m = re.search("\[.*?\]", line.rstrip('\n'))
    typeoffset = re.sub("[ \[\]]", "", typeoffset_m.group())
    return typeoffset

def storesibling(line):
    return storetype(line)

def storesize(line):
    return re.sub(".*\(.*?\) ", "", line.rstrip('\n'))

def storearraysize(line):
    size = int(re.sub(".*\(.*?\) ", "", line.rstrip('\n')))
    return str(size + 1)

def storetag(line):
    return re.sub(" \[ *[0-9a-f]*\] *", "", line.rstrip('\n'))

def storeoffset(line):
    offset_m = re.match(" \[ *[0-9a-f]*\] *", line)
    return re.sub("[ \[\]]", "", offset_m.group())

def storelocation(line):
    return re.sub(".*\(.*?\) ", "", line.rstrip('\n'))

def storeaddr(line):
    addr_m = re.search("0x[0-9a-f]*", line.rstrip('\n'))
    if addr_m:
        return addr_m.group()
    else:
        return "0"
    
def storegvarname(line):
    gvarname_m = re.search("\<.*\>", line.rstrip('\n'))
    gvarname = re.sub("[\<\>]", "", gvarname_m.group())
    return gvarname

# 指定の大域変数のオフセットとアドレスを取得する
def read_gvarinfo(target_name):
    MODE = TDEFO
    for line in open(READFILE, "r", encoding="utf-8_sig"):
        if re.match(" \[ *[0-9a-f]*\] *", line):
            offset = storeoffset(line)
            tag = storetag(line)
            
            if tag == "variable":
                MODE = TVAR
            else:
                MODE = TDEFO
        
        elif MODE == TVAR:
            if re.match(" *type ", line):
                datatype = storetype(line)
            elif re.match(" *\[.*\] addr.*<.*>", line):
                addr = storeaddr(line)
                name = storegvarname(line)
                if name == target_name:
                    break
    else:
        print("指定の変数が見つかりません")
        sys.exit()

    varD = {}
    varD[name] = [tag, datatype, addr]
    return varD, offset

# target_offset に最も近いcompile_unit のオフセット２つを取得する
def read_range(target_offset):
    lower = upper = "0"
    for line in open(READFILE, "r", encoding="utf-8_sig"):
        if re.match(" \[ *[0-9a-f]*\] *", line):
            offset = storeoffset(line)
            tag = storetag(line)

            if tag == "compile_unit":
                if int(offset, 16) < int(target_offset, 16):
                    lower = offset
                    continue
                elif int(offset, 16) > int(target_offset, 16):
                    upper = offset
                    return lower, upper
        else:
            continue

def getTBASElist(line, offset, typeD):
    if re.match(" *byte_size ", line):
        typeD[offset][2] = storesize(line)
    elif re.match(" *name ", line):
        typeD[offset][1] = storename(line)
    return typeD[offset]

def getTTYPEDEFlist(line, offset, typeD):
    if re.match(" *type ", line):
        typeD[offset][1] = storetype(line)
    return typeD[offset]

def getTPOINTERlist(line, offset, typeD):
    if re.match(" *byte_size ", line):
        typeD[offset][2] = storesize(line)
    elif re.match(" *type ", line):
        typeD[offset][1] = typeD[offset][3] = storetype(line)
    return typeD[offset]

def getTARRAYlist(line, offset, typeD):
    if re.match(" *type ", line):
        typeD[offset][1] = typeD[offset][3] = storetype(line)
    elif re.match(" *sibling ", line):
        typeD[offset][4] = storesibling(line)
    return typeD[offset]

def getTSUBRANGElist(line, offset, typeD):
    if re.match(" *upper_bound ", line):
        typeD[offset][1] = storearraysize(line)
    return typeD[offset]

def getTSTRUCTlist(line, offset, typeD):
    if re.match(" *name ", line):
        typeD[offset][1] = storename(line)
    elif re.match(" *byte_size ", line):
        typeD[offset][2] = storesize(line)
    elif re.match(" *sibling ", line):
        typeD[offset][3] = storesibling(line)
    return typeD[offset]

def getTUNIONlist(line, offset, typeD):
    return getTSTRUCTlist(line, offset, typeD)

def getTMEMlist(line, offset, typeD):
    if re.match(" *name ", line):
        typeD[offset][1] = storename(line)
    elif re.match(" *type ", line):
        typeD[offset][2] = storetype(line)
    elif re.match(" *data_member_location ", line):
        typeD[offset][3] = storelocation(line)
    return typeD[offset]

def getTENUMlist(line, offset, typeD):
    return getTSTRUCTlist(line, offset, typeD)

def maketypeD(lower, upper):
    MODE = TDEFO
    typeD = {"0" : ["base_type", "void", "8"]}
    for line in open(READFILE, "r", encoding="utf-8_sig"):
        if re.match(" \[ *[0-9a-f]*\] *", line):
            offset = storeoffset(line)
            tag = storetag(line)

            if int(offset, 16) <= int(lower, 16):
                continue
            elif int(offset, 16) >= int(upper, 16):
                break
            
            if   tag == "base_type":
                MODE = TBASE
                typeD[offset] = [tag, "void", "8"]
            elif tag == "const_type":
                MODE = TCONST
                typeD[offset] = [tag, "0", "8"]
            elif tag == "volatile_type":
                MODE = TVOLATILE
                typeD[offset] = [tag, "0", "8"]
            elif tag == "typedef":
                MODE = TTYPEDEF
                typeD[offset] = [tag, "0", "8"]
            elif tag == "pointer_type":
                MODE = TPOINTER
                typeD[offset] = [tag, "0", "8", "0", 1] # [tag, type, size, ref, pcnt]
            elif tag == "array_type":
                MODE = TARRAY
                typeD[offset] = [tag, "0", "0", "0", "0"] # [tag, type, size, sibling, ref, sibling]
            elif tag == "subrange_type":
                MODE = TSUBRANGE
                typeD[offset] = [tag, 0]
            elif tag == "structure_type":
                MODE = TSTRUCT
                typeD[offset] = [tag, "void", "0", "0"] # [tag, name, size, sibling]
            elif tag == "union_type":
                MODE = TUNION
                typeD[offset] = [tag, "void", "0", "0"]
            elif tag == "member":
                MODE = TMEM
                typeD[offset] = [tag, "void", "0", 0] # [tag, name, type, location]
            elif tag == "enumeration_type":
                MODE = TENUM
                typeD[offset] = [tag, "void", "4", "0"]
            elif tag == "subroutine_type":
                MODE = TSUBROUTINE
                typeD[offset] = [tag, "function", "8"]
            else:
                MODE = TDEFO

        elif MODE == TBASE:
            typeD[offset] = getTBASElist(line, offset, typeD)
        elif MODE in {TCONST, TVOLATILE, TTYPEDEF}:
            typeD[offset] = getTTYPEDEFlist(line, offset, typeD)
        elif MODE == TPOINTER:
            typeD[offset] = getTPOINTERlist(line, offset, typeD)
        elif MODE == TARRAY:
            typeD[offset] = getTARRAYlist(line, offset, typeD)
        elif MODE == TSUBRANGE:
            typeD[offset] = getTSUBRANGElist(line, offset, typeD)
        elif MODE == TSTRUCT:
            typeD[offset] = getTSTRUCTlist(line, offset, typeD)
        elif MODE == TUNION:
            typeD[offset] = getTUNIONlist(line, offset, typeD)
        elif MODE == TMEM:
            typeD[offset] = getTMEMlist(line, offset, typeD)
        elif MODE == TENUM:
            typeD[offset] = getTENUMlist(line, offset, typeD)
        else:
            continue
    return typeD

def getmemrange(typeD, target_datatype):
    return target_datatype, typeD[target_datatype][3]

def make_target_memL(typeD, memlower, memupper, target_memname_set):
    memL = []
    for offset in typeD.keys():
        if int(offset, 16) <= int(memlower, 16):
            continue
        elif int(offset, 16) >= int(memupper, 16):
            continue

        if typeD[offset][1] not in target_memname_set:
            continue
            
        memL.append(typeD[offset])
    return memL


def read_typeD_range(typeD, lower, upper):
    l = []
    for offset in typeD.keys():
        if int(offset, 16) <= int(lower, 16):
            continue
        elif int(offset, 16) >= int(upper, 16):
            continue
            
        l.append(typeD[offset])
    return l

target_name = "init_task"

# 指定の大域変数情報 (オフセット、アドレス、型情報のオフセット)を取得する
varD, target_offset = read_gvarinfo(target_name)
pprint.pprint(varD)



# 指定の大域変数が含まれるcompile_unitの範囲を取得する
lower, upper = read_range(target_offset)
#print(lower, upper)

# デバッグファイル内の指定の範囲を読み取った辞書 typeDを作成する
# 同時に参照先が存在するかのフラグ辞書 refDも作成する
typeD = maketypeD(lower, upper)
#pprint.pprint(typeD)
#pprint.pprint(refD)

def get_target_memL(typeD, struct_type):
    memlower, memupper = getmemrange(typeD, struct_type)
    #print(memlower, memupper)

    target_memname_set = {"tasks", "pid", "comm"}
    return make_target_memL(typeD, memlower, memupper, target_memname_set)

def getmemL(typeD, struct_type):
    memlower, memupper = getmemrange(typeD, struct_type)
    #print(memlower, memupper)

    return read_typeD_range(typeD, memlower, memupper)

def get_struct_name(l):
    return l[1]

def add_target_meminfoD(typeD, struct_type, meminfoD):
    meminfoD[get_struct_name(typeD[struct_type])] = get_target_memL(typeD, struct_type)
    return meminfoD

def add_meminfoD(typeD, struct_type, meminfoD):
    meminfoD[get_struct_name(typeD[struct_type])] = getmemL(typeD, struct_type)
    return meminfoD

def getvardatatype(l):
    return l[1]

target_meminfoD = {}
target_meminfoD = add_target_meminfoD(typeD, getvardatatype(varD[target_name]), target_meminfoD)
#pprint.pprint(target_meminfoD)

def getmemtype(mem):
    return mem[2]

def getval(dictionary, offset):
    return dictionary[offset]

def add_memtype_to_ptypeD(memL, ptypeD):
    for mem in memL:
        offset = getmemtype(mem)
        ptypeD[offset] = getval(typeD, offset)
    return ptypeD

def add_meminfoD_to_ptypeD(meminfoD, ptypeD):
    for memL in meminfoD.values():
        ptypeD = add_memtype_to_ptypeD(memL, ptypeD)
    return ptypeD

target_ptypeD = {varD[target_name][1] : typeD[varD[target_name][1]]}
ptypeD = {"0" : ["base_type", "void", "8"], "1" : ["base_type", "addr_t", "8"]}
ptypeD = add_meminfoD_to_ptypeD(target_meminfoD, ptypeD)
#pprint.pprint(ptypeD)

def gettag(l):
    return l[0]

def gettype(l):
    return l[1]

def add_type_to_ptypeD(typeD, meminfoD, ptypeD):
    tmp_ptypeD   = {}
    tmp_meminfoD = {}
    ptypeD_num = len(ptypeD)
    meminfoD_num = len(meminfoD)
    for offset, l in ptypeD.items():
        tag = gettag(l)
        if tag in {"base_type"}:
            continue
        elif tag in {"typedef", "const", "volatile"}:
            if ptypeD.get(gettype(l)) is None:
                tmp_ptypeD[gettype(l)] = typeD[gettype(l)]
        elif tag in {"pointer_type", "array_type"}:
            if ptypeD.get(gettype(l)) is None:
                tmp_ptypeD[gettype(l)] = typeD[gettype(l)]
        elif tag in {"structure_type"}:
            tmp_meminfoD = add_meminfoD(typeD, offset, tmp_meminfoD)
            tmp_ptypeD   = add_meminfoD_to_ptypeD(tmp_meminfoD, tmp_ptypeD)
        else:
            continue

    ptypeD.update(tmp_ptypeD)
    meminfoD.update(tmp_meminfoD)
    add_num = len(ptypeD) - ptypeD_num + len(meminfoD) - meminfoD_num
    return ptypeD, meminfoD, add_num

def add_ptypeD(typeD, meminfoD, ptypeD):
    while True:
        ptypeD, meminfoD, add_num = add_type_to_ptypeD(typeD, meminfoD, ptypeD)
        if add_num == 0:
            break
    return ptypeD, meminfoD

meminfoD = {}
ptypeD, meminfoD = add_ptypeD(typeD, meminfoD, ptypeD)
ptypeD.update(target_ptypeD)
meminfoD.update(target_meminfoD)
#pprint.pprint(ptypeD)
pprint.pprint(meminfoD)

def get_array_sibling(l):
    return l[4]

def get_arrayrange(typeD, offset):
    return offset, get_array_sibling(typeD[offset])

def set_arraysize(typeD, offset):
    lower, upper = get_arrayrange(typeD, offset)
    subrangel = read_typeD_range(typeD, lower, upper)
    typeD[offset][4] = subrangel[0][1]
    return typeD[offset]

def getref(l):
    return l[3]

def getsize(l):
    return l[2]

def getarraysize(l):
    return l[4]

def set_arraybytesize(ptypeD, offset):
    ref = getref(ptypeD[offset])
    type_bytesize = int(getsize(ptypeD[ref]))
    arraysize = getarraysize(ptypeD[offset])
    ptypeD[offset][2] = type_bytesize * arraysize
    return ptypeD[offset]
    
def update_arraysize(typeD, ptypeD):
    for offset in ptypeD.keys():
        tag = gettag(ptypeD[offset])
        if tag == "array_type":
            ptypeD[offset] = set_arraysize(typeD, offset)
            ptypeD[offset] = set_arraybytesize(ptypeD, offset)
    return ptypeD

ptypeD = update_arraysize(typeD, ptypeD)
#pprint.pprint(ptypeD)

# pointer と array の型参照先を base か structure にする
# typedef, const, volatile はすべて削除する

def getname(l):
    return l[1]

def get_ref_end(ptypeD, offset):
    tag = gettag(ptypeD[offset])
    if tag in {"typedef", "const", "volatile"}:
        return get_ref_end(ptypeD, gettype(ptypeD[offset]))
    else:
        return offset

def get_name_end(ptypeD, offset):
    tag = gettag(ptypeD[offset])
    if tag in {"typedef", "const", "volatile", "pointer_type", "array_type"}:
        return get_name_end(ptypeD, gettype(ptypeD[offset]))
    else:
        return getname(ptypeD[offset])

# メンバの型参照先を最終参照先に更新する
# typedefとかは飛ばすように更新する
def update_memtype(ptypeD, meminfoD):
    for meml in meminfoD.values():
        for mem in meml:
            memtype = getmemtype(mem)
            ref_end = get_ref_end(ptypeD, memtype)
            mem[2] = ref_end

    return meminfoD

meminfoD = update_memtype(ptypeD, meminfoD)
#pprint.pprint(ptypeD)
pprint.pprint(meminfoD)

def update_pointer_and_array(ptypeD):
    for offset, l in ptypeD.items():
        tag = gettag(l)
        if tag in {"pointer_type", "array_type"}:
            l[3] = get_ref_end(ptypeD, gettype(l))
            l[1] = get_name_end(ptypeD, gettype(l))
            ptypeD[offset] = l

    return ptypeD


ptypeD = update_pointer_and_array(ptypeD)
#pprint.pprint(ptypeD)

def remove_tcv(ptypeD):
    tmp = {}
    for offset, l in ptypeD.items():
        tag = gettag(ptypeD[offset])
        if tag not in {"typedef", "const", "volatile"}:
            tmp[offset] = l
            
    return tmp

ptypeD = remove_tcv(ptypeD)
pprint.pprint(ptypeD)

def make_idxD(ptypeD):
    idxD = {}
    for i, offset in enumerate(sorted(ptypeD)):
        idxD[offset] = i
    return idxD

idxD = make_idxD(ptypeD)
pprint.pprint(idxD)

#def make_varD():


#varD = make_varD()
#pprint.pprint(varD)

    
def write_include(c):
    c.append("#include \"" + WRITEFILE2 +"\"") 
    c.append("")
    return c

def write_get_typename(ptypeD, c):
    c.append("char* get_typename(int tbit){")
    c.append("  switch(tbit){")

    for offset, l in sorted(ptypeD.items()):
        if gettag(l) not in {"base_type", "structure_type"}:
            continue
        s = "  case _" + getname(l).upper().replace(' ', '_') + ":"
        s = s + " " * (len("LONG_LONG_UNSIGNED_INT") - len(getname(l)) + 1)
        s = s + "return \""
        if gettag(l) == "structure_type":
            s = s + "struct " + getname(l) + "\";"
        elif getname(l) == "addr_t":
            s = s + "int\";"
        else:
            s = s + getname(l) + "\";"
        c.append(s)

    c.append("  default: return \"\";")
    c.append("  }")
    c.append("}")
    c.append("")
    return c

def write_void(c):
    c.append("  case _VOID:")    
    c.append("    printf(\"0x%lx\", (long int)data);")    
    c.append("    break;")
    return c

def write_addr_t(c):
    c.append("  case _ADDR_T:")    
    c.append("    printf(\"0x%lx\", (long int)data);")    
    c.append("    break;")
    return c

def write_long_unsigned_int(c):
    c.append("  case _LONG_UNSIGNED_INT:")    
    c.append("    printf(\"%lu\", (long unsigned int)data);")
    c.append("    break;")
    return c

def write_unsigned_char(c):
    c.append("  case _UNSIGNED_CHAR:")    
    c.append("    printf(\"%hhu\", (unsigned char)data);")    
    c.append("    break;")
    return c

def write_short_unsigned_int(c):
    c.append("  case _SHORT_UNSIGNED_INT:")    
    c.append("    printf(\"%hu\", (short unsigned int)data);")    
    c.append("    break;")
    return c

def write_unsigned_int(c):
    c.append("  case _UNSIGNED_INT:")    
    c.append("    printf(\"%u\", (unsigned int)data);")    
    c.append("    break;")
    return c

def write_signed_char(c):
    c.append("  case _SIGNED_CHAR:")
    c.append("    printf(\"%hhd\", (signed char)data);")
    c.append("    break;")
    return c

def write_short_int(c):
    c.append("  case _SHORT_INT:")    
    c.append("    printf(\"%d\", (short int)data);")    
    c.append("    break;")
    return c

def write_int(c):
    c.append("  case _INT:")    
    c.append("    printf(\"%d\", (int)data);")    
    c.append("    break;")
    return c

def write_long_int(c):
    c.append("  case _LONG_INT:")    
    c.append("    printf(\"%ld\", (long int)data);")
    c.append("    break;")
    return c

def write_float(c):
    c.append("  case _FLOAT:")    
    c.append("    printf(\"%f\", (float)data);")    
    c.append("    break;")
    return c

def write_double(c):
    c.append("  case _DOUBLE:")    
    c.append("    printf(\"%lf\", (double)data);")
    c.append("    break;")
    return c

def write_char(c):
    c.append("  case _CHAR:")
    c.append("    printf(\"%c\", (char)data);")
    c.append("    break;")
    return c

def write_print_base(ptypeD, c):
    c.append("void print_base(long data, int tbit){")
    c.append("  switch(tbit){")

    for l in ptypeD.values():
        if gettag(l) != "base_type":
            continue
        
        if gettype(l) == "void":
            c = write_void(c)
        elif gettype(l) == "addr_t":
            c = write_addr_t(c)
        elif gettype(l) == "long_unsigned_int":
            c = write_long_unsigned_int(c)
        elif gettype(l) == "unsigned_char":
            c = write_unsigned_char(c)
        elif gettype(l) == "short_unsigned_int":
            c = write_short_unsigned_int(c)
        elif gettype(l) == "unsigned_int":
            c = write_unsigned_int(c)
        elif gettype(l) == "signed_char":
            c = write_signed_char(c)
        elif gettype(l) == "short_int":
            c = write_short_int(c)
        elif gettype(l) == "int":
            c = write_int(c)
        elif gettype(l) == "long_int":
            c = write_long_int(c)
        elif gettype(l) == "float":
            c = write_float(c)
        elif gettype(l) == "double":
            c = write_double(c)
        elif gettype(l) == "char":
            c = write_char(c)

    c.append("  default:")
    c.append("    printf(\"[%s] not defined\", __func__);")    
    c.append("    return;")
    c.append("  }")
    c.append("}")
    c.append("")
    return c

def getvaraddr(l):
    return l[2]


def write_gvarinfo(varD, idxD, c):
    c.append("struct gvarinfo gvars[" + str(len(varD)) + "] = {")
    for name, l in varD.items():
        s = "    {" + str(idxD[getvardatatype(l)]) + ", "
        s = s + "\"" + name + "\"" + ", "
        s = s + getvaraddr(l) + "},"
        c.append(s)
    c.append("};")
    c.append("")
    return c

def getmemname(l):
    return l[1]

def getmemdatatype(l):
    return l[2]

def getmemlocation(l):
    return l[3]

    

def write_memberinfo(meminfoD, idxD, c):
    for name, l in meminfoD.items():
        c.append("struct memberinfo " + name + "[" + str(len(l)) + "] = {")
        for m in l:
            s = "    {" + str(idxD[getmemdatatype(m)]) + ", "
            s = s + "\"" + getmemname(m) + "\"" + ", "
            s = s + "0x" + getmemlocation(m) + "},"
            c.append(s)
        c.append("};")
        c.append("")
    return c

def write_base_type(l, c):
    s = "    {" + gettag(l) + ", "
    s = s + "_" + getname(l).upper().replace(' ', '_') + ", "
    s = s + getsize(l) + "},"
    c.append(s)
    return c

def getmemnum(meminfoD, struct_name):
    return str(len(meminfoD[struct_name]))

def write_structure_type(l, meminfoD, c):
    s = "    {" + gettag(l) + ", "
    s = s + "_" + getname(l).upper().replace(' ', '_') + ", "
    s = s + getsize(l) + ", "
    s = s + ".memnum=" + getmemnum(meminfoD, getname(l)) + ", "
    s = s + ".mem=" + getname(l) + "},"
    c.append(s)
    return c

def getpcount(l):
    return l[4]

def write_pointer_type(l, idxD, c):
    s = "    {" + gettag(l) + ", "
    s = s + "_" + getname(l).upper().replace(' ', '_') + ", "
    s = s + getsize(l) + ", "
    s = s + ".saki=" + str(idxD[getref(l)]) + ", "
    s = s + ".pcount=" + str(getpcount(l)) + "},"
    c.append(s)
    return c

def write_array_type(l, idxD, c):
    s = "    {" + gettag(l) + ", "
    s = s + "_" + getname(l).upper().replace(' ', '_') + ", "
    s = s + getsize(l) + ", "
    s = s + ".saki=" + str(idxD[getref(l)]) + ", "
    s = s + ".arraysize=" + str(getarraysize(l)) + "},"
    c.append(s)
    return c

def write_typeinfo(ptypeD, meminfoD, c):
    c.append("struct typeinfo types[" + str(len(ptypeD)) + "] = {")
    for offset, l in sorted(ptypeD.items()):
        tag = gettag(l)
        if tag == "base_type":
            c = write_base_type(l, c)
        elif tag == "structure_type":
            c = write_structure_type(l, meminfoD, c)
        elif tag == "pointer_type":
            c = write_pointer_type(l, idxD, c)
        elif tag == "array_type":
            c = write_array_type(l, idxD, c)
        else:
            print("error : not defined tag")
            sys.exit()
    c.append("};")
    c.append("")
    return c


def write_c(ptypeD, varD, meminfoD, idxD):
    c = []
    c = write_include(c)
    c = write_get_typename(ptypeD, c)
    c = write_print_base(ptypeD, c)
    c = write_gvarinfo(varD, idxD, c)
    c = write_memberinfo(meminfoD, idxD, c)
    c = write_typeinfo(ptypeD, meminfoD, c)

    with open(WRITEFILE1, mode="w") as f:
        f.writelines("\n".join(c))

def write_include_gurde(h):
    s0 = "#ifndef _" + WRITEFILE2.replace(".", "_").upper()
    s1 = "#define _" + WRITEFILE2.replace(".", "_").upper()
    sl = "#endif"
    h.insert(0, s0)
    h.insert(1, s1)
    h.insert(2, "")
    h.append(sl)
    return h

def write_define_tbit(ptypeD, h):
    i = 0
    for offset, l in sorted(ptypeD.items()):
        if gettag(l) not in {"base_type", "structure_type"}:
            continue

        s = "#define _" + getname(l).upper().replace(' ', '_')
        s = s + " " * (len("LONG_LONG_UNSIGNED_INT") - len(getname(l)) + 1)
        s = s + str(hex(i))
        h.append(s)
        i = i + 1
        
    h.append("")
    return h

def write_define_enum(h):
    h.append("enum type {")
    h.append("    base_type,")
    h.append("    pointer_type,")
    h.append("    array_type,")
    h.append("    structure_type,")
    h.append("    union_type,")
    h.append("    enumeration,")
    h.append("};")
    h.append("")
    return h

def write_define_struct(h):
    h.append("struct gvarinfo {")
    h.append("    int tidx;")
    h.append("    char* name;")
    h.append("    long addr;")
    h.append("};")
    h.append("")
    h.append("struct memberinfo {")
    h.append("    int tidx;")
    h.append("    char* name;")
    h.append("    long offset;")
    h.append("};")
    h.append("")
    h.append("struct typeinfo {")
    h.append("    enum type kind;")
    h.append("    int tbit;")
    h.append("    int bytesize;")
    h.append("    union {")
    h.append("        int saki; //pointer")
    h.append("        int memnum; //structure, union")
    h.append("    };")
    h.append("    union {")
    h.append("        int pcount; //pointer")
    h.append("        int arraysize; //array")
    h.append("        struct memberinfo* mem; //structure, union")
    h.append("    };")
    h.append("};")
    h.append("")
    return h

def write_prototype(h):
    h.append("void print_base(long data, int tbit);")
    h.append("char* get_typename(int tbit);")
    h.append("")
    return h

def write_extern(varD, meminfoD, ptypeD, h):
    h.append("extern struct gvarinfo gvars[" + str(len(varD)) + "];")
    for name, l in meminfoD.items():
        h.append("extern struct memberinfo " + name + "[" + str(len(l)) + "];")
    h.append("extern struct typeinfo types[" + str(len(ptypeD)) + "];")
    h.append("")
    return h

def write_h(ptypeD, varD, meminfoD):
    h = []
    h = write_define_tbit(ptypeD, h)
    h = write_define_enum(h)
    h = write_define_struct(h)
    h = write_prototype(h)
    h = write_extern(varD, meminfoD, ptypeD, h)
    h = write_include_gurde(h)
    
    with open(WRITEFILE2, mode="w") as f:
        f.writelines("\n".join(h))

    

write_c(ptypeD, varD, meminfoD, idxD)
write_h(ptypeD, varD, meminfoD)
print(READFILE)
print("├─>" + WRITEFILE1)
print("└─>" + WRITEFILE2)


sys.exit()


#write_infoc()
#with open("info.c") as f: print(f.read())

#write_infoh()
#with open("info.h") as f: print(f.read())




