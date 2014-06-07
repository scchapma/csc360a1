/*
 * CSC 360, Summer 2014
 * 
 * Assignment #1
 *
 * Interstellar-space problem: skeleton.
 */

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>


/* Random # below threshold indicates H; otherwise C. */
#define ATOM_THRESHOLD 0.55
#define DEFAULT_NUM_ATOMS 50
#define TRUE   1
#define FALSE  0
#define BUFFER_SIZE 3
#define CARBON_SIZE 2
#define HYDROGEN_SIZE 1
#define CARBON 1
#define HYDROGEN 0


/* Global / shared variables */
int  cNum = 0, hNum = 0;
long numAtoms;
sem_t c_sem, h_sem;
sem_t c_mutex, h_mutex;


/* Add buffer - array of pthreads, size 3 */
pthread_t *buffer [BUFFER_SIZE];


/* Whatever needs doing for your global data structures, do them here. */
void init()
{
    /* init buffer to all null */
    int i;
    for(i=0; i<BUFFER_SIZE; i++){
        if(!buffer[i]){
            fprintf(stdout, "Buffer %d is null.\n", i);
        }
    }
    
    /*init semaphores*/
    int init_c = sem_init(&c_sem, 0, CARBON_SIZE);
    int init_h = sem_init(&h_sem, 0, HYDROGEN_SIZE);
    int init_c_mutex = sem_init(&c_mutex, 0, 1);
    int init_h_mutex = sem_init(&h_mutex, 0, 1);
    
    /*check for nulls */
    if(init_c != 0 || init_h != 0 || init_c_mutex != 0 || init_h_mutex != 0){
        fprintf(stdout, "Exiting - failed to initialize the semaphores.\n");
        exit(1);
    }
    
}


/* Needed to pass legit copy of an integer argument to a pthread */
int *dupInt( int i )
{
	int *pi = (int *)malloc(sizeof(int));
	assert( pi != NULL);
	*pi = i;
	return pi;
}


void makeRadical(int atom, int type)
{
    /* Change this message into something meaningful in the sense
     * that either a C or an H will arrive to find enough atoms
     * present for a new radical, and that newly arrived atom will
     * triggers the actions that resume blocked atoms.
     */
	fprintf(stdout, "A ethynyl radical was made by actions of %d\n",
        atom);
}


void *hReady( void *arg )
{
	int id = *((int *)arg);
	printf("h%d is alive\n", id);

    /* Other things happen past this point... */
    
    
    
}


void *cReady( void *arg )
{
	int id = *((int *)arg);
	printf("c%d is alive\n", id);

    /* Other things happen past this point... */
}



int main(int argc, char *argv[])
{
	long seed;
	numAtoms = DEFAULT_NUM_ATOMS;
	pthread_t **atom;
	int i;
	int status;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: %s <seed> [<num atoms>]\n", argv[0]);
		exit(1);
	}

	if ( argc >= 2) {
		seed = atoi(argv[1]);
	}

	if (argc == 3) {
		numAtoms = atoi(argv[2]);
		if (numAtoms < 0) {
			fprintf(stderr, "%ld is not a valid number of atoms\n",
				numAtoms);
			exit(1);
		}
	}

	init();
	srand(seed);
	atom = (pthread_t **)malloc(numAtoms * sizeof(pthread_t *));
	assert (atom != NULL);
	for (i = 0; i < numAtoms; i++) {
		atom[i] = (pthread_t *)malloc(sizeof(pthread_t));
		if ( (double)rand()/(double)RAND_MAX < ATOM_THRESHOLD ) {
			hNum++;
			status = pthread_create (
					atom[i], NULL, hReady,
					(void *)dupInt(hNum)
				);
		} else {
			cNum++;
			status = pthread_create (
					atom[i], NULL, cReady,
					(void *)dupInt(cNum)
				);
		}
		if (status != 0) {
			fprintf(stderr, "Error creating atom thread\n");
			exit(1);
		}
	}
    
    fprintf(stdout, "Carbon atoms: %d.\n", cNum);
    fprintf(stdout, "Hydrogen atoms: %d.\n", hNum);

	/*
	 * Now the tricky bit begins....  All the atoms are allowed
	 * to go their own way, but how does the Interstellar Space
	 * problem terminate? There is a non-zero probability that
	 * some atoms will not become part of a radical; that is,
	 * many atoms may be blocked on some semaphore variable of
	 * our own devising. How do we ensure the program ends when
	 * (a) all possible radicals have been created and (b) all
	 * remaining atoms are blocked (i.e., not on the ready queue)?
	 */
}
