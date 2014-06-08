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
#include <errno.h>


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
sem_t mutex;
int radical_counter = 0;
int max_molecules = 0, final_max_molecules = 0;


/* Add buffer - array of pthreads, size 3 */
int h_buffer [HYDROGEN_SIZE];
int c_buffer [CARBON_SIZE];


/* Whatever needs doing for your global data structures, do them here. */
void init()
{
    /* init buffer to all null */
    int i;
    for(i=0; i<HYDROGEN_SIZE; i++){
        if(!h_buffer[i]){
            fprintf(stdout, "\nHydrogen buffer %d is null.\n", i);
        }
    }

    for(i=0; i<CARBON_SIZE; i++){
        if(!c_buffer[i]){
            fprintf(stdout, "Carbon buffer %d is null.\n", i);
        }
    }
    
    /*init semaphores*/
    int init_c = sem_init(&c_sem, 1, CARBON_SIZE);
    int init_h = sem_init(&h_sem, 1, HYDROGEN_SIZE);
    int init_c_mutex = sem_init(&c_mutex, 0, 1);
    int init_h_mutex = sem_init(&h_mutex, 0, 1);
    int init_mutex = sem_init(&mutex, 1, 1);
    
    /*check for nulls */
    if(init_c != 0 || init_h != 0 || init_c_mutex != 0 || init_h_mutex != 0 || init_mutex != 0){
        fprintf(stdout, "Exiting - failed to initialize the semaphores.  Error: %d\n", errno);
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

int compute_max_molecules (int hNum, int cNum){ 
	int max_possible_molecules = cNum/2;
	/*take min of theoretical max (given #C) and #H*/
	if(max_possible_molecules > hNum) max_molecules = hNum;
	else max_molecules = max_possible_molecules;
	fprintf(stdout, "Max molecules: %d\n\n", max_molecules);
	return max_molecules;
}


void makeRadical(int atom, int type)
{
    /* Change this message into something meaningful in the sense
     * that either a C or an H will arrive to find enough atoms
     * present for a new radical, and that newly arrived atom will
     * triggers the actions that resume blocked atoms.
     */

  radical_counter++;
  char type_char; 
  if(type == HYDROGEN) type_char = 'H';
  else type_char = 'C';

  fprintf(stdout, "A ethynyl radical was made by actions of %c%d.\n", type_char, atom);
  fprintf(stdout, "Radical composition: H%d, C%d, C%d.\n", h_buffer[0], c_buffer[0], c_buffer[1]);
  fprintf(stdout, "Molecule #%d.\n\n", radical_counter);

	/*reset buffers*/
  h_buffer[0] = c_buffer[0] = c_buffer[1] = 0;
  /*fprintf(stdout, "H0 buffer: %d\n", h_buffer[0]);
  fprintf(stdout, "C0 buffer: %d\n", c_buffer[0]);
  fprintf(stdout, "C1 buffer: %d\n", c_buffer[1]);*/
  
  /*reset semaphores for h and c*/
  sem_post(&h_sem);
  sem_post(&c_sem);
  sem_post(&c_sem);

  /*track max computations */
  max_molecules = compute_max_molecules(hNum, cNum);
  if(radical_counter==max_molecules){
    fprintf(stdout, "Radical counter = max_molecules: %d\n", radical_counter);
  }else{
    fprintf(stdout, "Radical counter != max_molecules.  RC: %d, MM: %d\n", radical_counter, max_molecules);
  }

  /*when radical_counter == max_molecules, do something */ 
  if(radical_counter==final_max_molecules){
    fprintf(stdout, "Radical counter = final_max_molecules.\n");
    /*make calls to release remaining threads */
    int h_remaining = hNum - radical_counter;
    int c_remaining = cNum - (2*radical_counter);
    int i,j;
    for (i=0; i<h_remaining; i++){
      sem_post(&h_sem);
    }
    for (i=0; i<c_remaining; i++){
      sem_post(&c_sem);
    }
  }
	
}


void *hReady( void *arg )
{
	int id = *((int *)arg);
	sem_wait(&mutex);
	printf("h%d is alive\n", id);
	sem_post(&mutex);

    /* Other things happen past this point... */

	sem_wait(&h_sem);
	sem_wait(&mutex);

	/*if hydrogen buffer empty, add hydrogen atom*/
	if(!h_buffer[0]){
	  h_buffer[0] = id;
	  fprintf(stdout, "Hydrogen atom #%d in buffer.\n", id);
	}

	/*if carbon buffer full, call makeRadical*/
	if(c_buffer[0] && c_buffer[1]){
	  makeRadical(id, HYDROGEN);
	}

	sem_post(&mutex);
	/*sem_post(&h_sem);*/
	
	sem_wait(&mutex);
	printf("return h%d\n", id);
	sem_post(&mutex);
    
}


void *cReady( void *arg )
{
	int id = *((int *)arg);
	sem_wait(&mutex);
	printf("c%d is alive\n", id);
	sem_post(&mutex);

    /* Other things happen past this point... */
	
	sem_wait(&c_sem);
	sem_wait(&mutex);	

	/*if carbon buffer not full, add carbon atom*/
	if(!c_buffer[0]){
	  c_buffer[0] = id;
	  fprintf(stdout, "Carbon atom #%d in buffer 0.\n", id);
	}else if(!c_buffer[1]){
	  fprintf(stdout, "Carbon atom #%d in buffer 1.\n", id);
	  c_buffer[1] = id;
	}

	/*if hydrogen buffer full, call makeRadical*/
	if(h_buffer[0] && c_buffer[0] && c_buffer[1]){
	  makeRadical(id, CARBON); 
	}
	
	sem_post(&mutex);
	/*sem_post(&c_sem);*/

	sem_wait(&mutex);
	printf("return c%d\n", id);
	sem_post(&mutex);
	
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

	final_max_molecules = compute_max_molecules(hNum, cNum);

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

	fprintf(stdout, "Carbon atoms: %d\n", cNum);
	fprintf(stdout, "Hydrogen atoms: %d\n", hNum);

	/*given #H and #C, determine # of radicals that can be produced*/	 
	/*int max_possible_molecules = cNum/2;*/
	/*take min of theoretical max (given #C) and #H*/
	/*if(max_possible_molecules > hNum) max_molecules = hNum;
	else max_molecules = max_possible_molecules;
	fprintf(stdout, "Max molecules: %d\n\n", max_molecules);*/

	/* join threads */
	for (i=0; i<numAtoms; i++){
	  pthread_join(*atom[i], NULL);
	}

	exit(0);
}
