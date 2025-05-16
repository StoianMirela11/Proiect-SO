#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


pid_t monitor_pid=0;
int necesita_monitor=0;
int monitor_pipe[2];

typedef struct
{
  float latitude;
  float longitude;
}COORDINATES;

typedef struct{
  char treasure_id[15];
  char user_name[30];
  COORDINATES gps_coordinates;
  char clue_text[10];
  int value;
}TREASURE_DATA;


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

void semnal_monitor_terminat(int sig)
{
  // Doar deblocheaza pause()
}

void listeaza_treasures(char* nume_director)
{
  struct stat st;
  if(stat(nume_director, &st)==-1)
    {
      printf("Vanatoarea cautata nu exista!\n");
      exit(EXIT_FAILURE);
    }
 
  char path[256];
  snprintf(path, sizeof(path),"%s/treasure_file.dat", nume_director);

  struct stat file;
  if(stat(path,&file)==-1)
    {
      perror("Fisierul nu exista!");
      exit(EXIT_FAILURE);
    }

  int fisier=open(path, O_RDONLY);
  if(fisier==-1)
    {
      perror("Eroare la deschidere!");
      exit(EXIT_FAILURE);
    }

  TREASURE_DATA buffer;
  
  while(read(fisier, &buffer, sizeof(buffer))>0)
    {
	  printf("Comoara:%s\n", buffer.treasure_id);
    }
  close(fisier);
}

void listeaza_hunts()
{
    struct dirent* intrare;
    DIR* director = opendir(".");
    if (director == NULL)
    {
        perror("Eroare la deschiderea directorului curent");
        exit(EXIT_FAILURE);
    }

    while ((intrare = readdir(director)) != NULL)
    {
        if (intrare->d_type == DT_DIR)
        {
            // Ignoram "." si ".."
            if (intrare->d_name[0] == '.')
	      {
                continue;
	      }
	    else
	      {
		printf("%s\n", intrare->d_name);
	      }

        }
    }
    closedir(director);

}

void start_monitor()
{
  if(pipe(monitor_pipe)<0)
    {
      perror("Eroare la crearea pipe-ului\n");
      exit(1);
    }
  monitor_pid=fork();

  if(monitor_pid < 0)
    {
    perror("Operatia nu s-a putut efectuta!");
    exit(EXIT_FAILURE);
    }
  if(monitor_pid==0)
    {
      dup2(monitor_pipe[1], STDOUT_FILENO); // se inlocuieste iesirea standard stdout
      close(monitor_pipe[0]);
      close(monitor_pipe[1]);
      execl("./treasure_manager", "./treasure_manager", NULL);//un nou proces care sa ruleze un program diferit de cel al parintelui
      perror("Eroare la crearea unui nou proces");
      exit(EXIT_FAILURE);
    }
  //codul parintelui
  close(monitor_pipe[1]);
  printf("Monitorul a fost deschid cu PID: %d\n",monitor_pid);
}

//functie care citeste liniile redirectate prin pipe
void citeste_din_monitor() {
    char buffer[256];
    int n;

    // Temporar setează pipe-ul ca non-blocking
    int flags = fcntl(monitor_pipe[0], F_GETFL);
    fcntl(monitor_pipe[0], F_SETFL, flags | O_NONBLOCK);

    usleep(100000);  // mică pauză pentru ca monitorul să apuce să scrie

    while ((n = read(monitor_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    // Restaurăm comportamentul original
    fcntl(monitor_pipe[0], F_SETFL, flags);
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

  struct sigaction sa_usr2;
  sa_usr2.sa_handler = semnal_monitor_terminat;
  sigemptyset(&sa_usr2.sa_mask);
  sa_usr2.sa_flags = 0;

  sigaction(SIGUSR2, &sa_usr2, NULL);
  
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
       else if(monitor_pid==0)
	 {
	   printf("Monitorul nu ruleaza. Operatia nu poate fi efectuata.\n");
	   continue;
	 }
      else if (strcmp(command, "list_hunts")==0)
	{
	  trimite_comanda("list_hunts", SIGUSR1);
	  pause();
	  citeste_din_monitor();
	}
      else if(strcmp(command, "list_treasures")==0)
	{
	  printf("Lista de vanatori este:\n");
	  listeaza_hunts();
	  char vanatoare[20];

	  while(1)
	    {
	      printf("Introdu numele vanatorii: ");
	      if(fgets(vanatoare, sizeof(vanatoare), stdin)==NULL)
		{
		  perror("Eroare la citirea vanatorii");
		  exit(EXIT_FAILURE);
		}
	      vanatoare[strcspn(vanatoare,"\n")]=0;

	      if(access(vanatoare, F_OK)==-1)
		{
		  printf("Vanatoarea nu exista!\nIntroduceti o alta vanatoare valida.\n");
		}
	      else
		{
		  break;
		}
	    }
	  char comanda_completa[128];
	  snprintf(comanda_completa, sizeof(comanda_completa), "list_treasures %s", vanatoare);
	  trimite_comanda(comanda_completa, SIGUSR1);
	  pause();
	  citeste_din_monitor();
	}
      else if(strcmp(command, "view_treasure")==0)
	{
	  char vanatoare[20];
	  char comoara[20];

	  printf("Lista de vanatori este:\n");
	  listeaza_hunts();

	  while(1)
	    {
	      printf("Introdu numele vanatorii: ");
	      if(fgets(vanatoare, sizeof(vanatoare), stdin)==NULL)
		{
		  perror("Eroare la citirea vanatorii");
		  exit(EXIT_FAILURE);
		}
	      vanatoare[strcspn(vanatoare,"\n")]=0;
	      if(access(vanatoare, F_OK)==-1)
		{
		  printf("Vanatoarea nu exista!\nIntroduceti o alta vanatoare valida.\n");
		}
	      else
		{
		  break;
		}
	    }

	      //deschidem fisierul de comori ca sa verificam daca comoara data de la tastatura exista pentru a afisa date
	      char path[256];
	      snprintf(path, sizeof(path), "%s/treasure_file.dat", vanatoare);
	      int fisier=open(path, O_RDONLY);
	      if(fisier==-1)
		{
		  perror("Eroare la deschiderea fisierului.\n");
		  exit(EXIT_FAILURE);
		}

	      printf("Lista de comori este:\n");
	      listeaza_treasures(vanatoare);
	      
	      while(1)
		{
		  printf("Introdu numele comorii: ");
		  if(fgets(comoara, sizeof(comoara), stdin)==NULL)
		    {
		      perror("Eroare la citirea comorii");
		      exit(EXIT_FAILURE);
		    }
		  comoara[strcspn(comoara, "\n")]=0;
		  TREASURE_DATA buffer;
		  int gasit=0;
		  while(read(fisier, &buffer, sizeof(buffer))>0)
		    {
		      if(strncmp(buffer.treasure_id, comoara, strlen(comoara)) == 0)

			{
			  gasit=1;
			  break;
			}
		    }
		  if(gasit)
		    {
		      break;
		    }
		  else
		    {
		      printf("Comoara nu exista.\nIntroduceti un alt nume de comoara.\n");
		    }
		}
	  char comanda_completa[128];
	  snprintf(comanda_completa, sizeof(comanda_completa), "view_treasure %s %s", vanatoare, comoara);
	  trimite_comanda(comanda_completa, SIGUSR2);
	  pause();
	  citeste_din_monitor();
	}
      else if(strcmp(command, "stop_monitor")==0)
	{
	  trimite_comanda(command, SIGTERM);
	  citeste_din_monitor();
	}
      else
	{
	  printf("Comanda invalida!\n");
	}
    }
  return 0;
}

