#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <time.h>


void detail(struct stat stats){
  //droits user
  if (stats.st_mode & S_IRUSR) printf("r");
  else printf("-");
  if (stats.st_mode & S_IWUSR) printf("w");
  else printf("-");
  if (stats.st_mode & S_IXUSR) printf("x");
  else printf("-");

  //droits groupe
  //droits user
  if (stats.st_mode & S_IRGRP) printf("r");
  else printf("-");
  if (stats.st_mode & S_IWGRP) printf("w");
  else printf("-");
  if (stats.st_mode & S_IXGRP) printf("x");
  else printf("-");

  //droits autres
  //droits user
  if (stats.st_mode & S_IROTH) printf("r");
  else printf("-");
  if (stats.st_mode & S_IWOTH) printf("w");
  else printf("-");
  if (stats.st_mode & S_IXOTH) printf("x");
  else printf("-");

  printf(" ");

  //nombre de liens
  printf("%lu ", stats.st_nlink);

  //utilisateur
  struct passwd *pw = getpwuid(stats.st_uid);
  printf("%s\t", pw->pw_name);

  //groupe utilisateur
  struct passwd *gr = getpwuid(stats.st_gid);
  printf("%s\t", gr->pw_name);

  //size
  printf("%ld\t", stats.st_size);

  //date
  char *date = ctime(&stats.st_mtime);
  date[strlen(date) - 1] = '\0';
  printf("%s ", date);
}

int main(int argc, char **argv){
  DIR *rep = opendir("./");

 if (argc != 1 && (argc > 2 || strcmp(argv[1],"-l") != 0)){
    fprintf(stderr,"Error argument must be \"-l\" or nothing.\n");
    return 1;
  }

  if (rep == NULL){
    fprintf(stderr,"Error current directory can't be opened");
    perror("opendir");
    return 1;
  }

  struct dirent* file = NULL;
  struct stat stats;


  while((file = readdir(rep)) != NULL){

    char *fileName = file->d_name;

    if(fileName[0] != '.'){

      if(argc == 2){
        if (stat(fileName, &stats) == 0){
          printf("%c ", file->d_type);
          detail(stats);
        }

      }

      printf("%s\n", fileName);
    }
  }


  closedir(rep);
  return 0;
}
