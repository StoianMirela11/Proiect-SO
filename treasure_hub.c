#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>

pid_t monitor_pid;

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
  char command[256];
  while(1)
    {
      printf(">>>");
      if(fgets(command, sizeof(command), stdin)==NULL)
	{
	  perror("Eroare la citirea comenzii");
	  exit(EXIT_FAILURE);
	}
      command[strcspn(command, "\n")]=0; //stergem caracterul de \n

      if(strcmp(command,"start_monitor")==0)
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
    }
  return 0;
}

