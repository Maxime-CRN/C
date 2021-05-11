#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

FILE *f1, *f2;
char *f2_path;

void print_end(int status, int pid){
    if(WIFEXITED(status)){
        printf("%d exited, status=%d\n", pid, WIFSIGNALED(status));
    }
    if(WIFSIGNALED(status)){
        printf("%d killed by signal %d\n", pid, WTERMSIG(status));
    }
}

void handSIGINT(int sig){
  if(sig == SIGINT){
    if(f1 != NULL) fclose(f1);
    if(f2 != NULL) fclose(f2);
    unlink(f2_path);
    exit(0);
  }

}


int main(int argc, char **argv){
  if(argc != 3){
    fprintf(stderr, "Error\nUsage : copy file_1 file_2\n");
    exit(EXIT_FAILURE);
  }

  f1 =fopen(argv[1],"r");

  if(f1 == NULL){perror("fopen");exit(EXIT_FAILURE);}
  f2 =fopen(argv[2],"w");
  f2_path = argv[2];

  if(f2 == NULL){perror("fopen");exit(EXIT_FAILURE);}

  struct stat statf1, statf2;
  if(stat(argv[1], &statf1) == -1){perror("stat");exit(EXIT_FAILURE);}

  if(stat(argv[1], &statf2) == -1){perror("stat");exit(EXIT_FAILURE);}


  struct sigaction action;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = handSIGINT;
  sigaction(SIGINT, &action, NULL);

  int pipefd[2];
  pipe(pipefd);

  if(fork()==0){
    //lecture f1
    char buf[1];
    while(feof(f1)==0){
      fread(buf, sizeof(char), 1,f1);
      write(pipefd[1],buf,1);
    }
    exit(0);
  }

  close(pipefd[1]);

  if(fork()==0){
    char buf[1];
    while(read(pipefd[0],buf,1)>0){
      fwrite(buf, sizeof(char), 1,f2);
    }
    exit(0);
  }
  close(pipefd[0]);

  int stat, pid = wait(&stat);
  print_end(stat, pid);
  pid = wait(&stat);
  print_end(stat, pid);

  fclose(f1);
  fclose(f2);

  return 0;
}
