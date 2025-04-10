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

  // găsim calea fișierului cu datele
  char path[256];
  snprintf(path, sizeof(path), "%s/treasure_file.dat", hunt_id);

  struct stat file;
  if(stat(path, &file)==-1)
  {
    perror("Fisierul cu comori nu exista\n");
    exit(-1);
  }

  // deschidem fișierul vechi
  int fisier = open(path, O_RDWR);
  if(fisier == -1)
  {
    perror("Eroare la deschiderea fisierului\n");
    exit(-1);
  }

  // creăm fișierul nou
  char new_path[256];
  snprintf(new_path, sizeof(new_path), "%s/new_treasure_file.dat", hunt_id);
  int new_fisier = open(new_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(new_fisier == -1)
  {
    perror("Eroare la deschiderea pentru scrierea in noul fisier\n");
    exit(-1);
  }

  TREASURE_DATA buffer;
  while(read(fisier, &buffer, sizeof(buffer)) > 0)
  {
    if(strcmp(buffer.treasure_hunt, id) != 0) // dacă nu găsim comoara, o scriem în fișierul nou
    {
      if(write(new_fisier, &buffer, sizeof(buffer)) == -1)
      {
        perror("Eroare la scriere in fisierul nou\n");
        exit(-1);
      }
    }
  }

  close(fisier);
  close(new_fisier);

  // ștergem fișierul vechi
  if(remove(path) == -1)
  {
    perror("Eroare la stergerea fisierului vechi\n");
    exit(-1);
  }

  // redenumim fișierul nou
  if(rename(new_path, path) == -1)
  {
    perror("Eroare la redenumirea fisierului nou\n");
    exit(-1);
  }
}

//creare fisier pentru logg


void log_file(char* hunt_id, char* operatie)
{
  struct stat st;
  if(stat(hunt_id, &st) == -1)
  {
    perror("Nu exista directorul!\n");
    exit(-1);
  }

  char path[256];
  snprintf(path, sizeof(path), "%s/logged_hunt.txt", hunt_id);

  // Deschidem fișierul de log pentru adăugare
  int fisier = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fisier == -1)
  {
    perror("Eroare la deschidere\n");
    exit(-1);
  }

  char buffer[256];
  char* time_str = strtok(ctime(&st.st_ctime), "\n"); // Eliminăm caracterul de newline
  snprintf(buffer, sizeof(buffer), "[%s] S-a efectuat optiunea: %s\n", time_str, operatie);

  if(write(fisier, buffer, strlen(buffer)) == -1)
  {
    perror("Eroare la scrierea in fisier!\n");
    exit(-1);
  }

  close(fisier);

  // Realizăm legătura simbolică
  char link_path[256];
  snprintf(link_path, sizeof(link_path), "logged_hunt-%s", hunt_id);
  if(access(link_path, F_OK) == -1)  // Verificăm dacă legătura simbolică există
  {
    if(symlink(path, link_path) == -1)
    {
      perror("Eroare la creare symlink\n");
    }
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
	  //stergem o vanatoare
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
      exit(-1);
    }
  return 0;
}
