/*
* proj2.c
* IOS - projekt 2 (synchronizace)
* 30. 4. 2022
* Autor: Katerina Cepelkova, xcepel03
* VUT FIT
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // pid_t
#include <sys/mman.h> // mman
#include <semaphore.h> // semafory
#include <unistd.h> // fork
#include <sys/wait.h> // wait


///// STRUKTURY ///// 
typedef struct sem_pair {
    sem_t mutex;          // ochranny semafor
    int value;        // pocitadlo hodnot
} sem_pair_t; // pocitadlo chranene semaforem


///// GLOBALNI PROMENE ///// 
// Globalni promene pro barieru
sem_t *barrier1;
sem_t *barrier2;
sem_t *barrier3;
int *count;

int *noM; // counter na vytvorene molekuly

int *remO; // zbyvajici O
int *remH; // zbyvajici H
sem_t *rem_sem;

int *line; // radek, na ktery se zapisuje
sem_t *line_sem; // semafor pro *line

sem_t *mutex; // ochrana value oxygen a hydrogen
sem_pair_t *hydrogen; // oxygen + oxyQueue
sem_pair_t *oxygen; // hydrogen + hydroQueue


///// POMOCNE FCE ///// 
// Vytvoreni sem_pair -> vrati ukazatel na strukturu, pri chybe NULL
sem_pair_t *sem_pair_init(int sem_val)
{
    sem_pair_t *pair;
    pair = mmap(NULL, sizeof(sem_pair_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (pair == MAP_FAILED)
    {
        fprintf(stderr, "Chybna inicializace sdilene pameti struktury.\n");
        return NULL;
    }
    
    if (sem_init(&pair->mutex, 1, sem_val) == -1)
    {
        fprintf(stderr, "Chybna inicializace semaforu - mutax semaforu.\n");
        return NULL;
    }

    pair->value = 0;
    return pair;
}


// Uvolneni sem_pair
void sem_pair_dest(sem_pair_t *pair)
{
    sem_destroy(&pair->mutex);
    munmap(pair, sizeof(sem_pair_t));
}


// Vytvoreni semaforu -> vrati ukazatel na semafor, pri chybe NULL
sem_t *sem_t_init(int sem_val)
{
    sem_t *sem;
    sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (sem == MAP_FAILED)
    {
        fprintf(stderr, "Chybna inicializace sdilene pameti struktury.\n");
        return NULL;
    }
    
    if (sem_init(sem, 1, sem_val) == -1)
    {
        fprintf(stderr, "Chybna inicializace semaforu - mutax semaforu.\n");
        return NULL;
    }

    return sem;
}


// Uvolneni semaforu
void sem_t_dest(sem_t *sem)
{
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));
}


// Sdileni pameti pro int -> vrati ukazatel na alokovanou pamet, pri chybe NULL
int *mmap_int()
{
    int *ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (ptr == MAP_FAILED)
    {
        fprintf(stderr, "Chybna inicializace sdilene pameti noM.\n");
        return NULL;
    }
    return ptr;
}


// Znovupouzitelna bariera
void barrier(int n)
{
    sem_wait(barrier3);
    (*count)++;
    if (*count == n)
    {
        sem_wait(barrier2);
        sem_post(barrier1);
    }
    sem_post(barrier3);

    sem_wait(barrier1);
    sem_post(barrier1);

    // critical point
    if (*count % n == 0)
        (*noM)++; // pocitadlo nactenych molekul

    sem_wait(barrier3);
    (*count)--;
    if (*count == 0)
    {
        sem_wait(barrier1);
        sem_post(barrier2);
    }
    sem_post(barrier3);

    sem_wait(barrier2);
    sem_post(barrier2);
}

// Generovani O molukul
void oxygen_f(FILE *f, int idO, int TI, int TB)
{
    sem_wait(line_sem);
    fprintf(f, "%d: O %d: started\n", *line, idO);
    (*line)++;
    sem_post(line_sem);
    usleep(1000 * (rand() % (TI + 1)));
    sem_wait(line_sem);
    fprintf(f, "%d: O %d: going to queue\n", *line, idO);
    (*line)++;
    sem_post(line_sem);
    sem_wait(mutex);
    oxygen->value += 1;
    if (hydrogen->value >= 2)
    {
        sem_post(&hydrogen->mutex);
        sem_post(&hydrogen->mutex);
        hydrogen->value -= 2;
        sem_post(&oxygen->mutex);
        oxygen->value -= 1;
    }
    else
        sem_post(mutex);

    // zbrana deadlocku
    if (*remH < 2)
    {
        sem_wait(line_sem);
        fprintf(f, "%d: O %d: not enough H\n", *line, idO);
        (*line)++;
        sem_post(line_sem);
        return;
    }
    sem_wait(&oxygen->mutex); 
    if (*remH < 2)
    {
        sem_wait(line_sem);
        fprintf(f, "%d: O %d: not enough H\n", *line, idO);
        (*line)++;
        sem_post(line_sem);
        sem_post(&oxygen->mutex); 
        return;
    }

    sem_wait(line_sem);
    fprintf(f, "%d: O %d: creating molecule %d\n", *line, idO, *noM + 1);
    (*line)++;
    sem_post(line_sem);

    barrier(3);
    usleep(1000 * (rand() % (TB + 1)));
    sem_wait(line_sem);
    fprintf(f, "%d: O %d: molecule %d created\n", *line, idO, *noM);
    (*line)++;
    sem_post(line_sem);
    sem_post(mutex);
}


// Generovani H molekul
void hydrogen_f(FILE *f, int idH, int TI)
{
    sem_wait(line_sem);
    fprintf(f, "%d: H %d: started\n", *line, idH);
    (*line)++;
    sem_post(line_sem);
    usleep(1000 * (rand() % (TI + 1)));
    sem_wait(line_sem);
    fprintf(f, "%d: H %d: going to queue\n", *line, idH);
    (*line)++;
    sem_post(line_sem);
    sem_wait(mutex);
    hydrogen->value += 1;
    if (hydrogen->value >= 2 && oxygen->value >= 1)
    {
        sem_post(&hydrogen->mutex);
        sem_post(&hydrogen->mutex);
        hydrogen->value -= 2;
        sem_post(&oxygen->mutex);
        oxygen->value -= 1;
    }
    else
        sem_post(mutex);

    // zbrana deadlocku
    if (*remO == 0 || *remH == 1)
    {
        sem_wait(line_sem);
        fprintf(f, "%d: H %d: not enough O or H\n", *line, idH);
        (*line)++;
        sem_post(line_sem);
        return;
    }

    sem_wait(&hydrogen->mutex);
    sem_wait(line_sem);
    fprintf(f, "%d: H %d: creating molecule %d\n", *line, idH, *noM + 1);
    (*line)++;
    sem_post(line_sem);

    barrier(3);
    sem_wait(line_sem);
    fprintf(f, "%d: H %d: molecule %d created\n", *line, idH, *noM);
    (*line)++;
    sem_post(line_sem);
}


///// MAIN ///// 
int main(int argc, char *argv[])
{
    int return_val = 0;
    int NO = 0; // pocitadlo kysliku
    int NH = 0; // pocitadlo vodiku
    int TI = 0; // max cas (ms) cekani vytvorenych atomu pred zazarenim
    int TB = 0; // TB - max cas pro vytvoreni 1 molekuly
    char *digit_check = NULL; // kontrola zadavani ne-cisel
    FILE *f = NULL;

    // Nacteni vstupu
    if (argc == 5) 
    {
        NO = strtol(argv[1], &digit_check, 10);
        if (NO < 0 || *digit_check || digit_check == argv[1])
        {
            fprintf(stderr, "Argument NO mimo ocekavany format.\n");
            return 1;
        }
        NH = strtol(argv[2], &digit_check, 10);
        if (NH < 0 || *digit_check || digit_check == argv[2])
        {
            fprintf(stderr, "Argument NH mimo ocekavany format.\n");
            return 1;
        }
        TI = strtol(argv[3], &digit_check, 10);
        if (TI < 0 || TI > 1000 || *digit_check || digit_check == argv[3])
        {
            fprintf(stderr, "Argument TI mimo ocekavany format.\n");
            return 1;
        }
        TB = strtol(argv[4], &digit_check, 10);
        if (TB < 0 || TB > 1000 || *digit_check || digit_check == argv[4])
        {
            fprintf(stderr, "Argument TB mimo ocekavany format.\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Chybny pocet argumentu -> %d.\n", argc);
        return 1;
    }
    
    // vytvoreni souboru
    f = fopen("proj2.out", "w");
    if (f == NULL)
    {
        fprintf(stderr, "Soubor nelze otevrit.\n");
        return_val = 1;
        goto _file;
    }

    setbuf(f, NULL); // nastaveni postupneho zapisovani

    // inicializace sdilene pameti + semaforu
    // bariery
    barrier1 = sem_t_init(0);
    if (barrier1 == NULL)
    {
        return_val = 1;
        goto _bar1;
    }
    barrier2 = sem_t_init(1);
    if (barrier2 == NULL)
    {
        return_val = 1;
        goto _bar2;
    }
    barrier3 = sem_t_init(1);
    if (barrier3 == NULL)
    {
        return_val = 1;
        goto _bar3;
    }
    count = mmap_int();     
    if (count == NULL)
    {
        return_val = 1;
        goto _count;
    }
    // pocitadlo molekul
    /// noM = mmap_int(noM); TODO zkouska
    noM = mmap_int();
    if (noM == NULL)
    {
        return_val = 1;
        goto _noM;
    }
    // zbyvajici prvky
    remO = mmap_int();
    if (remO == NULL)
    {
        return_val = 1;
        goto _remO;
    }
    remH = mmap_int();
    if (remH == NULL)
    {
        return_val = 1;
        goto _remH;
    }
    rem_sem = sem_t_init(1);
    if (rem_sem == NULL)
    {
        return_val = 1;
        goto _rem_sem;
    }
    // zapisovany radek
    line = mmap_int();
    if (line == NULL)
    {
        return_val = 1;
        goto _line;
    }
    line_sem = sem_t_init(1);
    if (line_sem == NULL)
    {
        return_val = 1;
        goto _line_sem;
    }
    // generace molekul
    mutex = sem_t_init(1);
    if (mutex == NULL)
    {
        return_val = 1;
        goto _mutex;
    }
    hydrogen = sem_pair_init(0);
    if (hydrogen == NULL)
    {
        return_val = 1;
        goto _hydrogen;
    }
    oxygen = sem_pair_init(0);
    if (oxygen == NULL)
    {
        return_val = 1;
        goto _oxygen;
    }

    // inicializace promenych na pocatecni hodnoty
    *count = 0;
    *noM = 0;
    *line = 1;
    *remO = NO;
    *remH = NH;

    // Generovani H2O
    for (int i = 1; i <= NO || i <= NH; i++)
    {
        if (i <= NH)
        {
            pid_t id = fork();
            if (id == -1)
            {
                fprintf(stderr, "Chybny fork.\n");
                return_val = 1;
                goto _fork;
            }
            else if (id == 0)
            {
                hydrogen_f(f, i, TI);
                sem_wait(rem_sem);
                (*remH)--;
                sem_post(rem_sem);
                if (*remH < 2)
                    sem_post(&oxygen->mutex); // uvolneni zablokovaneho O
                return 0;
            }
        }
        if (i <= NO)
        {
            pid_t id = fork();
            if (id == -1)
            {
                fprintf(stderr, "Chybny fork.\n");
                return_val = 1;
                goto _fork;
            }
            else if (id == 0)
            {
                oxygen_f(f, i, TI, TB);
                sem_wait(rem_sem);
                (*remO)--;
                sem_post(rem_sem);
                return 0;
            }
        }
    }

    // Zavirani souboru, cisteni pameti od semaforu a sdilenych pameti
    while(wait(NULL) > 0);
_fork:
_oxygen:
    sem_pair_dest(oxygen);
_hydrogen:    
    sem_pair_dest(hydrogen);
_mutex:
    sem_t_dest(mutex);
_line_sem:
    sem_t_dest(line_sem);
_line:
    munmap(line, sizeof(int));
_rem_sem:
    sem_t_dest(rem_sem);
_remH:
    munmap(remH, sizeof(int));
_remO:
    munmap(remO, sizeof(int));
_noM:
    munmap(noM, sizeof(int));
_count:
    munmap(count, sizeof(int));
_bar3:
    sem_t_dest(barrier3);
_bar2:
    sem_t_dest(barrier2);
_bar1:
    sem_t_dest(barrier1);
_file:
    fclose(f);

    return return_val;
}
