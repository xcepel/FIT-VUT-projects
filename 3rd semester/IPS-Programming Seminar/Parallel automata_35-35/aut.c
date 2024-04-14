// xvasak01

#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <string.h>


using namespace std;

vector<mutex *> zamky; /* pole zamku promenne velikosti */

char *line;
int line_loaded = 0; //(*(zamky[0]))
int score; //(*(zamky[1]))
int done = 0; //(*(zamky[2]))

char *read_line(int *res) {
	string line;
	char *str;
	if (getline(cin, line)) {
		str = (char *)malloc((line.length() + 1));
		strcpy(str, line.c_str());
		*res = 1;
		return str;
	} else {
		*res = 0;
		return NULL;
	}
}


void f(int ID, char *str, int sc) {
	//printf("Thread %i started, with string '%s' assigned score %d\n",ID, str, sc);
	int lines_done = 0;
	while (true) {
		(*(zamky[0])).lock();
		while (0 <= line_loaded && line_loaded <= lines_done) {
			(*(zamky[0])).unlock();
			(*(zamky[0])).lock();
		}
		if (line_loaded == -1) {
			(*(zamky[0])).unlock();
			return;
		}
		(*(zamky[0])).unlock();
		int state = 0;
		//run the automaton on the line
		for (int i = 0; line[i] != '\0'; i++) {
			switch (state) {
				case 0:
					if (line[i] == str[0])
						state = 1;
					continue;
				case 1:
					if (line[i] == str[1])
						state = 2;
					else if (line[i] == str[0])
						state = 1;
					else
						state = 0;
					continue;
				case 2:
					if (line[i] == str[2]) 
						state = 3;
					else if (line[i] == str[0])
						state = 1;
					else
						state = 0;
					continue;
				case 3:
					if (line[i] == str[3]) 
						state = 4;
					else if (line[i] == str[0]) 
						state = 1;
					else 
						state = 0;
					continue;
				case 4:
					if (line[i] == str[4]) {
						(*(zamky[1])).lock();
						score += sc;
						(*(zamky[1])).unlock();
						break;
					}
					else if (line[i] == str[0]) {
						state = 1;
					}
					else {
						state = 0;
					}
					continue;
					
				default:
					fprintf(stderr, "Internal error in thread %i('%s' : %d)\n", ID, str, sc);
					break;
			} //end switch
			break;
		} //end for
		lines_done++;
		(*(zamky[2])).lock();
		done++;
		(*(zamky[2])).unlock();
	} //end while
}


bool str_check(char c) {
        if (c >= 'a' && c <= 'z')
                return true;
        if (c >= 'A' && c <= 'Z')
                return true;
        if (c >= '0' && c <= '9')
                return true;

        return false;
}

int main(int argc, char *argv[]) {
	if (argc < 4 || argc % 2 != 0) {
                printf("USAGE: aut MIN_SCORE STR1 SC1 [ STR2 SC2 ] [ STR3 SC3 ] ...");
		fprintf(stderr, "Wrong number of arguments.\n");
		return 1;
 	}

        char *endptr;
        strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' && endptr == argv[1]) {
	 	fprintf(stderr, "Min_score is not int.\n");
	 	return 1;
        }

	for (int i = 2; i < argc; i += 2) {
        	if (strlen(argv[i]) != 5) {
	 		fprintf(stderr, "Str is not the right length.\n");
	 		return 1;
                }
/*
                for (int j = 0; j < 5; j++) {
                        if (!str_check(argv[i][j])) {
	 		        fprintf(stderr, "Str %s contains wrong char %c.\n", argv[i], argv[i][j]);
	 		        return 1;
                        }
                }
*/
                strtol(argv[i + 1], &endptr, 10);
                if (*endptr != '\0' && endptr == argv[i + 1]) {
	 		fprintf(stderr, "Sc is not int.\n");
	 		return 1;
                }
        }

	/*******************************
 	* Inicializace threadu a zamku
	* *****************************/
	int num = (argc - 2) / 2;
	int num_zamky = 3;
	vector <thread *> threads; /* pole threadu promenne velikosti */

	/* vytvorime zamky */
	zamky.resize(num_zamky); /* nastavime si velikost pole zamky */
	for (int i = 0; i < num_zamky; i++) {	
		mutex *new_zamek = new mutex();
		zamky[i] = new_zamek;
		/* Pokud je treba, tak vytvoreny zamek muzeme rovnou zamknout */
		//(*(zamky[i])).lock();
	}

	/* vytvorime thready */
	threads.resize(num); /* nastavime si velikost pole threads */
	for (int i = 0; i < num; i++) {	
		thread *new_thread = new thread(f, i, argv[2 * (i + 1)], atoi(argv[3 + 2 * i]));
		threads[i] = new_thread;
	}
	/**********************************
	 * Vlastni vypocet aut
	 * ********************************/
	int res;
	line = read_line(&res);
	while (res) {
		(*(zamky[0])).lock();
		line_loaded += 1;
		(*(zamky[0])).unlock();
		

		(*(zamky[2])).lock();
		while (done != num) {
			(*(zamky[2])).unlock();
			(*(zamky[2])).lock();
		}
		done = 0;
		(*(zamky[2])).unlock();

		(*(zamky[1])).lock();
		if (score >= atoi(argv[1])) {
			printf("%s\n", line);
		}
		score = 0;
		(*(zamky[1])).unlock();

		free(line); /* uvolnim pamet */
		line=read_line(&res);
	}
	(*(zamky[0])).lock();
	line_loaded = -1;
	(*(zamky[0])).unlock();


	/**********************************
	 * Uvolneni pameti
	 * ********************************/

	/* provedeme join a uvolnime pamet threads */
	for (int i = 0; i < num; i++) {
		(*(threads[i])).join();
		delete threads[i];
	}
	/* uvolnime pamet zamku */
	for (int i = 0; i < num_zamky; i++) {
		delete zamky[i];
	}

	//printf("everything finished\n");
	return 0;
}
