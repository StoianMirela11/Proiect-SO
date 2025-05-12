#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <fcntl.h>

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

typedef struct{
  char user_name[30];
  int score;
}TREASURE_SCORE;

int main(int argc, char** argv)
{
  if(argc!=2)
    {
      perror("Eroare la argumentele programului de calculare a scorului");
      exit(EXIT_FAILURE);
    }
  //deschid fisierul pentru citirea valorilor din fisierul de treasures
  int fisier=open(argv[1], O_RDONLY);
  if(fisier==-1)
    {
      perror("Eroare la deschiderea fisierului pentru citirea valorilor");
      exit(EXIT_FAILURE);
    }
  TREASURE_SCORE scoruri[100]; //un tablou in care pastrez toti userii impreuna cu scorul total al fiecaruia
  TREASURE_DATA aux;
  int nr_scoruri=0;

  while(read(fisier, &aux, sizeof(TREASURE_DATA))>0)
    {
      int gasit =0;
      for(int i=0;i<nr_scoruri;i++)
	{
	  if(strcmp(aux.user_name, scoruri[i].user_name)==0)//inseamna ca user-ul exista deja in tabloul meu cu scoruri
	    {
	      scoruri[i].score=scoruri[i].score+aux.value;
	      gasit=1;
	      break;
	    }
	}
      if(gasit==0)
	{
	  strcpy(scoruri[nr_scoruri].user_name, aux.user_name);
	  scoruri[nr_scoruri].score=aux.value;
	  nr_scoruri++;
	}
    }

  close(fisier);
  for(int i=0;i<nr_scoruri;i++)
    {
      printf("User_name: %s, total score: %d", scoruri[i].user_name, scoruri[i].score);
      printf("\n");
    }
  return 0;
}
