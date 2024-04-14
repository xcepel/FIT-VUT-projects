/**
 *  autor: Katerina Cepelkova, xcepel03
 *  soubor: pwcheck.c
 *  1. IZP Projekt - Prace s textem
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 102 //100 + \n + \0
#define CHAR_NUM 256 //pocet moznosti charu


//Porovnani 2 stringu
bool str_cmp(char *str1, char *str2)
{
    for (int i = 0; str1[i] != '\0' || str2[i] != '\0'; i++)
    {
        if (str1[i] != str2[i])
            return true;
    }
    return false;
}


// Zpracovani vstupnich argumentu
bool args_process(int argc, char *argv[], long *level, long *param)
{   
    char *endptr;   //end pointer == ukazatel na konec str

    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Chyba, zadan spatny pocet argumentu.\n");
        return false;
    }

    //Prevedeni retezce na cislo a kontrola
    *level = strtol(argv[1], &endptr, 10);
    if (*level < 1 || *level > 4 || *endptr != '\0') 
    {
        fprintf(stderr, "Chyba, byl zadan neznamy level %s.\n", argv[1]);
        return false;
    } 

    //Prevedeni retezce na cislo a kontrola
    *param = strtol(argv[2], &endptr, 10);
    if (*param < 1 || *endptr != '\0') 
    {
        fprintf(stderr, "Chyba, byl zadan neznamy parametr %li.\n", *param);
        return false;
    }


    if (argc == 4 && str_cmp(argv[3], "--stats") == true) 
    {
        fprintf(stderr, "Chyba, byl zadan neznamy prikaz %s.\n", argv[3]);
        return false;
    }
    
    return true;
}


//Spocita delku stringu
int str_len(char *str) 
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }

    return len;
}



//Porovna n znaku dvou retezcu 
bool cmp_n_chars(char *str1, char *str2, long n)
{       
    for (int k = 0; k < n; k++)
    {
        if (str1[k] != str2[k])
            return false;
    }

    return true;
}


//Nalezeni velkeho a maleho pismene
bool up_and_low_case(char *str)
{
    bool up = false, low = false;
    
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z') 
            up = true;
        else if (str[i] >= 'a' && str[i] <= 'z')
            low = true;
    }

    return up && low;
}


//Kontroluje, zda retezec obsahuje alespon n skupin znaku
bool contain_groups(char *str, long group_num)
{
    bool up = false, low = false, num = false, spec = false;
  
    if (group_num > 4)
        group_num = 4;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z') 
            up = true;
        else if (str[i] >= 'a' && str[i] <= 'z')
            low = true;
        else if (str[i] >= '0' && str[i] <= '9')
            num = true;
        else if (str[i] >= 32 && str[i] <= 126)
            spec = true;
    }

    return up + low + num + spec >= group_num;
}


//Spocitani nejdelsi sekvence stejnych znaku
int char_sequence(char *str)
{
    int same = 1, max = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == str[i + 1])
            same++;
        else
        {
            if (same > max)
                max = same;
            same = 1;
        }
    }

    return max;
}


//Kontrola, zda retezec obsahuje 2 podretezce delky min
bool substr(char *str, long min)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        for (int j = i + 1; str[j] != '\0'; j++)
        {
            if (cmp_n_chars(&str[i], &str[j], min) == true)
                return true;
        }
    }

    return false;
}


//Spocita pocet true v poli (odpovida poctu unikatnich znaku)
int chars_num(bool chars[])
{
    int num = 0;
    for (int i = 0; i < CHAR_NUM; i++)
    {
       num += chars[i];
    }

    return num;
}


int main(int argc, char *argv[]) 
{
    int passwd_num;
    long level, param;
    char passwd[MAX];
    int min = MAX, sum = 0;
    bool chars[CHAR_NUM] = {false};

    if ((args_process(argc, argv, &level, &param) == false))
        return 1;

    //Nacteni a kontrola, zda kazde heslo splnuje dane podminky
    for (passwd_num = 0; fgets(passwd, MAX, stdin) != NULL; passwd_num++)
    {
        int passwd_len = str_len(passwd);
        //zjisteni, zda je nactene cele heslo
        if (passwd[passwd_len - 1] != '\n')
        {   
            fprintf(stderr, "Chyba, bylo zadano moc dlouhe heslo.\n");
            return 1;
        }
        passwd_len--;   //zruseni konce radku
        passwd[passwd_len] = '\0';

        sum += passwd_len;  //sum = sum + passwd_len
    
        //Zjisteni delky nejkratsiho hesla
        if (passwd_len < min)
            min = passwd_len;

        //Ukladani vyuziti znaku
        for (int i = 0; i < passwd_len; i++)
        {
            chars[(int)passwd[i]] = true;
        }

        if (up_and_low_case(passwd) == false)
            continue;
        
        if (level >= 2 && contain_groups(passwd, param) == false) 
            continue;

        if (level >= 3 && char_sequence(passwd) >= param)
            continue;

        if (level >= 4 && substr(passwd, param) == true)
            continue;
  
        printf("%s\n", passwd);
    }

    //Vypis statistik 
    if (argc == 4)
    {
        printf("Statistika:\n");
        printf("Ruznych znaku: %i\n", chars_num(chars));
        if (min == MAX)	//oprava pro prazdny vstup
        {
            printf("Minimalni delka: 0\n");
            printf("Prumerna delka: 0\n");
        }
        else
        {
            printf("Minimalni delka: %i\n", min);
            printf("Prumerna delka: %.1f\n", (float)sum/passwd_num);
        }
    }

    return 0; 
}
