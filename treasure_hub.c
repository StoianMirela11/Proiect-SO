#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

pid_t monitor_pid=0;

void meniu()
{
  printf("1) start_monitor - porneste un proces care monitorizeaza vanatorile\n");
  printf("2) list_hunts - cere monitorului sa listeze vanatorile si numarul total de comori pentru fiecare\n");
  printf("3) list_treasures - afiseaza comorile dintr-o vanatoare\n");
  printf("4) view_treasure - cere monitorului sa afiseze o comoara dintr-o anumita vanatoare\n");
  printf("5) stop_monitor - cere monitorului sa se oreasca si sa se intoarca la prompt\n");
  printf("6) exit - incheie programul\n");
}

void sigchld_handler(int sig)
{
  int status;
  wait(&status);
  if(WIFEXITED(status))
    {
      printf("Monitorul s-a terminat cu codul %d\n", WEXITSTATUS(status));
    }
  monitor_pid=0;
}

void start_monitor()
{
  monitor_pid=fork();

  if(monitor_pid < 0)
    {
    perror("Operatia nu s-a putut efectuta!");
    exit(EXIT_FAILURE);
    }
  if(monitor_pid==0)
    {
      execl("./treasure_manager", "./treasure_manager", NULL);//un nou proces care sa ruleze un program diferit de cel al parintelui
      perror("Eroare la crearea unui nou proces");
      exit(EXIT_FAILURE);
    }
  //codul parintelui
  printf("Monitorul a fost deschid cu PID: %d\n",monitor_pid);
}

void comenzi(char* command)
{
  int fisier=open("comenzi.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fisier==-1)
    {
      perror("Eroare la deschiderea fisierului");
      exit(EXIT_FAILURE);
    }
  if(write(fisier, command,strlen(command))==-1)
    {
      perror("Eroare la scrierea in fisier");
      close(fisier);
      exit(EXIT_FAILURE);
    }

  close(fisier);
}

void trimite_comanda(char* comanda, int semnal)
{
  comenzi(comanda);
  if(monitor_pid!=0)
    {
      kill(monitor_pid, semnal);
    }
  else
    {
      printf("Monitorul nu ruleaza.\n");
    }
}


int main()
{

  setbuf(stdout, NULL);
  
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  //memset(&sa, 0x00, sizeof(struct sigaction));

  sigaction(SIGCHLD, &sa, NULL);
  
  char command[256];
  while(1)
    {
      meniu();
      printf(">>Introduceti comanda: \n>>");
      fflush(stdout);
      if(fgets(command, sizeof(command), stdin)==NULL)
	{
	  if (errno == EINTR)
	    {
	      continue;
	    }
	  perror("Eroare la citirea comenzii");
	  exit(EXIT_FAILURE);
	}
      command[strcspn(command, "\n")]=0; //stergem caracterul de \n

      if(strcmp(command,"start_monitor")==0)
	{
	  start_monitor();
	}
      else if (strcmp(command, "list_hunts")==0)
	{
	  trimite_comanda("list_hunts", SIGUSR1);
	}
      else if(strcmp(command, "list_treasures")==0)
	{
	  char vanatoare[20];
	  printf("Introdu numele vanatorii: ");
	  if(fgets(vanatoare, sizeof(vanatoare), stdin)==NULL)
	    {
	      perror("Eroare la citirea vanatorii");
	      exit(EXIT_FAILURE);
	    }
	  vanatoare[strcspn(vanatoare,"\n")]=0;
	  char comanda_completa[128];
	  snprintf(comanda_completa, sizeof(comanda_completa), "list_treasures %s", vanatoare);
	  trimite_comanda(comanda_completa, SIGUSR1);
	}
      else if(strcmp(command, "view_treasure")==0)
	{
	  char vanatoare[20];
	  char comoara[20];
	  printf("Introdu numele vanatorii: ");
	  if(fgets(vanatoare, sizeof(vanatoare), stdin)==NULL)
	    {
	      perror("Eroare la citirea vanatorii");
	      exit(EXIT_FAILURE);
	    }
	  vanatoare[strcspn(vanatoare,"\n")]=0;
	  printf("Introdu numele comorii: ");
	  if(fgets(comoara, sizeof(comoara), stdin)==NULL)
	    {
	      perror("Eroare la citirea comorii");
	      exit(EXIT_FAILURE);
	    }
	  comoara[strcspn(comoara, "\n")]=0;
	  char comanda_completa[128];
	  snprintf(comanda_completa, sizeof(comanda_completa), "view_treasure %s %s", vanatoare, comoara);
	  trimite_comanda(comanda_completa, SIGUSR2);
	}
      else if(strcmp(command, "stop_monitor")==0)
	{
	  trimite_comanda(command, SIGTERM);
	}
      else if(strcmp(command, "exit")==0)
	{
	  if(monitor_pid!=0 && kill(monitor_pid, 0)==0)
	    {
	      printf("Eroare: Monitorul inca ruleaza ... .\nOpriti monitorul inainte de inchierea programului.\n");
	    }
	  else
	    {
	      printf("Programul s-a inchis!\n");
	      exit(0);
	    }
	}
      else
	{
	  printf("Comanda invalida!\n");
	}
    }
  return 0;
}

