# VUT projekt
# IPP 2022/2023, 2BIT
# Autor:
# Kateřina Čepelková, xcepel03

import re
import sys
import argparse
import xml.etree.ElementTree as ET


# TRIDY #
# Tridy pro ukladani argumentu instrukci
class Argument:
    def __init__(self, arg_type, value):
        self.type = arg_type
        self.value = value


# Tridy pro ukladani instrukci a jejich atributu
class Instruction:
    def __init__(self, order, op_code):
        self.order = int(order)
        self.op_code = op_code
        self.args = {}  # slovnik vsech argumentu instrukce (0 - 3)

    def add_argument(self, arg_type, value, key):
        if key not in ('arg1', 'arg2', 'arg3'):
            error("XML - Chybny zapis subelementu (arg) instrukce", 32)
        self.args[key] = Argument(arg_type, value)

    def arg_cnt(self):
        return len(self.args)

    # Vrati typ urciteho (pos) argumentu
    def get_arg_type(self, pos):
        if self.args.get(pos) is not None:
            return self.args.get(pos).type
        return None

    # Vrati hodnotu urciteho (pos) argumentu
    def get_arg_value(self, pos):
        if self.args.get(pos) is not None:
            return self.args.get(pos).value
        return None


# Trida interpretu s metodami pro intepretaci
class Interpret:
    def __init__(self, instructions, file, labels):
        self.instructions = instructions.copy()  # seznam instrukci -> (instrukce, poradi)
        self.run_inst = instructions.copy()  # seznam instrukci pro beh -> (instrukce, poradi)
        self.labels = labels  # seznam navesti -> (label, poradi label v kodu)
        self.file = file.copy()  # vstupy souboru rozdeleny po radcich

        # Ramce pro ukladani promenych -> dvojice = (hodnota, typ)
        self.GF = dict()  # globalni ramec - na zacatku prazdny
        self.LF = list()  # zasobnik pro lokalni ramce - na zacatku prazdny
        self.TF = None  # docasny ramec
        self.stack = list()  # datovy zasobnik
        self.last_pos = list()  # zasobnik volani

        self.ex_inst = 0  # pocet vykonanych instrukci

    # Vrati "ukazatel" na zadany ramec (XX@), nebo error, pokud neexistuje
    def get_frame(self, frame):
        if frame == 'GF@':
            return self.GF
        elif frame == 'LF@':
            if self.LF is None:
                error("Lokalni rámec neni definovany", 55)
            try:
                return self.LF[-1]
            except IndexError:
                error("Lokalni rámec neni definovany", 55)
        elif frame == 'TF@':
            if self.TF is None:
                error("Dočasný rámec není definovaný", 55)
            return self.TF

        error("Rámec {} není definován".format(frame), 55)

    # Vrati hodnotu promene / None = je nedefinovana / Error = neexistuje
    def get_touple(self, var):
        frame = self.get_frame(var[0:3])
        name = var[3:]
        try:
            return frame[name]
        except KeyError:
            error("Pristup k neexistujici promene", 54)

    # Vrati dvojici (hodnota, typ) ze stacku (pokud neni prazdny)
    def get_stack_touple(self):
        if len(self.stack) == 0:
            error("Datovy zasobnik je prazdny", 56)
        return self.stack.pop()

    # Vrati hodnotu argumentu (vyjme ji z promene pokud potreba)
    def get_value_from_arg(self, inst, arg_num):
        if inst.get_arg_type(arg_num) == 'var':
            touple = self.get_touple(inst.get_arg_value(arg_num))
            if touple is None:
                error("Pokus o cteni hodnoty neinicializovane promenne", 56)
            return touple[0]
        else:
            return inst.get_arg_value(arg_num)

    # Vrati typ argumentu (vyjme ji z promene pokud potreba)
    def get_type_from_arg(self, inst, arg_num):
        if inst.get_arg_type(arg_num) == 'var':
            touple = self.get_touple(inst.get_arg_value(arg_num))
            if touple is None:
                error("Pokus o cteni argumentu neinicializovane promenne", 56)
            return touple[1]
        else:
            return inst.get_arg_type(arg_num)

    # METODY PRO INSTRUKCE:
    # Prace s ramci, volani funkci #
    # Prirazeni hodnoty do promene
    def move(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        self.get_touple(inst.get_arg_value('arg1'))  # kontrola, zda je definovana

        if self.get_type_from_arg(inst, 'arg2') == 'label':  # Label neni promena
            error("Spatny typ operandu", 53)
        # Ulozeni arg2 do arg1

        name_arg1 = inst.get_arg_value('arg1')
        self.get_frame(name_arg1[:3])[name_arg1[3:]] \
            = (self.get_value_from_arg(inst, 'arg2'), self.get_type_from_arg(inst, 'arg2'))

    # Vytvori novy doscasny ramec
    def createframe(self, inst):
        arg_cnt_check(inst, 0)

        self.TF = None  # odstraneni stareho ramce
        self.TF = dict()

    # Presun docasneho ramce na zasobnik ramcu
    def pushframe(self, inst):
        arg_cnt_check(inst, 0)

        self.LF.append(self.get_frame('TF@'))
        self.TF = None

    # Presun aktualniho ramce do docasneho
    def popframe(self, inst):
        arg_cnt_check(inst, 0)

        self.get_frame('LF@')

        self.TF = self.LF.pop()

    # Definuje novou promnenou v ramci
    def defvar(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        frame = self.get_frame(inst.get_arg_value('arg1')[:3])
        name = inst.get_arg_value('arg1')[3:]

        if name in frame:
            error("Redefinice promenne", 52)
        frame[name] = None

    # Skok na navesti s podporou navratu
    def call(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        self.last_pos.append(inst.order + 1)
        arg1_val = self.get_value_from_arg(inst, 'arg1')
        self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Navrat na ulozenou pozici (last_pos)
    def _return(self, inst):
        arg_cnt_check(inst, 0)

        if len(self.last_pos) == 0:
            error("Neexistujici navratova hodnota", 56)
        back = self.last_pos.pop()

        new_order = []
        for pair in self.instructions:
            if pair[1] >= back:  # ukladame prvky na poradi stejnem (label) a vyssim nez cil skoku
                new_order.append(pair)
        self.run_inst = new_order

    # Prace s datovym zasobnikem #
    # Vlozi hodnotu na vrchol dat. zasobniku
    def pushs(self, inst):
        arg_cnt_check(inst, 1)

        self.stack.append((self.get_value_from_arg(inst, 'arg1'), self.get_type_from_arg(inst, 'arg1')))

    # Vyjme hodnotu z vrcholu dat. zasobniku
    def pops(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        if len(self.stack) == 0:
            error("Datovy zasobnik je prazdny", 56)
        name_arg1 = inst.get_arg_value('arg1')
        self.get_frame(name_arg1[:3])[name_arg1[3:]] = self.stack.pop()

    # Vyprazdni datovy zasobnik
    def clears(self, inst):
        arg_cnt_check(inst, 0)
        self.stack = []

    # Aritmeticke, relacni, booleovske a konverzni instrukce #
    # Soucet 2 ciselnych hodnot
    def add(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        name_arg1 = inst.get_arg_value('arg1')
        if arg2_type == 'int' and arg3_type == 'int':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val) + int(arg3_val)), 'int')
        elif arg2_type == 'float' and arg3_type == 'float':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = \
                (str(float.hex(float_transfer(arg2_val) + float_transfer(arg3_val))), 'float')
        else:
            error("Spatny typ operandu - ADD vyzaduje 2 inty/floaty.", 53)

    # Soucet 2 ciselnych hodnot ze zasobniku
    def adds(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not symb1[1] == 'int' or not symb2[1] == 'int':
            error("Spatny typ operandu - ADDS vyzaduje 2 inty.", 53)

        self.stack.append((str(int(symb1[0]) + int(symb2[0])), 'int'))

    # Odecitani 2 ciselnych hodnot
    def sub(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        name_arg1 = inst.get_arg_value('arg1')
        if arg2_type == 'int' and arg3_type == 'int':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val) - int(arg3_val)), 'int')
        elif arg2_type == 'float' and arg3_type == 'float':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = \
                (str(float.hex(float_transfer(arg2_val) - float_transfer(arg3_val))), 'float')
        else:
            error("Spatny typ operandu - SUB vyzaduje 2 inty/floaty.", 53)

    # Odecitani 2 ciselnych hodnot ze zasobniku
    def subs(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not symb1[1] == 'int' or not symb2[1] == 'int':
            error("Spatny typ operandu - SUBS vyzaduje 2 inty.", 53)

        self.stack.append((str(int(symb1[0]) - int(symb2[0])), 'int'))

    # Nasobeni 2 ciselnych hodnot
    def mul(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        name_arg1 = inst.get_arg_value('arg1')
        if arg2_type == 'int' and arg3_type == 'int':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val) * int(arg3_val)), 'int')
        elif arg2_type == 'float' and arg3_type == 'float':
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = \
                (str(float.hex(float_transfer(arg2_val) * float_transfer(arg3_val))), 'float')
        else:
            error("Spatny typ operandu - MUL vyzaduje 2 inty/floaty.", 53)

    # Nasobeni 2 ciselnych hodnot ze zasobniku
    def muls(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not symb1[1] == 'int' or not symb2[1] == 'int':
            error("Spatny typ operandu - MULS vyzaduje 2 inty.", 53)

        self.stack.append((str(int(symb1[0]) * int(symb2[0])), 'int'))

    # Deleni 2 ciselnych hodnot
    def div(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        name_arg1 = inst.get_arg_value('arg1')
        if arg2_type == 'int' and arg3_type == 'int':
            if arg3_val == '0':
                error("Deleni nulou", 57)
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val) // int(arg3_val)), 'int')
        elif arg2_type == 'float' and arg3_type == 'float':
            if float_transfer(arg3_val) == 0:
                error("Deleni nulou", 57)
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = \
                (str(float.hex(float_transfer(arg2_val) / float_transfer(arg3_val))), 'float')
        else:
            error("Spatny typ operandu - DIV vyzaduje 2 inty/floaty.", 53)

    # Deleni 2 celo-ciselnych hodnot
    def idiv(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        name_arg1 = inst.get_arg_value('arg1')
        if arg2_type == 'int' and arg3_type == 'int':
            if arg3_val == '0':
                error("Deleni nulou", 57)
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val) // int(arg3_val)), 'int')
        else:
            error("Spatny typ operandu - IDIV vyzaduje 2 inty/floaty.", 53)

    # Deleni 2 ciselnych hodnot ze zasobniku
    def idivs(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not symb1[1] == 'int' or not symb2[1] == 'int':
            error("Spatny typ operandu - IDIVS vyzaduje 2 inty.", 53)
        if symb2[0] == '0':
            error("Deleni nulou", 57)

        self.stack.append((str(int(symb1[0]) // int(symb2[0])), 'int'))

    # Relacni operator mensi
    def lt(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        if not arg2_type == arg3_type or arg2_type not in ('int', 'bool', 'string', 'float'):
            error("Spatny typ operandu - LT vyzaduje 2 hodnoty stejneho typu (int/bool/string).", 53)
        if arg2_type == 'int':
            arg2_val = int(arg2_val)
            arg3_val = int(arg3_val)
        elif arg2_type == 'float':
            arg2_val = float_transfer(arg2_val)
            arg3_val = float_transfer(arg3_val)
        elif arg2_type == 'bool':
            arg2_val = str_to_bool(arg2_val)
            arg3_val = str_to_bool(arg3_val)

        name_arg1 = inst.get_arg_value('arg1')
        if arg2_val < arg3_val:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Relacni operator mensi - hodnoty ze zasobniku
    def lts(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb2_val = symb2[0]
        symb1 = self.get_stack_touple()
        symb1_val = symb1[0]

        if not symb1[1] == symb2[1] or symb1[1] not in ('int', 'bool', 'string'):
            error("Spatny typ operandu - LTS vyzaduje 2 hodnoty stejneho typu (int/bool/string).", 53)
        if symb1[1] == 'int':
            symb1_val = int(symb1_val)
            symb2_val = int(symb2_val)
        elif symb1[1] == 'bool':
            symb1_val = str_to_bool(symb1_val)
            symb2_val = str_to_bool(symb2_val)

        if symb1_val < symb2_val:
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Relacni operator vetsi
    def gt(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        if not arg2_type == arg3_type or arg2_type not in ('int', 'bool', 'string', 'float'):
            error("Spatny typ operandu - GT vyzaduje 2 hodnoty stejneho typu (int/bool/string).", 53)
        if arg2_type == 'int':
            arg2_val = int(arg2_val)
            arg3_val = int(arg3_val)
        elif arg2_type == 'float':
            arg2_val = float_transfer(arg2_val)
            arg3_val = float_transfer(arg3_val)
        elif arg2_type == 'bool':
            arg2_val = str_to_bool(arg2_val)
            arg3_val = str_to_bool(arg3_val)

        name_arg1 = inst.get_arg_value('arg1')
        if arg2_val > arg3_val:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Relacni operator vetsi - hodnoty ze zasobniku
    def gts(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb2_val = symb2[0]
        symb1 = self.get_stack_touple()
        symb1_val = symb1[0]

        if not symb1[1] == symb2[1] or symb1[1] not in ('int', 'bool', 'string'):
            error("Spatny typ operandu - GTS vyzaduje 2 hodnoty stejneho typu (int/bool/string).", 53)
        if symb1[1] == 'int':
            symb1_val = int(symb1_val)
            symb2_val = int(symb2_val)
        elif symb1[1] == 'bool':
            symb1_val = str_to_bool(symb1_val)
            symb2_val = str_to_bool(symb2_val)

        if symb1_val > symb2_val:
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Relacni operator rovno
    def eq(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')

        name_arg1 = inst.get_arg_value('arg1')
        if equality(arg2_val, arg2_type, arg3_val, arg3_type):
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Relacni operator rovno pro datovy zasobnik
    def eqs(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if equality(symb1[0], symb1[1], symb2[0], symb2[1]):
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Booleovsky operator and
    def _and(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if not arg2_type == 'bool' or not arg3_type == 'bool':
            error("Spatny typ operandu - AND vyzaduje 2 booly.", 53)
        arg2_val = str_to_bool(self.get_value_from_arg(inst, 'arg2'))
        arg3_val = str_to_bool(self.get_value_from_arg(inst, 'arg3'))

        name_arg1 = inst.get_arg_value('arg1')
        if arg2_val and arg3_val:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Booleovsky operator and nad datovym zasobnikem
    def ands(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb2_val = symb2[0]
        symb1 = self.get_stack_touple()
        symb1_val = symb1[0]

        if not symb1[1] == 'bool' or not symb2[1] == 'bool':
            error("Spatny typ operandu - ANDS vyzaduje 2 booly.", 53)
        symb1_val = str_to_bool(symb1_val)
        symb2_val = str_to_bool(symb2_val)

        if symb1_val and symb2_val:
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Booleovsky operator or
    def _or(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if not arg2_type == 'bool' or not arg3_type == 'bool':
            error("Spatny typ operandu - OR vyzaduje 2 booly.", 53)
        arg2_val = str_to_bool(self.get_value_from_arg(inst, 'arg2'))
        arg3_val = str_to_bool(self.get_value_from_arg(inst, 'arg3'))

        name_arg1 = inst.get_arg_value('arg1')
        if arg2_val or arg3_val:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Booleovsky operator or nad datovym zasobnikem
    def ors(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb2_val = symb2[0]
        symb1 = self.get_stack_touple()
        symb1_val = symb1[0]

        if not symb1[1] == 'bool' or not symb2[1] == 'bool':
            error("Spatny typ operandu - ORS vyzaduje 2 booly.", 53)
        symb1_val = str_to_bool(symb1_val)
        symb2_val = str_to_bool(symb2_val)

        if symb1_val or symb2_val:
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Booleovsky operator not
    def _not(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_type = self.get_type_from_arg(inst, 'arg2')
        if not arg2_type == 'bool':
            error("Spatny typ operandu - NOT vyzaduje 2 booly.", 53)
        arg2_val = str_to_bool(self.get_value_from_arg(inst, 'arg2'))

        name_arg1 = inst.get_arg_value('arg1')
        if not arg2_val:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('true', 'bool')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('false', 'bool')

    # Booleovsky operator not nad datovym zasobnikem
    def nots(self, inst):
        arg_cnt_check(inst, 0)
        symb1 = self.get_stack_touple()
        symb1_val = symb1[0]

        if not symb1[1] == 'bool':
            error("Spatny typ operandu - NOTS vyzaduje bool.", 53)
        symb1_val = str_to_bool(symb1_val)

        if not symb1_val:
            self.stack.append(('true', 'bool'))
        else:
            self.stack.append(('false', 'bool'))

    # Prevod celeho cisla na znak
    def int2char(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        if not arg2_type == 'int':
            error("Spatny typ operandu - INT2CHAR vyzaduje int", 53)

        name_arg1 = inst.get_arg_value('arg1')
        try:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (chr(int(arg2_val)), 'string')
        except ValueError:
            error("INT2CHAR - Nevalidni ordinalni hodnota znaku v Unicode", 58)

    # Prevod intu na float
    def int2float(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        if not arg2_type == 'int':
            error("Spatny typ operandu - INT2FLOAT vyzaduje int", 53)

        name_arg1 = inst.get_arg_value('arg1')
        self.get_frame(name_arg1[:3])[name_arg1[3:]] = (float.hex(float(arg2_val)), 'float')

    # Prevod celeho cisla ze zasobniku na znak
    def int2chars(self, inst):
        arg_cnt_check(inst, 0)

        symb1 = self.get_stack_touple()
        if not symb1[1] == 'int':
            error("Spatny typ operandu - INT2CHARS vyzaduje int.", 53)

        try:
            self.stack.append((chr(int(symb1[0])), 'string'))
        except ValueError:
            error("INT2CHARS - Nevalidni ordinalni hodnota znaku v Unicode", 58)

    # Vrati ordinalni hodnotu znaku na urcite pozici
    def stri2int(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if not arg2_type == 'string' or not arg3_type == 'int':
            error("Spatny typ operandu - STRI2INT vyzaduje 1 string a 1 int", 53)
        if int(arg3_val) < 0:
            error(" STRI2INT nemuze pristoupit na zapornou pozici", 58)

        name_arg1 = inst.get_arg_value('arg1')
        try:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(ord(arg2_val[int(arg3_val)])), 'string')
        except IndexError:
            error("GETCHAR - Indexace mimo dany retezec", 58)

    # Vrati ordinalni hodnotu znaku na urcite pozici stringu ze stacku
    def stri2ints(self, inst):
        arg_cnt_check(inst, 0)

        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not symb1[1] == 'string' or not symb2[1] == 'int':
            error("Spatny typ operandu - STRI2INTS vyzaduje 1 string a 1 int.", 53)
        if int(symb2[0]) < 0:
            error("STRI2INT nemuze pristoupit na zapornou pozici", 58)

        try:
            self.stack.append((str(ord(symb1[0][int(symb2[0])])), 'string'))
        except IndexError:
            error("GETCHARS - Indexace mimo dany retezec", 58)

    # Prevod floatu na int
    def float2int(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        if not arg2_type == 'float':
            error("Spatny typ operandu - FLOAT2INT vyzaduje float", 53)

        name_arg1 = inst.get_arg_value('arg1')

        if arg2_val[:2] == "0x":
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(float.fromhex(arg2_val))), 'int')
        else:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(int(arg2_val)), 'int')

    # Vstupne-vystupni instrukce #
    # Nacteni hodnoty
    def read(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var' or inst.get_arg_type('arg2') != 'type':
            error("Spatny typ operandu", 53)

        # Nacteni hodnoty
        name_arg1 = inst.get_arg_value('arg1')
        try:
            value = self.file.pop(0)
        except IndexError:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('nil', 'nil')
            return

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        if arg2_val not in ('string', 'int', 'bool', 'float'):
            error("Spatny typ operandu - READ - typ musi byt string/int/bool", 32)

        res = (value, 'string')
        # Upravovani hodnoty
        if arg2_val == "int":
            try:
                res = (str(int(value)), 'int')
            except ValueError:
                res = ('nil', 'nil')
        elif arg2_val == "float":
            try:
                res = (str(float_transfer(value)), 'float')
            except ValueError:
                res = ('nil', 'nil')
        elif arg2_val == "bool":
            if value.lower() == 'true':
                res = ('true', 'bool')
            else:
                res = ('false', 'bool')

        self.get_frame(name_arg1[:3])[name_arg1[3:]] = res

    # Vypis hodnoty na stdout
    def write(self, inst):
        arg_cnt_check(inst, 1)

        arg_val = self.get_value_from_arg(inst, 'arg1')
        arg_type = self.get_type_from_arg(inst, 'arg1')
        if arg_type == 'nil':
            arg_val = ''
        elif arg_type == 'string':
            try:
                matches = re.findall(r'\\\d{3}', arg_val)
                for match in matches:
                    arg_val = arg_val.replace(match, chr(int(match[1:])))
            except TypeError:
                arg_val = ''
        elif arg_type == 'float':
            try:
                if arg_val[:2] == "0x":
                    arg_val = str(arg_val)
                else:
                    arg_val = str(float.hex(float(arg_val)))
            except ValueError:
                arg_val = ''

        print(arg_val, end='')

    # Prace s retezci #
    # Konkatenace 2 retezcu
    def concat(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = str(self.get_value_from_arg(inst, 'arg2'))
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = str(self.get_value_from_arg(inst, 'arg3'))
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if not arg2_type == 'string' or not arg3_type == 'string':
            error("Spatny typ operandu - CONCAT vyzaduje 2 stringy.", 53)

        name_arg1 = inst.get_arg_value('arg1')
        self.get_frame(name_arg1[:3])[name_arg1[3:]] = (arg2_val + arg3_val, 'string')

    # Zjisti delku retezce
    def strlen(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = str(self.get_value_from_arg(inst, 'arg2'))
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        if not arg2_type == 'string':
            error("Spatny typ operandu - STRLEN vyzaduje 1 string.", 53)

        name_arg1 = inst.get_arg_value('arg1')
        self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(len(arg2_val)), 'int')

    # Vrati znak z retezce na urcite pozici
    def getchar(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if not arg2_type == 'string' or not arg3_type == 'int':
            error("Spatny typ operandu - GETCHAR vyzaduje 1 string a 1 int.", 53)
        if int(arg3_val) < 0:
            error("GETCHAR nemuze pristoupit na zapornou pozici", 58)

        name_arg1 = inst.get_arg_value('arg1')
        try:
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(arg2_val[int(arg3_val)]), 'string')
        except IndexError:
            error("GETCHAR - Indexace mimo dany retezec", 58)

    # Zmeni znak z retezce na urcite pozici
    def setchar(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        if arg2_type != 'int' or arg3_type != 'string':
            error("Spatny typ operandu - SETCHAR vyzaduje 1 int a 1 string.", 53)
        if int(arg2_val) < 0:
            error("SETCHAR nemuze pristoupit na zapornou pozici", 58)
        if arg3_val == '':
            error("SETCHAR - symb2 je prazdny retezec", 58)

        name_arg1 = inst.get_arg_value('arg1')
        try:  # prevedeni na list -> zmena char -> zpatky na string
            new_str = list(self.get_value_from_arg(inst, 'arg1'))
            new_str[int(arg2_val)] = str(arg3_val[0])
            self.get_frame(name_arg1[:3])[name_arg1[3:]] = (("".join(new_str)), 'string')
        except IndexError:
            error("SETCHAR - Indexace mimo dany retezec", 58)
        except TypeError:
            error("SETCHAR - Retezec je prazdny", 58)

    # Prace s typy #
    # Zjisti typ daneho symbolu
    def type(self, inst):
        arg_cnt_check(inst, 2)
        if inst.get_arg_type('arg1') != 'var':
            error("Spatny typ operandu", 53)

        name_arg1 = inst.get_arg_value('arg1')
        if inst.get_arg_type('arg2') == 'var':
            try:
                frame = self.get_frame(inst.get_arg_value('arg2')[0:3])
                var_name = inst.get_arg_value('arg2')[3:]
                if frame[var_name] is None:  # Neinicializovane promene
                    self.get_frame(name_arg1[:3])[name_arg1[3:]] = ('', 'string')
                    return
            except KeyError:
                error("Pristup k neexistujici promene", 54)
        self.get_frame(name_arg1[:3])[name_arg1[3:]] = (str(self.get_type_from_arg(inst, 'arg2')), 'string')

    # Instrukce pro rizeni toku programu
    # Definice navesti
    def label(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

    # Nepodmineny skok na navesti
    def jump(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        arg1_val = self.get_value_from_arg(inst, 'arg1')
        self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Podmineny skok na navesti pri rovnosti
    def jumpifeq(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        arg1_val = self.get_value_from_arg(inst, 'arg1')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')

        if equality(arg2_val, arg2_type, arg3_val, arg3_type):
            self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Podmineny skok na navesti pri rovnosti hodnot z datoveho zasobniku
    def jumpifeqs(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        arg1_val = self.get_value_from_arg(inst, 'arg1')
        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if equality(symb1[0], symb1[1], symb2[0], symb2[1]):
            self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Podmineny skok na navesti pri nerovnosti
    def jumpifneq(self, inst):
        arg_cnt_check(inst, 3)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        arg1_val = self.get_value_from_arg(inst, 'arg1')
        arg2_type = self.get_type_from_arg(inst, 'arg2')
        arg3_type = self.get_type_from_arg(inst, 'arg3')
        arg2_val = self.get_value_from_arg(inst, 'arg2')
        arg3_val = self.get_value_from_arg(inst, 'arg3')

        if not equality(arg2_val, arg2_type, arg3_val, arg3_type):
            self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Podmineny skok na navesti pri nerovnosti hodnot z datoveho zasobniku
    def jumpifneqs(self, inst):
        arg_cnt_check(inst, 1)
        if inst.get_arg_type('arg1') != 'label':
            error("Spatny typ operandu", 53)

        arg1_val = self.get_value_from_arg(inst, 'arg1')
        symb2 = self.get_stack_touple()
        symb1 = self.get_stack_touple()

        if not equality(symb1[0], symb1[1], symb2[0], symb2[1]):
            self.run_inst = after_jump_inst(self.labels, arg1_val, self.instructions)

    # Ukonceni interpretace s navratovym kodem
    def exit(self, inst):
        arg_cnt_check(inst, 1)

        arg_type = self.get_type_from_arg(inst, 'arg1')
        if arg_type != 'int':
            error("Spatny typ operandu", 53)

        arg_val = int(self.get_value_from_arg(inst, 'arg1'))

        if arg_val > 0 and arg_val > 49:
            error("Nevalidni EXIT hodnota", 57)

        exit(arg_val)

    # Ladici instrukce #
    # Vypis hodnoty na stderr
    def dprint(self, inst):
        arg_cnt_check(inst, 1)

        arg_val = str(self.get_value_from_arg(inst, 'arg1'))
        arg_type = self.get_type_from_arg(inst, 'arg1')
        if arg_type == 'string':
            matches = re.findall(r'\\\d{3}', arg_val)
            for match in matches:
                arg_val = arg_val.replace(match, chr(int(match[1:])))
        sys.stderr.write(arg_val)

    # Vypis stavu interpretu na stderr
    def _break(self, inst):
        arg_cnt_check(inst, 0)
        sys.stderr.write('Pozice/Poradi v kodu: ' + str(inst.order) + '\n')
        sys.stderr.write('Obsah ramcu:' + '\n')
        sys.stderr.write('GF: ' + str(self.GF) + '\n')
        sys.stderr.write('LF: ' + str(self.LF) + '\n')
        sys.stderr.write('TF: ' + str(self.TF) + '\n')
        sys.stderr.write('Zasobnik: ' + str(self.stack) + '\n')
        sys.stderr.write('Pocet vykonanych instrukci: ' + str(self.ex_inst) + '\n')

    # ------------
    # Spoustec interpretu
    def run(self):
        while len(self.run_inst) != 0:
            inst = self.run_inst.pop(0)[0]
            getattr(self, inst.op_code)(inst)
            self.ex_inst += 1


# FUNKCE #
# Error handler
def error(msg, err_code):
    sys.stderr.write(msg + "\n")
    sys.exit(err_code)


# Funkce pro kontrolu spravneho poctu argumentu instrukce
def arg_cnt_check(inst, num):
    if inst.arg_cnt() != num:
        error("Spatny pocet argumentu", 32)
    if num == 1 and inst.get_arg_type('arg1') is None:
        error("Chybejici argumenty", 32)
    if num == 2 and (inst.get_arg_type('arg1') is None or inst.get_arg_type('arg2') is None):
        error("Chybejici argumenty", 32)
    if num == 3 and (inst.get_arg_type('arg1') is None or inst.get_arg_type('arg2') is None
                     or inst.get_arg_type('arg2') is None):
        error("Chybejici argumenty", 32)


# Prevedeni libovolneho floatu na decimalni float
def float_transfer(float_var):
    try:
        if float_var[:2] == "0x":
            return float.fromhex(float_var)
        else:
            return float(float_var)
    except ValueError:
        error("Spatny typ floatu", 32)


# Prevedeni stringu na bool (pro logicke instrukce)
def str_to_bool(val):
    if val == 'true':
        return True
    return False


# Vrati True, pokud se hodnoty rovnaji, jinak False
def equality(arg1_val, arg1_type, arg2_val, arg2_type):
    if arg1_type == 'nil' or arg2_type == 'nil':  # pokud je jeden z nich NIL, druhy muze byt libovolny
        if arg1_val == arg2_val or arg1_val == '' or arg2_val == '':
            return True
        else:
            return False
    elif arg1_type != arg2_type or arg1_type not in ('int', 'bool', 'string', 'nil'):
        error("Spatny typ operandu - ET vyzaduje 2 hodnoty stejneho typu (int/bool/string) "
              "nebo nil a int/bool/string.", 53)

    if arg1_type == 'int':
        arg1_val = int(arg1_val)
        arg2_val = int(arg2_val)
    elif arg2_type == 'float':
        arg1_val = float_transfer(arg1_val)
        arg2_val = float_transfer(arg2_val)
    elif arg1_type == 'bool':
        arg1_val = str_to_bool(arg1_val)
        arg2_val = str_to_bool(arg2_val)

    if arg1_val == arg2_val:
        return True
    else:
        return False


# Vrati nove poradi instrukci po skoku na label
def after_jump_inst(labels, label, instructions):
    dst = None
    try:
        dst = labels[label]  # cil skoku
    except KeyError:
        error("Snaha o skok na nedefinovane navesti", 52)

    new_order = []
    for pair in instructions:
        if pair[1] >= dst:  # ukladame prvky na poradi stejnem (label) a vyssim nez cil skoku
            new_order.append(pair)
    return new_order


# Kontrola, zda se typ argumentu shoduje s hodnotou
def check_type(_type, value):
    regex = r''
    if _type == 'int':
        regex = r'[-+]?\d+'
    elif _type == 'bool':
        regex = r'((true)|(false))'
    elif _type == 'nil':
        regex = r'(nil)'
    elif _type == 'string':
        regex = r'(([^#\s\\])|(\\\d{3}))*'
    elif _type == 'label':
        regex = r'[a-zA-Z_\-$&%*!?][\w\-$&%*!?]*'
    elif _type == 'type':
        regex = r'int|bool|string|nil|label|type|var|float'
    elif _type == 'var':
        regex = r'(LF|GF|TF)@[a-zA-Z_\-$&%*!?][\w\-$&%*!?]*'
    elif _type == 'float':
        return True
    else:
        error("Neznamy typ {}".format(_type), 52)

    return bool(re.fullmatch(regex, str(value)))


# Funkce pro zpracovani argumentu
def argpar():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=str, help='vstupni soubor s XML reprezentaci zdrojoveho kodu')
    parser.add_argument('--input', type=str, dest="file", help='soubor se vstupy')
    arguments, unknown = parser.parse_known_args()
    if unknown:  # nacteny neznamy parametr/atribut
        sys.stderr.write("Zadany neznamy atribut\n")
        sys.exit(10)

    return arguments


# Funkce pro kontrolu xml
def xml_check(root, source):
    # Osetreni hlavicky xml
    if re.match(r'<\?xml version=(["\'])1\.0\1 encoding=(["\'])UTF-8\2\?>', source[0]) is None:
        error("XML - Nepodporovana deklarace xml souboru", 31)
    # Osetreni root
    if root.tag != 'program' or list(root.attrib)[0] != 'language' \
            or re.fullmatch(r'(IPPcode23)', list(root.attrib.values())[0], re.IGNORECASE) is None:
        error("XML - Spatna deklarace root xml", 32)
    # Kontrola spravnosti formatu instrukci
    for child in root:
        if child.tag != 'instruction':
            error("XML - Chybny zapis hlavicky instrukce", 32)
        child_attr = list(child.attrib.keys())
        # Kontrola klicu instrukce
        try:
            if child_attr[0] != 'order' or child_attr[1] != 'opcode':
                error("XML - Chybny zapis atributu instrukce", 32)
        except IndexError:
            error("XML - Chybejici atribut", 32)
        for subchild in child:
            if (list(subchild.attrib.keys())[0]) != 'type':
                error("XML - Chybny zapis subelementu (arg) instrukce", 32)


# MAIN #
if __name__ == '__main__':
    file = None  # soubor - cteni
    source = None  # soubor - ulozeni xml v listu
    tree = None  # soubor - ulozeni xml v ET
    instructions = []  # seznam instrukci
    order_instructions = []  # seznam instrukci = (instrukce, poradi)
    labels = {}  # definovana navesti = (label: poradi jeji instrukce)

    # Vyhodnoceni parametru funkce
    if ("--help" in sys.argv and len(sys.argv) > 2) or len(sys.argv) == 1:
        error("Chybne zadane atributy", 10)

    if len(sys.argv) >= 2:  # source + import
        args = argpar()
        if args.source is not None:
            try:
                try:
                    tree = ET.parse(args.source)
                except ET.ParseError:
                    error("XML - Chybny xml soubor", 31)
                with open(args.source) as f:
                    source = f.readlines()
            except FileNotFoundError:
                error("Soubor nebyl nalezen", 11)
        else:
            tree = ET.parse(sys.stdin)
            source = sys.stdin.read()

        if args.file is not None:
            try:
                with open(args.file) as f:
                    file = [line.rstrip() for line in f.readlines()]  # nacteni radku bez znaku konce radku
            except FileNotFoundError:
                error("Soubor nebyl nalezen", 11)
        else:
            file = sys.stdin.read()
            file = file.split('\n')  # rozdeleni na radky
            if file[-1] == '':
                file.pop()  # odstraneni posledniho prazdneho radku

    # Nacteni a kontrola spravnosti formatu xml
    root = tree.getroot()
    xml_check(root, source)

    # Nacteni instrukci a jejich argumentu z xml
    for elem in root:
        op_code = list(elem.attrib.values())[1].casefold()
        if op_code not in ['move', 'createframe', 'pushframe', 'popframe', 'defvar', 'call', 'return', 'pushs', 'pops',
                           'add', 'sub', 'mul', 'idiv', 'lt', 'gt', 'eq', 'and', 'or', 'not', 'int2char', 'stri2int',
                           'read', 'write', 'concat', 'strlen', 'getchar', 'setchar', 'type', 'label', 'jump',
                           'jumpifeq', 'jumpifneq', 'exit', 'dprint', 'break', 'clears', 'adds', 'subs', 'muls',
                           'idivs', 'lts', 'gts', 'eqs', 'ands', 'ors', 'nots', 'int2chars', 'stri2ints', 'jumpifeqs',
                           'jumpifneqs', 'int2float', 'float2int', 'div']:
            error("Neznama instrukce {}".format(op_code), 32)
        if op_code in ['return', 'and', 'or', 'not', 'break']:
            op_code = '_' + op_code
        try:
            if int(list(elem.attrib.values())[0]) <= 0:
                error("Zaporne nebo 0 poradi nelze", 32)
        except ValueError:
            error("Zaporne nebo 0 poradi nelze", 32)
        new_instruct = Instruction(list(elem.attrib.values())[0], op_code)
        for subelem in elem:
            arg_type = list(subelem.attrib.values())[0]
            # Kontrola shody typu s promenou
            if not check_type(arg_type, subelem.text):
                error("Nekompatibilni typ: {} s hodnotou: {}".format(arg_type, subelem.text), 32)
            new_instruct.add_argument(arg_type, subelem.text, subelem.tag)
        instructions.append(new_instruct)

    # Serazeni poradi instrukci
    instructions.sort(key=lambda x: x.order)

    # Kontrola duplikatu
    orders = []
    for inst in instructions:
        if inst.order in orders:
            error("Duplicitni poradi instrukci", 32)
        orders.append(inst.order)

    # Ulozeni definovanych navesti a instrukci s jejich poradimi
    only_labels = []  # pomocna promena pro kontrolu duplikatu labels - nacteni pouze nazvu
    for inst in instructions:
        order_instructions.append((inst, int(inst.order)))

        if inst.op_code == 'label':
            if inst.get_arg_type('arg1') != 'label' or inst.get_arg_value('arg1') is None:
                error("Spatne zadana instrukce", 53)
            only_labels.append((inst.get_arg_value('arg1')))
            labels[inst.get_arg_value('arg1')] = int(inst.order)  # ulozeni label a poradi jeji instrukce

    if len(only_labels) != len(set(only_labels)):
        error("Pokus o vytvoreni 2+ stejne pojmenovanych navesti", 52)

    # Vyhodnoceni intepretace
    interpret = Interpret(order_instructions, file, labels)
    interpret.run()
