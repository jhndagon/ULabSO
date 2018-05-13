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

void* load_info(void* proceso);
void print_info(proc_info* pi);
  
int main(int argc, char *argv[]){
  int i;
  // number of process ids passed as command line parameters
  // (first parameter is the program name) 
  int n_procs = argc - 1;
  proc_info* all_proc;
  
  if(argc < 2){
    printf("Error\n");
    exit(1);
  }
  /*Allocate memory for each process info*/
  all_proc = (proc_info *)malloc(sizeof(proc_info)*n_procs);
  assert(all_proc!=NULL);
  
  //create threads
  pthread_t hilo[n_procs];

  // Get information from status file

  for(i = 0; i < n_procs; i++){
    int pid = atoi(argv[i+1]);
    all_proc[i].pid = pid;

    //Thread create
    pthread_create(&hilo[i],NULL, &load_info, &all_proc[i]);



    //load_info(pid, &all_proc[i]);
  }

  for(i = 0; i < n_procs; i++){
    pthread_join(hilo[i],NULL);    
  }
  
  //print information from all_proc buffer
  for(i = 0; i < n_procs; i++){
    //hilo
    print_info(&all_proc[i]);
  }


  // free heap memory
  free(all_proc);
  
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
  printf("Hilo # %lu\n",pthread_self());
  //instancia de
   proc_info *myinfo;
   myinfo = (proc_info *) proceso;

   int c = myinfo->pid;

  sprintf(path, "/proc/%d/status", c);
  fpstatus = fopen(path, "r");
  assert(fpstatus != NULL);
#ifdef DEBUG
  printf("%s\n", path);
#endif // DEBUG
  myinfo->pid = c;
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
void print_info(proc_info* pi){
  printf("PID: %d \n", pi->pid);
  printf("Nombre del proceso: %s", pi->name);
  printf("Estado: %s", pi->state);
  printf("Tamaño total de la imagen de memoria: %s", pi->vmsize);
  printf("Tamaño de la memoria en la región TEXT: %s", pi->vmexe);
  printf("Tamaño de la memoria en la región DATA: %s", pi->vmdata);
  printf("Tamaño de la memoria en la región STACK: %s", pi->vmstk);
  printf("Número de cambios de contexto realizados (voluntarios"
	 "- no voluntarios): %d  -  %d\n\n", pi->voluntary_ctxt_switches,  pi->nonvoluntary_ctxt_switches);
}
