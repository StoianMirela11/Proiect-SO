#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

typedef struct
{
  float latitude;
  float longitude;
}COORDINATES;

typedef struct{
  char treasure_hunt[15];
  char user_name[30];
  COORDINATES gps_coordinates;
  char clue_text[10];
  int value;
}TREASURE_DATA;

void add_treasure(char* nume_director)
{
  struct stat st;
  if(stat(nume_director, &st)==-1) //daca directorul nu exista
    {
      //daca directorul nu exista, il facem
      if(mkdir(nume_director, 0777)==-1) //citire, scriere si executie
	{
	  perror("Eroare la crearea directorului!\n");
	  exit(-1);
	}
    }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", nume_director);

  TREASURE_DATA new_treasure;

  int file=open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(file==-1)
    {
      perror("Eroare la deshiderea fisierului");
      exit(-1);
    }

  printf("Introduceti numele vanatorii: ");scanf("%14s", new_treasure.treasure_hunt);
  printf("Introduceti user_name: ");scanf("%s", new_treasure.user_name);
  printf("Introduceti coordonatele (latitudine, longitudine):"); scanf("%f %f", &new_treasure.gps_coordinates.latitude, &new_treasure.gps_coordinates.longitude);
  printf("Introduceti indiciul:");scanf("%9s", new_treasure.clue_text);
  printf("Introduceti valoarea:");scanf("%d", &new_treasure.value);

  if(write(file, &new_treasure, sizeof(new_treasure))==-1)
    {
      perror("ERoare la scrierea in fisier");
      exit(-1);
    }

  close(file);
}

void list_treasure(char* nume_director)
{
  struct stat st;

  if(stat(nume_director,&st)==-1)
    {
      perror("Directorul nu exista!");
      exit(-1);
    }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", nume_director); // calea fisierului meu

  struct stat file;

  if(stat(path, &file)==-1)
    {
      perror("Eroare! Nu se pot obtine informatii din fisier!");
      exit(-1);
    }

  printf("Numele vanatorii:%s\n", nume_director);
  printf("Dimensiunea fisierului:%ld\n", file.st_size);
  printf("Data ultimei modificari:%ld\n", file.st_mtime);
  printf("\n");

  int fisier=open(path, O_RDONLY);
  if(fisier==-1)
    {
      perror("Eroare la deschiderea pentru citire\n");
      exit(-1);
    }
  TREASURE_DATA buffer;
  
  while(read(fisier, &buffer ,sizeof(buffer))>0)
    {
      printf("Vanatoare:%s\n", buffer.treasure_hunt);
      printf("User:%s\n", buffer.user_name);
      printf("Coordonate GPS: %f, %f\n", buffer.gps_coordinates.latitude, buffer.gps_coordinates.longitude);
      printf("Indiciu: %s\n", buffer.clue_text);
      printf("Valoare: %d\n", buffer.value);
      printf("\n");
    }
  close(fisier);
  
}
//view <hunt_id> <id>: View details of a specific treasure
//id ul este o comoara anume din fisierul meu de comori

void view_treasure(char* hunt_id, char* id)
{
  struct stat st;
  if(stat(hunt_id, &st)==-1)
    {
      printf("Vanatoarea cautata nu exista!\n");
      exit(-1);
    }
  //daca exista directorul, ii gasesc calea si ma mut in el

  char path[256];
  snprintf(path, sizeof(path),"%s/treasure_file.dat", hunt_id);

  struct stat file;
  if(stat(path,&file)==-1)
    {
      perror("Fisierul nu exista!");
      exit(-1);
    }
  //deschidem fisierul
  int fisier=open(path, O_RDONLY);
  if(fisier==-1)
    {
      perror("Eroare la deschidere!");
      exit(-1);
    }

  TREASURE_DATA buffer;
  //citesc date de dimensiunea unui TRESURE_DATA
  while(read(fisier, &buffer, sizeof(buffer))>0)
    {
      if(strcmp(buffer.treasure_hunt, id)==0)
	{
	  printf("Vanatoare:%s\n", buffer.treasure_hunt);
	  printf("User:%s\n", buffer.user_name);
	  printf("Coordonate GPS: %f, %f\n", buffer.gps_coordinates.latitude, buffer.gps_coordinates.longitude);
	  printf("Indiciu: %s\n", buffer.clue_text);
	  printf("Valoare: %d\n", buffer.value);
	  printf("\n");
	}
    }
  close(fisier);
  
}
//hunt_id este denumirea folderului
//id este denumirea comorii
void remove_treasure(char* hunt_id, char* id)
{
  struct stat st;
  if(stat(hunt_id, &st)==-1)
    {
      perror("Vanatoarea nu exista!\n");
      exit(-1);
    }
  //daca vanatoarea exista, gasim calea fisierului nostru cu date
  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", hunt_id);

  struct stat file;
  if(stat(path, &file)==-1)
    {
      perror("Fisierul cu comori nu exista\n");
      exit(-1);
    }
  //daca exista, il deschidem pentru citire si scriere
  int fisier=open(path, O_RDWR);
  if(fisier==-1)
    {
      perror("Eroare la deschiderea fisierului\n");
      exit(-1);
    }

  //citesc fiecare comoara din fisier
  TREASURE_DATA buffer;
  off_t pozitie_scriere=0;
  while(read(fisier,&buffer, sizeof(buffer))>0)
    {
      if(strcmp(buffer.treasure_hunt, id)!=0)
	{
	  if(lseek(fisier, pozitie_scriere,SEEK_SET)==-1)
	    {
	      perror("Eroare la mutarea cursorului pentru scriere\n");
	      exit(-1);
	    }
	  if(write(fisier, &buffer, sizeof(buffer))==-1)
	    {
	      perror("Eroare la scrierea in fisier\n");
	      exit(-1);
	    }
	  pozitie_scriere=pozitie_scriere+sizeof(buffer);
	}
    }

  if(ftruncate(fisier, pozitie_scriere)==-1)
    {
      perror("Eroare la trunchierea fisierului\n");
      close(fisier);
      exit(-1);
    }
  close(fisier);
}

int main(int argc, char **argv)
{
  if(argc == 3)
    {
      if(strcmp(argv[1],"add")==0)
	{
	  add_treasure(argv[2]);
	}
      if(strcmp(argv[1], "list")==0)
	{
	  list_treasure(argv[2]);
	}
      if(strcmp(argv[1],"remove_hunt")==0)
	{
	  //stergem o vanatoare
	}
    }
  else if(argc == 4)
    {
      if(strcmp(argv[1],"view")==0)
	{
	  view_treasure(argv[2], argv[3]);
	}
      if(strcmp(argv[1], "remove")==0)
	{
	  remove_treasure(argv[2], argv[3]);
	}
    }
  else
    {
      perror(NULL);
      exit(-1);
    }
  return 0;
}
