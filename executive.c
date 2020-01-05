/* 
    Copyright © 2018 by Matteo D'Alessio, Vito Filomeno e Mirko Marasco
*/

#ifdef MULTIPROC
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>

#include "task.h"
#include "busy_wait.h"

typedef enum {
    IDLE,
    RUNNING,
    PENDING
} enum_state;

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t *cond;
  enum_state *state;
} struct_monitor;

typedef struct {
  pthread_t thread;
  unsigned int id;
  struct_monitor *monitor;
} struct_task;

/* Dichiaro gli handler dei thread */
void *p_task_handler(void *data);
void *ap_task_handler(void *data);
void *executive(void *data);

/* Funzione per creare gli n thread periodici */
void create_periodic_thread(struct_monitor *m, struct_task t[]);

/* Funzione per creare l'executive */
pthread_t create_executive_thread( struct_task t[] );

/* Funzione per cercare eventuali deadline miss */
void deadline_check( struct_monitor *m, struct_task t[] );

/* Funzione per gestire i task aperiodici */
void manage_ap_task();

/* Dichiaro le variabili globali */
struct_task ap_task;
struct_monitor monitor_ap;

int ap_flag = 0;
int frame;

void create_periodic_thread( struct_monitor *m, struct_task t[] ){ 
    
    /* Inizializzo il mutex */
  pthread_mutex_init(&m->mutex, NULL);
    
  int i;
  for (i = 0; i < NUM_P_TASKS; ++i) {
      
    pthread_cond_init(&m->cond[i], NULL);
    m->state[i] = IDLE;
    t[i].id = i;
    t[i].monitor = m;

    if(pthread_create(&t[i].thread, NULL, p_task_handler, &t[i]) != 0)
        exit(1);
  }
}

pthread_t create_executive_thread( struct_task t[] ){ 
    
  pthread_attr_t attr;
  struct sched_param param;
  pthread_attr_init(&attr);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  param.sched_priority = sched_get_priority_max(SCHED_FIFO);
  pthread_attr_setschedparam(&attr, &param);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_t executive_t;

  if (pthread_create(&executive_t, &attr, executive, t) != 0)
      exit(1);
    
  return executive_t;
}
void create_aperiodic_thread(){ 
    
  pthread_attr_t ap_attr;
  monitor_ap.cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
  struct sched_param ap_param;
    
  pthread_mutex_init(&monitor_ap.mutex, NULL);
  pthread_cond_init(monitor_ap.cond, NULL);
  ap_task.monitor = &monitor_ap;
  monitor_ap.state = (enum_state *) IDLE;
  
  pthread_attr_init(&ap_attr);
  pthread_attr_setschedpolicy(&ap_attr, SCHED_FIFO);
    
  /* Impostiamo priorità massima meno uno, per eseguirlo con priorità maggiore rispetto agli n thread periodici */
  ap_param.sched_priority= sched_get_priority_max(SCHED_FIFO) - 1; 
    
  pthread_attr_setschedparam(&ap_attr,  &ap_param);
  pthread_attr_setinheritsched(&ap_attr, PTHREAD_EXPLICIT_SCHED);

  if( pthread_create( &ap_task.thread, &ap_attr, ap_task_handler, &ap_task) != 0){
      printf("Errore nella creazione del task aperiodico\n");
      exit(1);
  }
}
void deadline_check( struct_monitor *m, struct_task t[] ){
    
  struct sched_param param;

  int i;
  for (i = 0; i < NUM_P_TASKS; i++) {

  if (m->state[i] == RUNNING) {
    printf("\033[1;31m");
    printf("\n------------------------------\n");
    printf("| DEADLINE MISS DEL TASK: %d  |\n", i);
    printf("| BLOCCO SISTEMA             |\n");
    printf("------------------------------\n");
    printf("\033[0m");

  /* Ripristino la priorità dei task periodici */
  param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
  pthread_setschedparam(t[i].thread, SCHED_FIFO, &param);
}
if(m->state[i] == PENDING) 
  m->state[i] = IDLE;
 }          
}
void manage_ap_task(){
    
            if(ap_flag == 1){
            /* Gestione successivo rilascio del task ap */
            pthread_mutex_lock(&monitor_ap.mutex);
            monitor_ap.state = (enum_state *) PENDING;
            pthread_mutex_unlock(&monitor_ap.mutex);      
        if((SLACK[frame] > 0) && (monitor_ap.state == (enum_state *) PENDING)){
            if( AP_WCET > 0 ){
                AP_WCET -= SLACK[frame];
                printf("AP_WCET residuo : %d\n\n", AP_WCET);
                pthread_cond_signal(monitor_ap.cond);
              } else {
                    ap_flag = 0;
                    pthread_mutex_lock(&monitor_ap.mutex);
                    monitor_ap.state = (enum_state *) IDLE;
                    pthread_mutex_unlock(&monitor_ap.mutex);
             }
       }
            
        if( AP_WCET == 0){
            ap_flag = 0;
            pthread_mutex_lock(&monitor_ap.mutex);
            monitor_ap.state = (enum_state *) IDLE;
            pthread_mutex_unlock(&monitor_ap.mutex);
         }
      }
}

int main() {
    
  /* Inizializzo la SCHEDULE */
  task_init();
    
  struct_monitor monitor;
  struct_task task[NUM_P_TASKS];
    
  /* Alloco memoria dinamica */
  monitor.cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * NUM_P_TASKS);
  monitor.state = (enum_state *)malloc(sizeof(enum_state) * NUM_P_TASKS);
  
  /* Creo i task periodici */
  create_periodic_thread(&monitor, task);
    
  /* Creo il thread dell'executive */
  pthread_t executive_t = create_executive_thread(task);
  
  /* Creo il task aperiodico */
  create_aperiodic_thread();
  
  /* Metto in esecuzione l'executive, sospendendo il main */
  pthread_join(executive_t, NULL);

  /* Libero la memoria */
  pthread_mutex_destroy(&monitor.mutex);
  pthread_mutex_destroy(&monitor_ap.mutex);

  int i;
  for (i = 0; i < NUM_P_TASKS; ++i)
    pthread_cond_destroy(&monitor.cond[i]);
  task_destroy();
  free(monitor.cond);
  free(monitor.state);

  return 0;
}

void *executive(void *data) {
    
    /* Setto una solo CPU per eseguire in monoprocessore */
  cpu_set_t set;
  CPU_ZERO(&set);      
  CPU_SET(0, &set);     
  sched_setaffinity(0, sizeof(cpu_set_t), &set);  

  struct_task *task = (struct_task *)data;
  struct_monitor *monitor = task[0].monitor;

  struct timespec time;
  struct timeval utime;

  gettimeofday(&utime, NULL);

  time.tv_sec = utime.tv_sec;
  time.tv_nsec = utime.tv_usec * 1000;

  pthread_cond_t timedcond;
  pthread_cond_init(&timedcond, NULL);
    
  int num_iperperiodi = 0;
  while (num_iperperiodi < 3) {
    
    int index_task = 0;

    int priority;
    struct sched_param param;

    /* Scorro tutti i frame di un iperperiodo */
    for (frame = 0; frame < NUM_FRAMES; frame++) { 
        
        /* Assegno a priority a partire da 97, così riserso priorità 99 per l'executive e 98 per il task aperiodico */
        priority = sched_get_priority_max(SCHED_FIFO) - 2;

        /* Eseguo il controllo per le deadline miss */
        deadline_check(monitor, task);
        
        index_task = 0;
        printf("\033[0;33m");
        printf("----------------------------\n");
        printf("| ESEGUO IL FRAME NUMERO %d |\n", frame);
        printf("----------------------------\n");
        printf("\033[0m");
        
        /* Gestione delle richieste del task aperiodico */
        manage_ap_task();

        while (SCHEDULE[frame][index_task] != -1) { 
            
            int id = SCHEDULE[frame][index_task];

            if (monitor->state[id] == IDLE ){
                
                /* Setto la priorità in modo decrescente secondo la politica FIFO */
                param.sched_priority = priority;
                priority--;
                pthread_setschedparam(task[id].thread, SCHED_FIFO, &param);
                
                /* I thread diventano pronti e faccio signal sulle condition variables*/
                monitor->state[id] = PENDING;
                pthread_cond_signal(&monitor->cond[id]);
            }

        index_task++; 
      }

       /* modifica busy wait */
      int nanosec = H_PERIOD / NUM_FRAMES * 10000000;
      time.tv_sec += (time.tv_nsec + nanosec) / 1000000000;
      time.tv_nsec = (time.tv_nsec + nanosec) % 1000000000;

      /* timedcond è una condition variable fittizia e time è il tempo assoluto per il quale l'executive deve addormentarsi */
      /* L'executive libera la cpu e vengono eseguiti i task pendenti in ordine di priorità */
      pthread_cond_timedwait(&timedcond, &monitor->mutex, &time);
    }
    num_iperperiodi++;
  }

  pthread_mutex_unlock(&monitor->mutex);
  return NULL;
}

void ap_task_request(){
    
  if (monitor_ap.state == (enum_state *) IDLE){ 
      
    /* Prima richiesta del task aperiodico */
    pthread_mutex_lock(&monitor_ap.mutex);
    printf("\033[32;1m\nRichiesta di un task aperiodico..\n\n\033[0m" );
  	ap_flag = 1;
  	pthread_mutex_unlock(&monitor_ap.mutex);
      
  } else {         
      
    /* Richieste successive del task aperiodico */
    printf("\033[1;31m");
    printf("\nDEADLINE MISS DEL TASK APERIODICO\n");
    printf("\033[0m");
    printf("\033[32;1m\nNuova richiesta del task aperiodico..\n\n\033[0m" );
    pthread_mutex_lock(&monitor_ap.mutex);
    ap_flag = 1;
    monitor_ap.state = (enum_state *) PENDING;
    pthread_mutex_unlock(&monitor_ap.mutex);
      
  }
}

void *p_task_handler(void *data) {

  /* Setto una solo CPU per eseguire in monoprocessore */
  cpu_set_t set;
  CPU_ZERO(&set);      
  CPU_SET(0, &set);     
  sched_setaffinity(0, sizeof(cpu_set_t), &set);  
    
  struct_task *task = (struct_task *) data;
  int id = task->id;
  struct timeval tstart, tend;
  double exc_time;

  while (1) {
      
    pthread_mutex_lock(&task->monitor->mutex);
      
    while (task->monitor->state[id] == IDLE)
      pthread_cond_wait(&task->monitor->cond[id], &task->monitor->mutex);

    task->monitor->state[id] = RUNNING;
    pthread_mutex_unlock(&task->monitor->mutex);

    
    gettimeofday(&tstart, NULL);

    /* Eseguo il task */
    P_TASKS[id]();

    gettimeofday(&tend, NULL);

    exc_time = (tend.tv_sec - tstart.tv_sec) * 1000.0;    
    exc_time += (tend.tv_usec - tstart.tv_usec) / 1000.0; 

    printf("(tempo: %f ms)\n", exc_time);

    pthread_mutex_lock(&task->monitor->mutex);
    task->monitor->state[id] = IDLE;
    pthread_mutex_unlock(&task->monitor->mutex);
      
  }
}

void *ap_task_handler(void *data){

  /* Setto una solo CPU per eseguire in monoprocessore */
  cpu_set_t set;
  CPU_ZERO(&set);      
  CPU_SET(0, &set);     
  sched_setaffinity(0, sizeof(cpu_set_t), &set);  

  while(1){
    pthread_cond_wait(monitor_ap.cond, &monitor_ap.mutex);
    (*AP_TASK)();
  }
    
  return NULL;
}