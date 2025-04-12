#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>

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
  if(stat(nume_director, &st)==-1) 
    {
      if(mkdir(nume_director, 0777)==-1)
	{
	  perror("Eroare la crearea directorului!\n");
	  exit(EXIT_FAILURE);
	}
    }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", nume_director);

  TREASURE_DATA new_treasure;

  int file=open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(file==-1)
    {
      perror("Eroare la deshiderea fisierului");
      exit(EXIT_FAILURE);
    }

  printf("Introduceti numele vanatorii: ");scanf("%14s", new_treasure.treasure_hunt);
  printf("Introduceti user_name: ");scanf("%s", new_treasure.user_name);
  printf("Introduceti coordonatele (latitudine, longitudine):"); scanf("%f %f", &new_treasure.gps_coordinates.latitude, &new_treasure.gps_coordinates.longitude);
  printf("Introduceti indiciul:");scanf("%9s", new_treasure.clue_text);
  printf("Introduceti valoarea:");scanf("%d", &new_treasure.value);

  if(write(file, &new_treasure, sizeof(new_treasure))==-1)
    {
      perror("Eroare la scrierea in fisier");
      exit(EXIT_FAILURE);
    }

  close(file);
}

void list_treasure(char* nume_director)
{
  struct stat st;

  if(stat(nume_director,&st)==-1)
    {
      perror("Directorul nu exista!");
      exit(EXIT_FAILURE);
    }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", nume_director);

  struct stat file;

  if(stat(path, &file)==-1)
    {
      perror("Eroare! Nu se pot obtine informatii din fisier!");
      exit(EXIT_FAILURE);
    }

  printf("Numele vanatorii:%s\n", nume_director);
  printf("Dimensiunea fisierului:%ld\n", file.st_size);
  printf("Data ultimei modificari:%ld\n", file.st_mtime);
  printf("\n");

  int fisier=open(path, O_RDONLY);
  if(fisier==-1)
    {
      perror("Eroare la deschiderea pentru citire\n");
      exit(EXIT_FAILURE);
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

void view_treasure(char* hunt_id, char* id)
{
  struct stat st;
  if(stat(hunt_id, &st)==-1)
    {
      printf("Vanatoarea cautata nu exista!\n");
      exit(EXIT_FAILURE);
    }
 
  char path[256];
  snprintf(path, sizeof(path),"%s/treasure_file.dat", hunt_id);

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

void remove_treasure(char* hunt_id, char* id)
{
  struct stat st;
  if(stat(hunt_id, &st)==-1)
  {
    perror("Vanatoarea nu exista!");
    exit(EXIT_FAILURE);
  }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", hunt_id);

  struct stat file;
  if(stat(path, &file)==-1)
  {
    perror("Fisierul cu comori nu exista");
    exit(EXIT_FAILURE);
  }

  int fisier = open(path, O_RDWR);
  if(fisier == -1)
  {
    perror("Eroare la deschiderea fisierului");
    exit(EXIT_FAILURE);
  }

  char new_path[256];
  snprintf(new_path, sizeof(new_path), "%s/new_treasure_file.dat", hunt_id);
  int new_fisier = open(new_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(new_fisier == -1)
  {
    perror("Eroare la deschiderea pentru scrierea in noul fisier");
    exit(EXIT_FAILURE);
  }

  TREASURE_DATA buffer;
  while(read(fisier, &buffer, sizeof(buffer)) > 0)
  {
    if(strcmp(buffer.treasure_hunt, id) != 0) 
    {
      if(write(new_fisier, &buffer, sizeof(buffer)) == -1)
      {
        perror("Eroare la scriere in fisierul nou");
        exit(EXIT_FAILURE);
      }
    }
  }

  close(fisier);
  close(new_fisier);

  if(remove(path) == -1)
  {
    perror("Eroare la stergerea fisierului vechi");
    exit(EXIT_FAILURE);
  }

  if(rename(new_path, path) == -1)
  {
    perror("Eroare la redenumirea fisierului nou\n");
    exit(EXIT_FAILURE);
  }
}

void log_file(char* hunt_id, char* operatie)
{
  struct stat st;
  if(stat(hunt_id, &st) == -1)
  {
    perror("Nu exista directorul!");
    exit(EXIT_FAILURE);
  }

  char path[256];
  snprintf(path, sizeof(path), "%s/logged_hunt.txt", hunt_id);

  int fisier = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fisier == -1)
  {
    perror("Eroare la deschidere");
    exit(EXIT_FAILURE);
  }

  char buffer[256];
  char* time_str = strtok(ctime(&st.st_ctime), "\n"); 
  snprintf(buffer, sizeof(buffer), "[%s] S-a efectuat optiunea: %s\n", time_str, operatie);

  if(write(fisier, buffer, strlen(buffer)) == -1)
  {
    perror("Eroare la scrierea in fisier!");
    exit(EXIT_FAILURE);
  }

  close(fisier);

  char link_path[256];
  snprintf(link_path, sizeof(link_path), "logged_hunt-%s", hunt_id);
  if(access(link_path, F_OK) == -1)
  {
    if(symlink(path, link_path) == -1)
    {
      perror("Eroare la creare symlink");
      exit(EXIT_FAILURE);
    }
  }
}

void remove_hunt(char* hunt_id)
{
  struct stat st;
  if(stat(hunt_id,&st)==-1)
    {
      perror("Vanatoarea nu exista!");
      exit(EXIT_FAILURE);
    }

  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", hunt_id);

  if(remove(path)==-1)
    {
      perror("Eroare la stergerea fisierului de comori!");
      exit(EXIT_FAILURE);
    }

  snprintf(path, sizeof(path), "%s/logged_hunt.txt", hunt_id);

  if(remove(path)==-1)
    {
      perror("Eroare la stergerea fisierului cu logg!");
      exit(EXIT_FAILURE);
    }

  snprintf(path, sizeof(path), "logged_hunt-%s", hunt_id );
  if(remove(path)==-1)
    {
      perror("Eroare la stergerea legaturii simbolice!");
      exit(EXIT_FAILURE);
    }

  if(rmdir(hunt_id)==-1)
    {
      perror("Eroare la stergerea vanatorii!");
      exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
  if(argc == 3)
    {
      if(strcmp(argv[1],"add")==0)
	{
	  add_treasure(argv[2]);
	  log_file(argv[2], argv[1]);
	}
      if(strcmp(argv[1], "list")==0)
	{
	  list_treasure(argv[2]);
	  log_file(argv[2], argv[1]);
	}
      if(strcmp(argv[1],"remove_hunt")==0)
	{
	  remove_hunt(argv[2]);
	}
    }
  else if(argc == 4)
    {
      if(strcmp(argv[1],"view")==0)
	{
	  view_treasure(argv[2], argv[3]);
	  log_file(argv[2], argv[1]);
	}
      if(strcmp(argv[1], "remove")==0)
	{
	  remove_treasure(argv[2], argv[3]);
	  log_file(argv[2], argv[1]);
	}
    }
  else
    {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
return 0;
}
