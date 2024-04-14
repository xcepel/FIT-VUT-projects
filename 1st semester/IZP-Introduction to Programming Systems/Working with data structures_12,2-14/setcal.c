/**
 *  autori: Katerina Cepelkova, xcepel03
 *	    Iva Hlavata, xhlava58
 *  soubor: setcal.c
 *  2. IZP Projekt - Prace s datovymi strukturami
 */
 
 
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAXE 31 //maximalni pocet znaku v prvku + \0
#define OPERATIONS "true", "false", "empty", "card", "complement", "union", "intersect", "minus", "sebseteq", "subset", "equals","reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain", "codomain", "injective", "surjective", "bijective", "closure_ref", "closure_sym", ""

//Struktura pro mnozinu
typedef struct {
    char **elem;
    int elemC;
    int id;
} set;


//Struktura na vice mnozin
typedef struct {
    set *sets;
    int setC;
} set_s;


//Struktura pro relace
typedef struct {
    char **elem1;
    char **elem2;
    int elemC;
    int id;
} rel;


//Struktura pro vice relaci
typedef struct {
    rel *rels;
    int relC;
} rel_s;


//Funkce na tisk mnozin
void print_set(set pset) {
    printf("S");
    for (int i = 0; i < pset.elemC; i++) {
        printf(" %s", pset.elem[i]);
    }
    printf("\n");
}


//Funkce na tisk relaci
void print_rel(rel prel) {
    printf("R");
    for (int i = 0; i < prel.elemC; i++) {
        printf(" (%s %s)", prel.elem1[i], prel.elem2[i]);
    }
    printf("\n");
}


//Funkce na uvolneni mnozin a relaci
void free_all(set_s s, rel_s r) {
    //Uvolneni univerza
    for (int i = 0; i < s.sets[0].elemC; i++) {
        free(s.sets[0].elem[i]);
    }

    //Uvolneni alokaci mnozin
    for (int i = 0; i < s.setC; i++) { //neuvolnuje univerzum na indexu 0
        free(s.sets[i].elem);
    }
    free(s.sets);

    //Uvolneni alokaci relaci
    for (int i = 0; i < r.relC; i++) {
        free(r.rels[i].elem1);
        free(r.rels[i].elem2);
    }
    free(r.rels);
}


//Pokud mnozina s obsahuje elem, vraci true
bool check_duplicity(set *s, char *elem) {
    for (int i = 0; i < s->elemC; i++) {
        if (!strcmp(elem, s->elem[i])) {
            return true;
        }
    }

    return false;
}

//Funkce na nacteni univerza - vraci true/false podle uspesnosti
bool get_universum(FILE *fp, set *universum) {
    char elem[MAXE] = {0};
    char *operations[] = {OPERATIONS};
    int c;

    for (universum->elemC = 0; (c = fgetc(fp)) == ' '; universum->elemC++) {
        fscanf(fp, "%30[a-zA-Z]", elem);

        for (int i = 0; operations[i][0] != '\0'; i++) {
            if (!strcmp(elem, operations[i])) {
                fprintf(stderr, "Univerzum obsahuje zakazany prvek.\n");
                return false;
            }
        }
         
        if (check_duplicity(universum, elem)) {
            fprintf(stderr, "Univerzum obsahuje opakujici se prvek.\n");
            return false;
        }

        void *tmp = realloc(universum->elem, sizeof(*universum->elem) * (universum->elemC + 1));
        if (tmp == NULL) {
            fprintf(stderr, "Nezdarena alokace.\n");
            return false;
        }

        universum->elem = tmp;
        universum->elem[universum->elemC] = malloc(strlen(elem) + 1); // delka retezce + '\0'
        if (universum->elem[universum->elemC] == NULL) {
            fprintf(stderr, "Nezdarena alokace.\n");
            return false;
        }
        strcpy(universum->elem[universum->elemC], elem);
    }

    if (c != '\n') {
        fprintf(stderr, "Neplatny vstup univerza.\n");
        return false;
    }

    return true;
}


//Ziska ukazatel na prvek v universu
char *get_elem(char *elem, set *universum) {
    for (int i = 0; i < universum->elemC; i++) {
        if (!strcmp(elem, universum->elem[i])) {
            return universum->elem[i];
        }
    }

    return NULL;
}


//Funkce na nacteni mnoziny
bool get_set(FILE *fp, int id, set *newset, set *universum) {
    char elem[MAXE];
    int c;

    newset->elem = NULL;

    for (newset->elemC = 0; (c = fgetc(fp)) == ' '; newset->elemC++) {
        fscanf(fp, "%30s", elem);

        void *tmp = realloc(newset->elem, sizeof(*newset->elem) * (newset->elemC + 1));
        if (tmp == NULL) {
            fprintf(stderr, "Nezdarena alokace.\n");
            free(newset->elem);
            return false;
        }

        newset->elem = tmp;
        newset->elem[newset->elemC] = get_elem(elem, universum);
        if (newset->elem[newset->elemC] == NULL) {    //kontrola, zda se prvek nachazi v univerzu 
            fprintf(stderr, "Neexistujici prvek univerza.\n");
            free(newset->elem);
            return false;
        }

        if (check_duplicity(newset, elem)) {
            fprintf(stderr, "Mnozina %d obsahuje opakujici se prvek.\n", id);
            free(newset->elem);
            return false;
        }

    }

    if (c != '\n')
    {
        fprintf(stderr, "Neplatny vstup mnoziny.\n");
        free(newset->elem);
        return false;
    }

    newset->id = id;

    return true;
}        


// Funkce nacitajici relaci
bool get_rel(FILE *fp, int id, rel *newrel, set *universum) {
    char elem1[MAXE];
    char elem2[MAXE + 1];
    int c;
    
    //nacitani i s koncovou zavorkou a nasledne odstraneni
    for (newrel->elemC = 0; (c = fgetc(fp)) == ' '; newrel->elemC++) {
        int scan = fscanf(fp, "(%30s %31s", elem1, elem2);

        int len = strlen(elem2);
        if (elem2[len - 1] != ')' || scan != 2) {
            fprintf(stderr, "Neplatny vstup relace.\n");
            free(newrel->elem1);
            free(newrel->elem2);
            return false;
        }
        else {
            elem2[len - 1] = '\0';
        }

        void *tmp = realloc(newrel->elem1, sizeof(*newrel->elem1) * (newrel->elemC + 1));
        if (tmp == NULL) {
            fprintf(stderr, "Nezdarena alokace.\n");
            free(newrel->elem1);
            free(newrel->elem2);
            return false;
        }

        newrel->elem1 = tmp;
        newrel->elem1[newrel->elemC] = get_elem(elem1, universum);
        if (newrel->elem1[newrel->elemC] == NULL) {
            fprintf(stderr, "Neexistujici prvek univerza.\n");
            free(newrel->elem1);
            free(newrel->elem2);
            return false;
        }

        tmp = realloc(newrel->elem2, sizeof(*newrel->elem2) * (newrel->elemC + 1));
        if (tmp == NULL) {
            fprintf(stderr, "Nezdarena alokace.\n");
            free(newrel->elem1);
            free(newrel->elem2);
            return false;
        }

        newrel->elem2 = tmp;
        newrel->elem2[newrel->elemC] = get_elem(elem2, universum);
        if (newrel->elem2[newrel->elemC] == NULL) {
            fprintf(stderr, "Neexistujici prvek univerza.\n");
            free(newrel->elem1);
            free(newrel->elem2);
            return false;
        }
   
        for (int i = 0; i < newrel->elemC; i++) {
           if ((!strcmp(newrel->elem1[newrel->elemC], newrel->elem1[i]))
                && (!strcmp(newrel->elem2[newrel->elemC], newrel->elem2[i]))) {
               fprintf(stderr, "V relaci %d se nachazi opakujici se prvky.\n", id);
               free(newrel->elem1);
               free(newrel->elem2);
               return false;
           }
        }
    }

    if (c != '\n') {
        fprintf(stderr, "Neplatny vstup relace.\n");
        free(newrel->elem1);
        free(newrel->elem2);
        return false;
    }

    newrel->id = id;

    return true;
}


//Funkce na nalezeni mnoziny podle id
set *get_set_by_id(set_s s, int id) {
    for (int i = 0; i < s.setC; i++) {
        if (s.sets[i].id == id)
            return &s.sets[i];
    }

    return NULL;
}


//Funkce na nacteni relace podle id
rel *get_rel_by_id(rel_s r, int id) {
    for (int i = 0; i < r.relC; i++) {
        if (r.rels[i].id == id)
            return &r.rels[i];
    }

    return NULL;
}


// ---------- FUNKCE NA MNOZINE ---------- 


//EMPTY - zkontroluje, zda je pocet prvku 0
void empty(set *s) {
    if (s->elemC == 0) {
        printf("true");
    }
    else {
        printf("false");
    }
}


//COMPLEMENT - Tiskne prvky nachazejici v univerzu a ne v mnozine
void complement(set *s, set *uni) {
    printf("S");
    for (int i = 0; i < uni->elemC; i++) {
        if (!check_duplicity(s, uni->elem[i])) {
            printf(" %s", uni->elem[i]);
        }
    }
    printf("\n");
}


//UNION - Vytiskne 1 mnozinu a pak prvky, ktere se nenachazeji v 1. z 2.
void unionC(set *s, set *t) {
    printf("S");
    for (int i = 0; i < s->elemC; i++) {
        printf(" %s", s->elem[i]);
    }

    for (int j = 0; j < t->elemC; j++) {
        if (!check_duplicity(s, t->elem[j])) {
            printf(" %s", t->elem[j]);
        }
    }
    printf("\n");
}


//INTERSECT - Tiskne prvky co se nachazi v obou mnozinach
void intersect(set *s, set *t) {
    printf("S");
    for (int i = 0; i < s->elemC; i++) {
        for (int j = 0; j < t->elemC; j++) {
            if (!strcmp(s->elem[i], t->elem[j]))
                printf(" %s", s->elem[i]);
        }
    }
    printf("\n");
}


//MINUS - Vytisknuti prvku z s-mnoziny, ktere se neshoduji s prvky t-mnoziny
void minus(set *s, set *t) {
    printf("S");
    for (int i = 0; i < s->elemC; i++) {
        if (!check_duplicity(t, s->elem[i])) {
            printf(" %s", s->elem[i]);
        }
    }
    printf("\n");
}


//SUBSETEQ - Pokud jsou vsechny prvky s-mnoziny soucasti t-mnoziny vrati true
bool subseteq(set *s, set *t) {
    if (s->elemC > t->elemC) {
        return false;
    }

    for (int i = 0; i < s->elemC; i++) {
        if (!check_duplicity(t, s->elem[i]))
            return false;
    }
    return true;
}


//SUBSET - Pokud jsou vsechny prvky s-mnoziny soucasti t-mnoziny a zaroven s!=t vrati true
void subset(set *s, set *t) {
    if (s->elemC >= t->elemC) {
        printf("false");
        return;
    }

    for (int i = 0; i < s->elemC; i++) {
        if (!check_duplicity(t, s->elem[i])) {
            printf("false");
            return;
        }
    }
    printf("true");
}


//EQUALS - Pokud se mnoziny rovnaji vrati true
void equals(set *s, set *t) {
    if (subseteq(s, t) && subseteq(t, s))
        printf("true");
    else
        printf("false");
}


// ---------- FUNKCE NA RELACICH ---------- 


//REFLEXIVE - Pokud najde stejny pocet prvku stylu(a, a) jako prvku univerza tiskne true
void reflexive(rel *r, set *u) {
    int j = 0;
    for (int i = 0; i < r->elemC; i++) {
        if (!strcmp(r->elem1[i], r->elem2[i]))
            j++;
    }

    if (j == u->elemC) {
        printf("true");
    } 
    else {
        printf("false");
    }
}


//SYMMETRIC - Pro kazdy prvek (a, b) hleda prvek (b, a) - pokud splneno tiskne true
void symmetric(rel *r) {
    for (int i = 0; i < r->elemC; i++) {
        for (int j = 0; j < r->elemC; j++) {
            if (!(strcmp(r->elem1[j], r->elem2[i])) && !(strcmp(r->elem2[j], r->elem1[i]))) {
                break;
            }
            if (j == r->elemC -1) {
                printf("false");
                return;
            }
        }
    }
    printf("true");
}


//ANTISYMMETRIC - Pro kazdy prvek (a, b) hleda prvek (b, a) - pokud ani 1 nenajde tiskne true
void antisymmetric(rel *r) {
    for (int i = 0; i < r->elemC; i++) {
        if (strcmp(r->elem1[i], r->elem2[i])) {
            for (int j = 0; j < r->elemC; j++) {
                if (!(strcmp(r->elem1[j], r->elem2[i])) && !(strcmp(r->elem2[j], r->elem1[i]))) {
                    printf("false");
                    return;
                }
            }
        }
    }
    printf("true");
}


//TRANZITIVE - Pro kazde prvky (a, b) a (b, c) hleda prvek (a, c) - pokud najde tiskne true
void transitive(rel *r) {
    for (int i = 0; i < r->elemC; i++) {
        for (int j = 0; j < r->elemC; j++) {
            if (!strcmp(r->elem2[i], r->elem1[j])) {
                for (int k = 0; k < r->elemC; k++) {
                    if (!(strcmp(r->elem1[i], r->elem1[k])) && !(strcmp(r->elem2[j], r->elem2[k])))
                        break;
                    if (k == r->elemC -1) {
                        printf("false");
                        return;
                    }
                }
            }
        }
    }
    printf("true");
}


//FUNCTION - Pokud se kazdy prvek elem1 nachazi v relaci jen jednou tiskne true
void function(rel *r) {
    for (int i = 0; i < r->elemC; i++) {
        for (int j = i + 1; j < r->elemC; j++) {
            if (!strcmp(r->elem1[i], r->elem1[j])) {
                printf("false");
                return;
            }
        }
    }
    printf("true");
}


//DOMAIN - Kontrola, zda se elem1 s urcitou hodnotou v dane casti nachazi jen 1 - pokud ano tak tisk
void domain(rel *r) {
    int count;
    int k;
    printf("S");
    for (int i = 0; i < r->elemC; i++) {
        count = 0;
        k = 0;
        for (int j = i + 1; j < r->elemC; j++) {
            if (strcmp(r->elem1[i], r->elem1[j]))
                count++;
            k++;
        }
        if (count == k)
            printf(" %s", r->elem1[i]);
    }
    printf("\n");
}


//CODOMAIN - Kontrola, zda se elem2 s urcitou hodnotou v dane casti nachazi jen 1 - pokud ano tisk
void codomain(rel *r) {
    int count;
    int k;
    printf("S");
    for (int i = 0; i < r->elemC; i++) {
        count = 0;
        k = 0;
        for (int j = i + 1; j < r->elemC; j++) {
            if (strcmp(r->elem2[i], r->elem2[j]))
                count++;
            k++;
            
        }
        if (count == k)
            printf(" %s", r->elem2[i]);
    }
    printf("\n");
}


//Funkce zkontroluje jestli je relace r funkci na mnozinach s a t
bool is_function_on(rel *r, set *s, set *t) {
    for (int i = 0; i < r->elemC; i++) { //Nachazi se vsechny prvky elem1 v mnozine s?
        if (!check_duplicity(s, r->elem1[i])) {
            return false;
        }   
    }
    for (int i = 0; i < r->elemC; i++) { //Nachazi se vsechny prvky elem2 v mnozine t?
        if (!check_duplicity(t, r->elem2[i])) {
            return false;
        }
    }

    for (int i = 0; i < r->elemC; i++) {
        for (int j = i + 1; j < r->elemC; j++) {
            if (!strcmp(r->elem1[i], r->elem1[j])) {
                return false;
            }
        }
    }

    return true;
}


//INJECTIVE - Vraci true, pokud kazdy prvek mnoziny s se nachazi v relaci na pozici prvku elem1
bool injective(rel *r) {
    for (int i = 0; i < r->elemC; i++) {
        for (int j = i + 1; j < r->elemC; j++) {
            if (!strcmp(r->elem2[i], r->elem2[j])) {
                return false;
            }
        }
    }
    return true;
}

    
//SURJECTIVE - Vraci true, pokud kazdy prvek mnoziny t se nachazi v relaci na pozici prvku elem2
bool surjective(rel *r, set *t) {
    int k = 0;
    for (int i = 0; i < t->elemC; i++) {
        for (int j = i; j < r->elemC; j++) {
            if (!strcmp(t->elem[i], r->elem2[j])) {
                k++; 
                break;
            }
        }
    }
    return (k == t->elemC);
}


//BIJECTIVE - pokud je relace surjektivni i injektivni tiskne true
bool bijective(rel *r, set *t) {
    if (surjective (r, t) && injective(r)) {
        return true;
    }
    return false;
}


//CLOSURE_REF - Tiskne reflexivni uzaver
void closure_ref(rel *r, set *u) {
    printf("R");
    for (int i = 0; i < r->elemC; i++) {
        printf(" (%s %s)", r->elem1[i], r->elem2[i]);
    }

    for (int i = 0; i < u->elemC; i++) {
        bool print = true;
        for (int j = 0; j < r->elemC; j++) {
            if (!strcmp(u->elem[i], r->elem1[j]) && !strcmp(r->elem1[j], r->elem2[j])) {
                print = false;
                break;
            }     
        }
        if (print) {
            printf(" (%s %s)", u->elem[i], u->elem[i]);
        }
    }
    printf("\n");
}


//CLOSURE_SYM - Tiskne symetricky uzaver
void closure_sym(rel *r) {
    printf("R");
    for (int i = 0; i < r->elemC; i++) {
        printf(" (%s %s)", r->elem1[i], r->elem2[i]);
    }

    for (int i = 0; i < r->elemC; i++) {
        bool print = true;
        for (int j = 0; j < r->elemC; j++) {
            if (!strcmp(r->elem1[i], r->elem2[j]) && !strcmp(r->elem2[i], r->elem1[j])) {
                print = false;
                break;
            }     
        }
        if (print && !strcmp(r->elem1[i], r->elem2[i])) {
            printf(" (%s %s)", r->elem2[i], r->elem1[i]);
        }
    }
    printf("\n");
}


// ---------- FUNKCE VYHODNOCUJICI VSTUPY ---------- 


//Pokud funkce nalezne zadanou mnozinu podle id provede operaci EMPTY, jinak vystup 1
bool f_empty(set_s sets, int id) {
    set *s = get_set_by_id(sets, id);
    if (s == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        empty(s);
        return 0;
    }
}


//Pokud funkce nalezne zadanou mnozinu podle id vytiskne pocet jejich prvku, jinak vystup 1
bool f_card(set_s sets, int id) {
    set *s = get_set_by_id(sets, id);
    if (s == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        printf("%d", s->elemC);
        return 0;
    }
}


//Pokud funkce nalezne zadanou mnozinu podle id provede operaci COMPLEMENT, jinak vystup 1
bool f_complement(set_s sets, int id, set *u) {
    set *s = get_set_by_id(sets, id);
    if (s == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        complement(s, u);
        return 0;
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci UNION, jinak vystup 1
bool f_union(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        unionC(s, t);
        return 0;
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci INTERSECT, jinak vystup 1
bool f_intersect(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        intersect(s, t);
        return 0;
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci MINUS, jinak vystup 1
bool f_minus(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        minus(s, t);
        return 0;
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci SUBSETEQ a vytiskne vysledek, jinak vystup 1
bool f_subseteq(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        if (subseteq(s, t)) {
            printf("true\n");
            return 0;
        }
        else {
            printf("false\n");
            return 0;
        }
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci SUBSET, jinak vystup 1
bool f_subset(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        subset(s, t);
        return 0;
    }
}


//Pokud funkce nalezne zadane mnoziny podle id provede operaci EQUALS, jinak vystup 1
bool f_equals(set_s sets, int id1, int id2) {
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny\n");
        return 1;
    }
    else {
        equals(s, t);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci REFLEXIVE, jinak vystup 1
bool f_reflexive(rel_s rels, int id, set *u) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        reflexive(r, u);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci SYMMETRIC, jinak vystup 1
bool f_symmetric(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        symmetric(r);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci ANTISYMMETRIC, jinak vystup 1
bool f_antisymmetric(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        antisymmetric(r);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci TRANSITIVE, jinak vystup 1
bool f_transitive(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        transitive(r);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci FUNCTION, jinak vystup 1
bool f_function(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        function(r);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci DOMAIN, jinak vystup 1
bool f_domain(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        domain(r);
        return 0;
    }
}


//Pokud funkce nalezne zadanou relaci podle id provede operaci CODOMAIN, jinak vystup 1
bool f_codomain(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        codomain(r);
        return 0;
    }
}


//Pokud funkce nalezne zadane relace a mnoziny podle id provede operaci INJECTIVE a vytiskne vysledek, jinak vystup 1
bool f_injective(rel_s rels, int id_r, set_s sets, int id1, int id2) {
    rel *r = get_rel_by_id(rels, id_r);
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (r == NULL || s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny nebo relace\n");
        return 1;
    }

    if (!is_function_on (r, s, t) || !(injective(r))) {
        printf("false\n");
    }
    else {
        printf("true\n");
    }
    return 0;
}


//Pokud funkce nalezne zadane relace a mnoziny podle id provede operaci SURJECTIVE a vytiskne vysledek, jinak vystup 1
bool f_surjective(rel_s rels, int id_r, set_s sets, int id1, int id2) {
    rel *r = get_rel_by_id(rels, id_r);
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (r == NULL || s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny nebo relace\n");
        return 1;
    }

    if (!is_function_on (r, s, t) || !surjective(r, t)) {
        printf("false\n");
    }
    else {
        printf("true\n");
    }

    return 0;
}


//Pokud funkce nalezne zadane relace a mnoziny podle id provede operaci BIJECTIVE, jinak vystup 1
bool f_bijective(rel_s rels, int id_r, set_s sets, int id1, int id2) {
    rel *r = get_rel_by_id(rels, id_r);
    set *s = get_set_by_id(sets, id1);
    set *t = get_set_by_id(sets, id2);
    if (r == NULL || s == NULL || t == NULL) {
        fprintf(stderr, "Chybne id mnoziny nebo relace\n");
        return 1;
    }

    if (!is_function_on (r, s, t) || (!bijective(r, t))) {
        printf("false\n");
    }
    else {
        printf("true\n");
    }
    return 0;
}


bool f_closure_ref(rel_s rels, int id, set *u) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        closure_ref(r, u);
        return 0;
    }
}


bool f_closure_sym(rel_s rels, int id) {
    rel *r = get_rel_by_id(rels, id);
    if (r == NULL) {
        fprintf(stderr, "Chybne id relace\n");
        return 1;
    }
    else {
        closure_sym(r);
        return 0;
    }
}


//Porovnava command s nazvy prikazu, podle nich se pote vola funkce
//Pri uspechu vraci 0
bool command_test(rel_s rels, set_s sets, set universum, int args, int a1, int b1, int c1, char command[]) {
    if (!strcmp(command, "empty")) {
	if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_empty(sets, a1))
            return 1;
    }
    else if (!strcmp(command, "card")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_card(sets, a1))
            return 1;
    }
    else if (!strcmp(command, "complement")) { 
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
    	if (f_complement(sets, a1, &universum))
            return 1;
    }
    else if (!strcmp(command, "union")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_union(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "intersect")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_intersect(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "minus")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_minus(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "subseteq")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_subseteq(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "subset")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_subset(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "equals")) {
        if (args != 2) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_equals(sets, a1, b1))
            return 1;
    }
    else if (!strcmp(command, "reflexive")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_reflexive(rels, a1, &universum))
            return 1;
    }
    else if (!strcmp(command, "symmetric")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_symmetric(rels, a1))
            return 1;
    }
    else if (!strcmp(command, "antisymmetric")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_antisymmetric(rels, a1))
            return 1;
    }
    else if (!strcmp(command, "transitive")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_transitive(rels, a1))
            return 1;
    }
    else if (!strcmp(command, "function")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_function(rels, a1))
            return 1;
    }
    else if (!strcmp(command, "domain")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_domain(rels, a1))
            return 1;
    }
    else if (!strcmp(command,"codomain")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_codomain(rels, a1))
            return 1;
    }
    else if (!strcmp(command,"injective")) {
        if (args != 3) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_injective(rels, a1, sets, b1, c1))
            return 1;
    }
    else if (!strcmp(command, "surjective")) {
        if (args != 3) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_surjective(rels, a1, sets, b1, c1))
            return 1;
    }
    else if (!strcmp(command, "bijective")) {
        if (args != 3) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_bijective(rels, a1, sets, b1, c1))
            return 1;
    }
    else if (!strcmp(command, "closure_ref")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_closure_ref(rels, a1, &universum))
            return 1;
    }
    else if (!strcmp(command, "closure_sym")) {
        if (args != 1) {
            fprintf(stderr, "Zadany chybny pocet argumentu prikazu %s.\n", command);
            return 1;
        }
        if (f_closure_sym(rels, a1))
            return 1;
    }
    else {
        fprintf(stderr, "Zadan neznamy prikaz.\n");
        return 1;
    }
    return 0;
}


int main(int argc, char *argv[]) {
    int line = 1;
    int c, a1, b1, c1;
    char command[MAXE] = {0};
    set_s sets = {0};
    rel_s rels = {0};

    if (argc != 2) { //kontrola spravneho poctu argumentu
        fprintf(stderr, "Zadan chybny pocet argumentu.\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r"); //otevreni souboru + osetreni chybneho otevreni
    if (fp == NULL) {
        fprintf(stderr, "Soubor nelze otevrit.\n");
        return 1;
    }

    if (fgetc(fp) != 'U') { //Prvni v souboru musi byt definice univerza
        fprintf(stderr, "Chybny format souboru.\n");
        fclose(fp);
        return 1;
    }

    //Nacteni univerza
    void *tmp = malloc(sizeof(*sets.sets)); 
    if (tmp == NULL) {
        fprintf(stderr, "Nezdarena alokace.\n");
        fclose(fp);
        return 1;
    }
    sets.sets = tmp;
    set universum = {NULL, 0, 1}; //Prazdne univerzum s id 1
    if (!get_universum(fp, &universum)) {
        fclose(fp);
        free_all(sets, rels);
        return 1;
    }
    sets.sets[0] = universum; //Ulozeni univerza k mnozim pro pouziti v operacich
    sets.setC++;
    line++; //id univerza = 1, ostatni od 1 dal

    //Tisk univerza
    printf("U");
    for (int i = 0; i < universum.elemC; i++) {
        printf(" %s", universum.elem[i]);
    }
    printf("\n");

    //Nacitani prvniho pismene a ukladani mnoziny/relace -> pri spatnem vstupu chyba
    while ((c = fgetc(fp)) != EOF && c != 'C') {
        //Nacteni mnoziny
        if (c == 'S') {
            void *tmp = realloc(sets.sets, sizeof(*sets.sets) * (sets.setC + 1));
            if (tmp == NULL) {    
                fclose(fp);
                free_all(sets, rels);
                fprintf(stderr, "Nezdarena alokace.\n");
                return 1;
            }
            sets.sets = tmp;
            set newset = {0};
            if (!get_set(fp, line, &newset, &universum)) {
                fclose(fp);
                free_all(sets, rels);
                return 1;
            }
            sets.sets[sets.setC] = newset;
            sets.setC++;
            print_set(newset);
            line++;
        }
        //Nacteni relace
        else if (c == 'R') {
            void *tmp = realloc(rels.rels, sizeof(*rels.rels) * (rels.relC + 1));
            if (tmp == NULL) {    
                fclose(fp);
                free_all(sets, rels);
                fprintf(stderr, "Nezdarena alokace.\n");
                return 1;
            }
            rels.rels = tmp;
            rel newrel = {0};
            if (!get_rel(fp, line, &newrel, &universum)) {
                fclose(fp);
                free_all(sets, rels);
                return 1;
            }
            rels.rels[rels.relC] = newrel;
            rels.relC++;
            print_rel(newrel);
            line++;
        }
        else {
            fprintf(stderr, "Chybny format vstupu.\n");
            fclose(fp);
            free_all(sets, rels);
            return 1;
        }
    }

    if (sets.setC == 1 && rels.relC == 0) { // S - na 1. pozici univerzum
        fprintf(stderr, "Chybny format vstupu - chybi mnoziny/relace.\n");
        fclose(fp);
        free_all(sets, rels);
        return 1;
    }
    
    if (c != 'C') { //Pokud po mnozinach/relacich nenasleduji prikazy -> chyba
        fprintf(stderr, "Chybny format vstupu - chybi prikaz.\n");
        fclose(fp);
        free_all(sets, rels);
        return 1;
    }

    //Nacitani a provadeni operaci/prikazu
    while (c != EOF) {
        int err;
        a1 = 0;
        b1 = 0;
        c1 = 0;
        if (c == 'C') {
            err = fscanf(fp, "%s %d", command, &a1); //Nacteni operace a prvniho id
            c = fgetc(fp);
            if (c != '\n') {
                err += fscanf(fp, "%d", &b1); //Nacteni druheho id
                c = fgetc(fp);
            }
            if (c != '\n') {
                err += fscanf(fp, "%d", &c1); //Nacteni tretiho id
                c = fgetc(fp);
            }
            if (c != '\n' || err < 2) {
                fprintf(stderr, "Chybny format vstupu.\n");
                fclose(fp);
                free_all(sets, rels);
                return 1;
            }

            //Zavolani funkce, ktera porovnavana command s nazvy prikazu, podle nich se pote volaji funkce
            //Pri uspechu vraci 0
            if (command_test(rels, sets, universum, err - 1, a1, b1, c1, command)) {
                fclose(fp);
                free_all(sets, rels);
		return 1;	
	    }
        }
        else {
            fprintf(stderr, "Chybny format vstupu.\n");
            fclose(fp);
            free_all(sets, rels);
            return 1;
        }
        c = fgetc(fp);
    }

    fclose(fp);
    free_all(sets, rels); 
    return 0;
}
