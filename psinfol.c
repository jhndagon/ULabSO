/**
 *   \file psinfo-l.c
 *   \brief base code for the program psinfo-l
 *          
 *  This program prints some basic information for a given 
 *  list of processes.
 *  You can use this code as a basis for implementing parallelization 
 *  through the pthreads library. 
 *
 *   \author: Danny Munera - Sistemas Operativos UdeA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_BUFFER 150
#define BUFFER_PROC 3 //# de procesos en buffer

//variables globales
sem_t ubicar;
sem_t imprimir;

int n_procs; //# de procesos pasados como argumentos
int pid_busqueda; //pid a buscar para imprimir


//#define DEBUG
typedef struct p_ {
  int pid;
  char name[MAX_BUFFER];
  char state[MAX_BUFFER];
  char vmsize[MAX_BUFFER];
  char vmdata[MAX_BUFFER];
  char vmexe[MAX_BUFFER];
  char vmstk[MAX_BUFFER];
  int voluntary_ctxt_switches;
  int nonvoluntary_ctxt_switches;
} proc_info;

proc_info* all_proc; //buffer

void* load_info(void* proceso);
void* print_info();
  
int main(int argc, char *argv[]){
  int i;
  // number of process ids passed as command line parameters
  // (first parameter is the program name) 
  n_procs = argc - 1;
  
  
  if(argc < 2){
    printf("Error\n");
    exit(1);
  }
  /*Allocate memory for each process info*/
  all_proc = (proc_info *)malloc(sizeof(proc_info)*BUFFER_PROC);
  assert(all_proc!=NULL);
  
  //create threads
  pthread_t hilo[n_procs];
  pthread_t imprime;

  //inicializacion de semaforos
  sem_init(&ubicar,0, 2);
  sem_init(&imprimir,0,0);

  pthread_create(&imprime,NULL,print_info,NULL);

  
  for(i = 0; i < n_procs; i++){
    sem_wait(&ubicar);

    for(int j = 0; j < BUFFER_PROC; j++){
        int pid = atoi(argv[i+1]);
        if(all_proc[j].pid == 0){
          all_proc[j].pid = pid;
          
          //Thread create

          pthread_create(&hilo[i],NULL, &load_info, &all_proc[j]);
          sem_post(&imprimir);
          break;
        }
    }
    //load_info(pid, &all_proc[i]);
  }

  for(i = 0; i < n_procs; i++){
    pthread_join(hilo[i],NULL);    
  }

  //espera de hilo imprimir
  pthread_join(imprime,NULL);



  // free heap memory
  free(all_proc);
  sem_destroy(&ubicar);
  sem_destroy(&imprimir);
  
  return 0;
}

/**
 *  \brief load_info
 *
 *  Load process information from status file in proc fs
 *
 *  \param pid    (in)  process id 
 *  \param myinfo (out) process info struct to be filled
 */
void* load_info(void* proceso){
  FILE *fpstatus;
  char buffer[MAX_BUFFER]=""; 
  char path[MAX_BUFFER]="";
  char* token;

  //instancia de
   proc_info *myinfo;
   myinfo = (proc_info *) proceso;

   int c = myinfo->pid;
   //pid_busqueda = c;

  sprintf(path, "/proc/%d/status", c);
  fpstatus = fopen(path, "r");
  assert(fpstatus != NULL);
#ifdef DEBUG
  printf("%s\n", path);
#endif // DEBUG
  //myinfo->pid = c;
  while (fgets(buffer, MAX_BUFFER, fpstatus)) {
    token = strtok(buffer, ":\t");
    if (strstr(token, "Name")){
      token = strtok(NULL, ":\t");
#ifdef  DEBUG
      printf("%s\n", token);
#endif // DEBUG
      strcpy(myinfo->name, token);
    }else if (strstr(token, "State")){
      token = strtok(NULL, ":\t");
      strcpy(myinfo->state, token);
    }else if (strstr(token, "VmSize")){
      token = strtok(NULL, ":\t");
      strcpy(myinfo->vmsize, token);
    }else if (strstr(token, "VmData")){
      token = strtok(NULL, ":\t");
      strcpy(myinfo->vmdata, token);
    }else if (strstr(token, "VmStk")){
      token = strtok(NULL, ":\t");
      strcpy(myinfo->vmstk, token);
    }else if (strstr(token, "VmExe")){
      token = strtok(NULL, ":\t");
      strcpy(myinfo->vmexe, token);
    }else if (strstr(token, "nonvoluntary_ctxt_switches")){
      token = strtok(NULL, ":\t");
      myinfo->nonvoluntary_ctxt_switches = atoi(token);
    }else if (strstr(token, "voluntary_ctxt_switches")){
      token = strtok(NULL, ":\t");
      myinfo->voluntary_ctxt_switches = atoi(token);
    }
#ifdef  DEBUG
    printf("%s\n", token);
#endif

  }
  fclose(fpstatus);
  
  return NULL;
}
/**
 *  \brief print_info
 *
 *  Print process information to stdout stream
 *
 *  \param pi (in) process info struct
 */ 
void* print_info(){
  

  //hacer el ciclo para recorrer el pid!=0 a buscar
  proc_info* pi;
  int i = 0;
  int procesos_impresos = 0;
  while(1){
    sem_wait(&imprimir);
    
    while(1){
      if(i == BUFFER_PROC){
        i = 0;
      }
      if(all_proc[i].pid != 0){
        pi = &all_proc[i];
        printf("PID: %d \n", pi->pid);
        printf("Nombre del proceso: %s", pi->name);
        printf("Estado: %s", pi->state);
        printf("Tamaño total de la imagen de memoria: %s", pi->vmsize);
        printf("Tamaño de la memoria en la región TEXT: %s", pi->vmexe);
        printf("Tamaño de la memoria en la región DATA: %s", pi->vmdata);
        printf("Tamaño de la memoria en la región STACK: %s", pi->vmstk);
        printf("Número de cambios de contexto realizados (voluntarios"
        "- no voluntarios): %d  -  %d\n\n", pi->voluntary_ctxt_switches,  pi->nonvoluntary_ctxt_switches);
        all_proc[i].pid = 0;
        sem_post(&ubicar);
        procesos_impresos++;
        break;
      }
      i++;      
    }   

    if(procesos_impresos>=n_procs){
      break;
    }
  }
   return NULL;
}
