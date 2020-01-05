/* 
    Copyright © 2018 by Matteo D'Alessio, Vito Filomeno e Mirko Marasco
*/

#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>

#include "task.h"
#include "busy_wait.h"

/* Lunghezza dell'iperperiodo */
#define H_PERIOD_ 20

/* Numero di frame */
#define NUM_FRAMES_ 5

/* Numero di task */
#define NUM_P_TASKS_ 5

int ap_exec_index = 0;

void task1_code();
void task2_code();
void task31_code();
void task32_code();
void task33_code();

void ap_task_code();

/* Questo inizializza i dati globali */
const unsigned int H_PERIOD = H_PERIOD_;
const unsigned int NUM_FRAMES = NUM_FRAMES_;
const unsigned int NUM_P_TASKS = NUM_P_TASKS_;

task_routine P_TASKS[NUM_P_TASKS_];
task_routine AP_TASK;
int * SCHEDULE[NUM_FRAMES_];

int SLACK[NUM_FRAMES_];
int AP_WCET;

//int cont_ape=0;

void task_init()
{
	/* Inizializzazione di P_TASKS[] */
	P_TASKS[0] = task1_code;
	P_TASKS[1] = task2_code;
    P_TASKS[2] = task31_code;
    P_TASKS[3] = task32_code;
    P_TASKS[4] = task33_code;

	/* Inizializzazione di AP_TASK */
	AP_TASK = ap_task_code;
	/* Inizializzazione di SCHEDULE e SLACK */

	/* frame 0 */
	SCHEDULE[0] = (int *) malloc( sizeof( int ) * 5 );
	SCHEDULE[0][0] = 0;
    SCHEDULE[0][1] = 1;
    SCHEDULE[0][2] = 1;
    SCHEDULE[0][3] = 2;
    SCHEDULE[0][4] = -1;

	SLACK[0] = 0;

	/* frame 1 */
	SCHEDULE[1] = (int *) malloc( sizeof( int ) * 5 );
	SCHEDULE[1][0] = 0;
    SCHEDULE[1][1] = 3;
    SCHEDULE[1][2] = 3;
    SCHEDULE[1][3] = 3;
    SCHEDULE[1][4] = -1;

	SLACK[1] = 0; /* tutto il frame */

	/* frame 2 */
	SCHEDULE[2] = (int *) malloc( sizeof( int ) * 4 );
	SCHEDULE[2][0] = 0;
    SCHEDULE[2][1] = 1;
    SCHEDULE[2][2] = 1;
    SCHEDULE[2][3] = -1;

	SLACK[2] = 1; /* tutto il frame */

	/* frame 3 */
	SCHEDULE[3] = (int *) malloc( sizeof( int ) * 4 );
	SCHEDULE[3][0] = 0;
    SCHEDULE[3][1] = 1;
    SCHEDULE[3][2] = 1;
    SCHEDULE[3][3] = -1;

	SLACK[3] = 1; /* tutto il frame */

	/* frame 4 */
	SCHEDULE[4] = (int *) malloc( sizeof( int ) * 4 );
	SCHEDULE[4][0] = 0;
    SCHEDULE[4][1] = 1;
    SCHEDULE[4][2] = 1;
    SCHEDULE[4][3] = 4;
    SCHEDULE[4][4] = -1;

	SLACK[4] = 0; /* tutto il frame */


	/* inizializzazione di AP_WCET */
	AP_WCET = 2;

	/* Custom Code */
  busy_wait_init();
}

void task_destroy()
{
	unsigned int i;

	/* Custom Code */

	for ( i = 0; i < NUM_FRAMES; ++i )
		free( SCHEDULE[i] );
}

/* Nota: nel codice dei task e' lecito chiamare ap_task_request() */

void task1_code()
{
    printf("> Task 1: completato! \n");
    busy_wait(3);
}

void task2_code()
{
    printf("> Task 2: completato! \n");
    busy_wait(3);
}

void task31_code()
{
    printf("> Task 31: completato! \n");
    busy_wait(3);
}


void task32_code()
{
    printf("> Task 32: completato! \n");
    busy_wait(3);
}

void task33_code()
{
    printf("> Task 33: completato! \n");
    busy_wait(160);
}

void ap_task_code()
{
	printf("Task aperiodico: completato! \n");
}
