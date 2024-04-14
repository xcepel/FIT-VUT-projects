<?php
declare(strict_types = 1);

/**
 *
 * VUT projekt - IPP 2022/2023, 2BIT
 * 
 * Katerina Cepelkova, xcepel03
 *
 */


### PREDDEFINOVANE FUNKCE ###
# Kontrola, zda je $word promenna
function var_check($word) {
    if (preg_match('/^(LF|GF|TF)@[a-zA-Z_\-$&%\*!?][\w\-$&%\*!?]*$/', $word))
        return true;
    else {
        lex_synt_error();
    }
}

# Kontrola, zda je $word sybol
function symb_check($word) { # promena nebo konstanta (int x bool x string x nil)
    if (preg_match('/^(LF|GF|TF)@[a-zA-Z_\-$&%\*!?][\w\-$&%\*!?]*$/', $word) || preg_match('/^(int@)(-|\+)?(\d+|0[xX][\da-fA-F]+|0o?[0-7]+)$/', $word) 
        || preg_match('/^(bool@)(true|false)$/', $word) || preg_match('/^(string@)(([^#\s\\\\])|(\\\\\d{3}))*$/', $word) 
        || preg_match('/^(nil@nil)$/', $word))
        return true;
    else {
        lex_synt_error();
    }
}

# Kontrola, zda je $word navesti
function label_check($word) {
    if (preg_match('/^[a-zA-Z_\-$&%\*!?][\w\-$&%\*!?]*$/', $word))
        return true;
    else {
        lex_synt_error();
    }
}

# Kontrola, zda je $word typ
function type_check($word) {
    if (preg_match('/^((int)|(string)|(bool))$/', $word))
        return true;
    else {
        lex_synt_error();
    }
}

# Fukce pro xml na zapis atributu argumentu
function arg_fill($xml, $symbol, $type) {
    if ($type == 'symb') { # je to symbol
        $the_type = explode('@', $symbol)[0]; # presny typ symbolu
        if (preg_match('/(GF)|(LF)|(TF)/', $the_type)) { # promnena
            $xml->writeAttribute('type', 'var');
            $xml->text($symbol);
        } else {
            $xml->writeAttribute('type', $the_type);
            if (!isset(explode('@', $symbol)[1])) {
                $xml->text('');
            } else {
                $const = substr($symbol, strlen($the_type) + 1); # delka typu + zavinac
                #if ($the_type == 'string') { # nepotrebne pro XMLWriter
                #    $const = str_replace(array('&', '<', '>', '\'', '"'), array('&amp;', '&lt;', '&gt;', '&apos;', '&quot;'), $const);
                #}
                $xml->text($const);
            }
        } 
    } else { # neni to symbol
        $xml->writeAttribute('type', $type);
        $xml->text($symbol); 
    }
}

# Funkce pro xml zapis instrukce a jejich atributu
function instruction($xml, $command, $order, $type1, $type2, $type3) {
    $xml->startElement('instruction');
    $xml->writeAttribute('order', strval($order));
    $xml->writeAttribute('opcode', $command[0]);
    # Argumenty
    if (count($command) > 1) {
        $xml->startElement('arg1');
        arg_fill($xml, $command[1], $type1);
        $xml->endElement(); # </arg1>
        if (count($command) > 2) {
            $xml->startElement('arg2');
            arg_fill($xml, $command[2], $type2);
            $xml->endElement(); # </arg2>
            if (count($command) > 3) {
                $xml->startElement('arg3');
                arg_fill($xml, $command[3], $type3);
                $xml->endElement(); # </arg2>
            }
        }
    }
    $xml->endElement(); # </instruction>
}

# Generovani chyby operacni instrukce
function opp_error() {
        fwrite(STDERR, "Neznamy/Chybny operacni kod.\n");
        exit(ExitCode::ERR_OPCODE);
}

# Generovani chybu lexemove/syntakticke
function lex_synt_error() {
        fwrite(STDERR, "Neplatny argument instrukce.\n");
        exit(ExitCode::ERR_LEX_SYNT);
}

# Pokud nalezne hledanou label vrati true, jinak ne
function find_label($array, $label) {
    for ($i = 0; $i < sizeof($array); $i++) {
        if ($array[$i][0] == $label) {
            return true;   
        }
    }
    return false;
}

# vrati pozici label
function found_label($array, $label) {
    for ($i = 0; $i < sizeof($array); $i++) {
        if ($array[$i][0] == $label) {
            return $i;   
        }
    }
    return;
}

# Pokud nalezne hledany op. code vrati true
function find_opcode($array, $wanted) {
    foreach($array as $code => $count) {
        if($code == $wanted) {
            return true;
        }
    }
    return false;
}

ini_set('display_errors', 'stderr'); # chyby na vystup


### TRIDY A PROMENNE ###
class ExitCode {
    const OK = 0;
    const INV_ARG = 10;
    const FILE_VER = 12;
    const ERR_HEAD = 21;
    const ERR_OPCODE = 22; 
    const ERR_LEX_SYNT = 23;
}

# Trida zpracovani --stats
class Statistics {
    public $check = false; # byl pouzit --stats?
    public $file_c = 0; # pocitadlo indexu skupiny v $groups
    # Pocitadla
    public $c_loc = 0; # radky s instrukcemi
    public $c_comments = 0;
    public $c_labels = 0;
    public $c_jumps = 0;
    public $c_backjumps = 0;
    public $c_fwjumps = 0;
    public $c_badjumps = 0;
}

# Trida pro zaznam souboru a jejich parametru
class StatisticFiles {
    public $filename;
    public $params = array(); # pole pro postupne ukladani parametru
}


$stats = new Statistics();
$groups = array(); # skupina pro --stats=file
$label_reg = array(); # registr labelu -> label | active | number of acceses
$op_codes = array(); # asociativni pole na zaznamenavani operacnich kodu

### KOD ###
# Zpracovani parametru
if ($argc != 1) {
    if ($argv[1] == "--help" && $argc == 2) {
        echo("Pouziti:\n");
        echo("\tSpusteni skriptu pomoci php|php8.1 parse.php [parametry] <stdin\n");
	echo("\tParametry:\n");
	echo("\t\t--help = vypise na standardni vystup napovedu skriptu\n");
	echo("\t\t--stats=[file] = do zvoleneho souboru [file] se budou vypisovat nasledne zadane statistiky (moznost zadat vice skupin)\n");
	echo("\t\t\t--loc = pocet radku s instrukcemi\n");
	echo("\t\t\t--comments = pocet radku, na kterych se vyskytoval komentar\n");
	echo("\t\t\t--labels = pocet definovanych navesti\n");
	echo("\t\t\t--jumps = pocet vsech instrukci navratu z volani a instrukci pro skoky\n");
	echo("\t\t\t--fwjumps = pocet doprednych skoku\n");
	echo("\t\t\t--backjumps = pocet zpetnych skoku\n");
	echo("\t\t\t--badjumps = pocet skoku na neexistujici navesti\n");
	echo("\t\t\t--frequent = vypise do statistik jmena nejcastejsich operacnich kodu, podle poctu statickych vyskytu\n");
	echo("\t\t\t--print=[string] = vypise do statistik retezec [string]\n");
	echo("\t\t\t--eol = vytiskne do statistik odradkovani\n");
        exit(ExitCode::OK);
    } else if (preg_match('/(--stats=)\S+/', $argv[1])) {
        $stats->check = true;
        $groups[$stats->file_c] = new StatisticFiles();
        $groups[$stats->file_c]->filename = substr($argv[1], 8); # delka --stats= je 8 znaku
        $pos = 2; # pozice v parametrech
        while ($pos < $argc) {
            switch ($argv[$pos]) {
                case '--loc':
                case '--comments':
                case '--labels':
                case '--jumps':
                case '--fwjumps':
                case '--backjumps':
                case '--badjumps':
                case '--frequent':
                case '--eol':
                    array_push($groups[$stats->file_c]->params, $argv[$pos++]);
                    continue 2;
                default:
                    if (preg_match('/^(--stats=)\S+/', $argv[$pos])) { # zapis nove skupiny
                        $stats->file_c++;
                        $groups[$stats->file_c] = new StatisticFiles();
                        $groups[$stats->file_c]->filename = substr($argv[$pos], 8); # delka --stats= je 8 znaku
                        for ($i = 0; $i < $stats->file_c; $i++) { # Kontrola, zda nezapisujeme 2x skupiny do stejneho souboru
                            if ($groups[$i]->filename == $groups[$stats->file_c]->filename) {
                                fwrite(STDERR, "Nelze zapisovat vicekrat do 1 souboru.\n");
                                exit(ExitCode::FILE_VER);
                            }
                        }
                        $pos++;
                        continue 2;
                    } else if (preg_match('/^(--print=).+/', $argv[$pos])) {
                        array_push($groups[$stats->file_c]->params, substr($argv[$pos], 8)); #delka --print= je 8 znaku
                        $pos++;
                        continue 2;
                    } else {
                        fwrite(STDERR, "Spatny vstup. Pro napovedu pouzijte php|php8.1 parse.php --help.\n");
                        exit(ExitCode::INV_ARG);
                    }
            }
            break;
        }
    } else {
        fwrite(STDERR, "Spatny vstup. Pro napovedu pouzijte php|php8.1 parse.php --help.\n");
        exit(ExitCode::INV_ARG);
    }    
}

# Vytvoreni xml
$xml = new XMLWriter();
$xml->openMemory();
$xml->setIndent(true);
$xml->startDocument('1.0', 'UTF-8');

# Kontrola headeru + odstraneni vseho pred nim
$line = fgets(STDIN);
if ($line != explode('#', $line)[0])
    $stats->c_comments++;
$line = trim(explode('#', $line)[0]);
while ($line == "") { 
    $line = fgets(STDIN);
    if ($line != explode('#', $line)[0])
        $stats->c_comments++;
    $line = trim(explode('#', $line)[0]);
}
if (strcasecmp($line, ".IPPcode23") != 0) {
    fwrite(STDERR, "Spatna/Chybejici hlavicka souboru.\n");
    exit(ExitCode::ERR_HEAD);
}
$xml->startElement('program');
$xml->writeAttribute('language', 'IPPcode23');

# Nacteni a vyhodnoceni zbytku
while ($line = fgets(STDIN)) { # nacitani po radku s whitespace
    if ($line != explode('#', $line)[0])
        $stats->c_comments++; 
    $line = explode('#', $line)[0]; # odstraneni komentaru
    $line = preg_split('/\s+/', trim($line, "\n")); # rozdeleni podle whitespace
    $split = array_filter($line); # oddeleni prazdnych poli
    if (empty($split)) # preskoceni prazdnych radku
        continue;
    $stats->c_loc++;

    $split[0] = strtoupper($split[0]);
    if(find_opcode($op_codes, $split[0])) {
        $op_codes[$split[0]]++; # kod po nekolikate
     } else {
        $op_codes[$split[0]] = 1;  # novy op. kod
     }    
     switch($split[0]) {
        # Prace s ramci, volani fci
        case 'MOVE':
            if (count($split) == 3) {
                if (var_check($split[1]) && symb_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'CREATEFRAME':
        case 'PUSHFRAME':
        case 'POPFRAME':
            if (count($split) == 1) {
                instruction($xml, $split, $stats->c_loc, '', '', '');
                continue 2; 
            } 
            lex_synt_error();
        case 'DEFVAR':
            if (count($split) == 2) {
                if (var_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'var', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'CALL':
            if (count($split) == 2) {
                if (label_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'label', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'RETURN':
            if (count($split) == 1) {
                instruction($xml, $split, $stats->c_loc, '', '', '');
                continue 2;
            } 
            lex_synt_error();
        # Prace s datovym zasobnikem
        case 'PUSHS':
            if (count($split) == 2) {
                if (symb_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'symb', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'POPS':
            if (count($split) == 2) {
                if (var_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'var', '', '');
                    continue 2;
                }
            }
            lex_synt_error();
        # Aritmeticke, relacni, booleovske a konverzni instrukce
        case 'ADD':
        case 'SUB':
        case 'MUL':
        case 'IDIV':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'LT':
        case 'GT':
        case 'EQ':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'AND':
        case 'OR':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'NOT':
            if (count($split) == 3) {
                if (var_check($split[1]) && symb_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'INT2CHAR':
            if (count($split) == 3) {
                if (var_check($split[1]) && symb_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'STRI2INT':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        # Vstupne-vystupni instrukce
        case 'READ':
            if (count($split) == 3) {
                if (var_check($split[1]) && type_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'type', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'WRITE':
            if (count($split) == 2) {
                if (symb_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'symb', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        # Prace s retezci
        case 'CONCAT':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'STRLEN':
            if (count($split) == 3) {
                if (var_check($split[1]) && symb_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'GETCHAR':
        case 'SETCHAR':
            if (count($split) == 4) {
                if (var_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', 'symb');
                    continue 2;
                }
            } 
            lex_synt_error();
        # Prace s typy
        case 'TYPE':
            if (count($split) == 3) {
                if (var_check($split[1]) && symb_check($split[2])) {
                    instruction($xml, $split, $stats->c_loc, 'var', 'symb', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        # Instrukce pro rizeni toku programu
        case 'LABEL':
            if (count($split) == 2) {
                if (label_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'label', '', '');
                    if (find_label($label_reg, $split[1])) {
                        for ($i = 0; $i < sizeof($label_reg); $i++) { # aktivace label
                            if ($label_reg[$i][0] == $split[1]) {
                                $label_reg[$i][1] = true;
                                break;   
                            }
                        }
                    } else {
                        $label_reg[] = array($split[1], true, 0); # vytvoreni label
                    }    
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'JUMP':
            $stats->c_jumps++;
            if (count($split) == 2) {
                if (label_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'label', '', '');
                    if(find_label($label_reg, $split[1])) {
                        if ($label_reg[found_label($label_reg, $split[1])][1] == true) {
                            $stats->c_backjumps++; # back jump
                        } else {
                            $label_reg[found_label($label_reg, $split[1])][2]++; # fw/bad jump
                        }
                    } else {
                        $label_reg[] = array($split[1], false, 1); # new fw/bad jump    
                    }
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'JUMPIFEQ':
        case 'JUMPIFNEQ':
            $stats->c_jumps++;
            if (count($split) == 4) {
                if (label_check($split[1]) && symb_check($split[2]) && symb_check($split[3])) {
                    instruction($xml, $split, $stats->c_loc, 'label', 'symb', 'symb');
                    if(find_label($label_reg, $split[1])) {
                        if ($label_reg[found_label($label_reg, $split[1])][1] == true) {
                            $stats->c_backjumps++; # back jump
                        } else {
                            $label_reg[found_label($label_reg, $split[1])][2]++; # fw/bad jump
                        }
                    } else {
                        $label_reg[] = array($split[1], false, 1); # zalozeni label    
                    }
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'EXIT':
            if (count($split) == 2) {
                if (symb_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'symb', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        # Ladici instrukce
        case 'DPRINT':
            if (count($split) == 2) {
                if (symb_check($split[1])) {
                    instruction($xml, $split, $stats->c_loc, 'symb', '', '');
                    continue 2;
                }
            } 
            lex_synt_error();
        case 'BREAK':
            if (count($split) == 1) {
                instruction($xml, $split, $stats->c_loc, '', '', '');
                continue 2;
            } 
            lex_synt_error();
        default:
            opp_error();
    }
}


$xml->endElement(); # </program>
$xml->endDocument();
echo ($xml->outputMemory());

# Vyhodnoceni stats
if ($stats->check) {
    for ($i = 0; $i <= $stats->file_c; $i++) {
        $name = $groups[$i]->filename;
        $fp = fopen("$name", "w");
        foreach($groups[$i]->params as $par) {
            switch($par) {
                case '--loc':
                    fwrite($fp, "$stats->c_loc\n");
                    continue 2;
                case '--comments':
                    fwrite($fp, "$stats->c_comments\n");
                    continue 2;
                case '--labels': # spocitani vsech jedinecnych aktivnich labels
                    for ($i = 0; $i < sizeof($label_reg); $i++) {
                        if ($label_reg[$i][1] == true) {
                            $stats->c_labels++;   
                        }
                    }
                    fwrite($fp, "$stats->c_labels\n");
                    continue 2;
                case '--jumps':
                    fwrite($fp, "$stats->c_jumps\n");
                    continue 2;
                case '--fwjumps':
                    for ($i = 0; $i < sizeof($label_reg); $i++) {
                        if ($label_reg[$i][1] == true) {
                            $stats->c_fwjumps += $label_reg[$i][2];   
                        }
                    }
                    fwrite($fp, "$stats->c_fwjumps\n");
                    continue 2;
                case '--backjumps':
                    fwrite($fp, "$stats->c_backjumps\n");
                    continue 2;
                case '--badjumps':
                    for ($i = 0; $i < sizeof($label_reg); $i++) {
                        if ($label_reg[$i][1] == false) {
                            $stats->c_badjumps += $label_reg[$i][2];   
                        }
                    }
                    fwrite($fp, "$stats->c_badjumps\n");
                    continue 2;
                case '--frequent':
                    # Vyhodnoceni
                    $most_freq = array();
                    foreach($op_codes as $code => $num) {
                        if ($num == max($op_codes)) {
                            $most_freq[] = $code;
                        }
                    }
                    # Zapis
                    for($i = 0; $i < (sizeof($most_freq) - 1); $i++) {
                        fwrite($fp, $most_freq[$i]);
                        fwrite($fp, ",");
                    }
                    fwrite($fp, $most_freq[$i]);
                    fwrite($fp, "\n");
                    continue 2;
                case '--eol':
                    fwrite($fp, "\n");
                    continue 2;
                default: # print
                    fwrite($fp, "$par\n");
                    continue 2;
            }
        }
        fclose($fp);
    }
}
exit(ExitCode::OK);
?>
