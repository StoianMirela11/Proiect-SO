#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

pid_t monitor_pid;

void sigchld_handler(int sig)
{
  int status;
  wait(&status);
  if(WIFEXITED(status))
    {
      printf("Monitorul s-a terminat cu codul %d\n", WEXITSTATUS(status));
    }
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

int main()
{
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGCHLD, &sa, NULL);
  
  char command[256];
  while(1)
    {
      printf("Introduceti comanda: \n");
      if(fgets(command, sizeof(command), stdin)==NULL)
	{
	  if (errno == EINTR)
	    {
	      return 0;
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
	  comenzi("list_hunts");
	  kill(monitor_pid, SIGUSR1);
	}
      else if(strcmp(command, "stop_monitor")==0)
	{
	  comenzi("stop_monitor");
	  kill(monitor_pid,SIGTERM);
	}
    }
  return 0;
}

