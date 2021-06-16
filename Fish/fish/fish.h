#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define INPUT_REDIRECT 0
#define OUTPUT_REDIRECT 1

//Number of subprocess not completed
int nb_bg_subprocess =0;

//Exercise 7
//Use SIGCHLD to know when sub processe finish and print it satus
void handSIG_CHILD(int signal){
	int stat;
	//Wait end of sub process
	int pid = wait(&stat);

	//decrements by 1 the non-terminated sub-process because this one has just ended
	--nb_bg_subprocess;

	//Print pid of sub process and informations about it execution
	if(WIFEXITED(stat)){
			fprintf(stderr,"\tBG : %d exited, status=%d\n", pid, WIFSIGNALED(stat));
	}
	if(WIFSIGNALED(stat)){
			fprintf(stderr,"\tBG : %d killed by signal %d\n", pid, WTERMSIG(stat));
	}
}

//Handle input and output redirection. Return the new file descriptor if succeeded, -1 if not
int cmd_redirection(const char *file, int type)
{
	//NULL file
	if(!file)
	{
		fprintf(stderr, "Error : Cannot redirect to a NULL file.\n");
		return -1 ;
	}

	printf("Redirecting %s to file '%s'.\n", (type == INPUT_REDIRECT) ? "input" : "output", file);

	//Wrong type
	if(type != INPUT_REDIRECT && type != OUTPUT_REDIRECT)
	{
		fprintf(stderr, "Error : wrong redirection type in source code.\n");
		return -1 ;
	}

	//opening file
	int flags = (type == INPUT_REDIRECT) ? O_RDONLY : (O_WRONLY | O_CREAT);
	int fd = open(file, flags);
	if(fd < 0)
	{
		fprintf(stderr, "Unable to open file '%s'\n",file);
		return -1;
	}

	//Redirecting
	int dup_fd = (type == INPUT_REDIRECT) ? STDIN_FILENO : STDOUT_FILENO;
	if(dup2(fd, dup_fd) < 0)
	{
		fprintf(stderr, "Unable to redirect %s\n", (type == INPUT_REDIRECT) ? "input" : "output");
		return -1;
	}

	close(fd);

	return 0;
}

//Exercise 7
//Execute background commands
void backgroundCommand(struct line *li, int numCommand){
	//Exercise 7
  //redirect signal for end of child to print status
  struct sigaction child;
  child.sa_flags = 0;
  sigemptyset(&child.sa_mask);
	child.sa_handler = handSIG_CHILD;
	sigaction(SIGCHLD, &child, NULL);

	int verif_fork;

	if ((verif_fork=fork()) == 0){
		int res;
		//If redirection for input
		if(li->redirect_input){
			res = cmd_redirection(li->file_input, 0);
			//res == -1 redirection failed
			if(res == -1) exit(EXIT_FAILURE);
		}

		//If redirection for output
		if(li->redirect_output){
			res = cmd_redirection(li->file_output, 1);
			//res == -1 redirection failed
			if(res == -1) exit(EXIT_FAILURE);
		}

		res = execvp(li->cmds[numCommand].args[0],li->cmds[numCommand].args);

		//If command badly executed, print error of command
		if(res == -1){
			perror(li->cmds[numCommand].args[0]);
		}
		exit(EXIT_SUCCESS);
	}

	//Check if sub-process was greatly created
	if(verif_fork !=-1){
		//Increment by 1 the unfinished sub-process because we have just added 1
		++nb_bg_subprocess;
	}

	return;
}


//Exiercise 3
//Execute foreground commands
void foregroundCommand(struct line *li){
	//reset signal of end of child to not wait background command in case of foreground command
	struct sigaction child;
	child.sa_flags = 0;
	sigemptyset(&child.sa_mask);
	child.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &child, NULL);

	int fg_pid;
	int stat;

	//Execute command in sub process
	if((fg_pid = fork())==0){
		/*Exercise 6
		Reset SIGINT to it default value just
		for execution of command*/
		struct sigaction dflt;
		dflt.sa_flags = 0;
		sigemptyset(&dflt.sa_mask);
		dflt.sa_handler = SIG_DFL;
		sigaction(SIGINT, &dflt, NULL);

		int res;
		//If redirection for input
		if(li->redirect_input){
			res = cmd_redirection(li->file_input, 0);
			//res == -1 redirection failed
			if(res == -1) exit(EXIT_FAILURE);
		}

		//If redirection for output
		if(li->redirect_output){
			res = cmd_redirection(li->file_output, 1);
			//res == -1 redirection failed
			if(res == -1) exit(EXIT_FAILURE);
		}

		res = execvp(li->cmds[0].args[0],li->cmds[0].args);

		//If command badly executed, print error of command
		if(res == -1){
			perror(li->cmds[0].args[0]);
		}
		exit(EXIT_SUCCESS);
	}
	//Wait end of sub process
	waitpid(fg_pid, &stat, 0);

	//Print pid of sub process and informations about it execution
	if(WIFEXITED(stat)){
		printf("\tFG : %d exited, status=%d\n", fg_pid, WIFSIGNALED(stat));
	}
	if(WIFSIGNALED(stat)){
		printf("\tFG : %d killed by signal %d\n", fg_pid, WTERMSIG(stat));
	}
	return;
}


//Handle internal commands like cd or exit. Return 1 if an internal command other than exit has been executed, -1 if more than 1 commands, 0 if no internal commands.
void cmd_interne(struct line *li)
{
	//exit command
	//Check if it is an internal command (using strcmp)
	if(strcmp(li->cmds[0].args[0],"exit") == 0)
	{
		printf("exiting...\n");
		//Wait for subprocess to end before exiting
		while(nb_bg_subprocess >0){};
		line_reset(li);
		exit(EXIT_SUCCESS);
	}

	//cd command
	//Check if it is an internal command (using strcmp)
	if(strcmp(li->cmds[0].args[0],"cd") == 0)
	{
		//start with home directory
		size_t len_dir = strlen("home") + strlen(getenv("USER")) + 1;

		char dir[len_dir];
		for (size_t i = 0; i < len_dir; ++i) {
		dir[i] = '\0';
	}
	//Check arguments number
		if (li->cmds[0].n_args < 2) {
	  	strcpy(dir, "~");
		} else {
			strcpy(dir, li->cmds[0].args[1]);
		}
		//Check if ~ is used (using strcmp)
	if (strcmp(dir, "~") == 0) {
			char *user = getenv("USER");
			strcpy(dir, "/home/");
			strcat(dir, user);
		}
		//Error if the change of the directory failed
		if (chdir(dir) == -1) {
				perror("chdir");
				fprintf(stderr, "failed to change directory to %s\n", dir);
				line_reset(li);
		}
	}
}

	//Handle a command line with pipes
	int handle_with_pipes(struct line *li){
		int pipes[li->n_cmds-1][2];
		int status;
		int res;
		for(int i=0;i<li->n_cmds;i++){
			res=pipe(pipes[i]);
		}
		pid_t pid=fork();

		if(pid==0){
			//If redirection for input
			if (li->redirect_input) {
				cmd_redirection(li->file_input, 0);
				//res == -1 redirection failed
				if(res == -1) exit(EXIT_FAILURE);
			}
			//If redirection for output
			else{
				if(li->background){
					cmd_redirection("/dev/null", 1);
					//res == -1 redirection failed
					if(res == -1) exit(EXIT_FAILURE);
				}
			}
			//Output of process to input of first pipe
			res=dup2(pipes[0][1],1);
			//closing pipe in child
			res=close(pipes[0][1]);
			res=close(pipes[0][0]);
			res = execvp(li->cmds[0].args[0], li->cmds[0].args);
			exit(1);
		}

		//closing pipe in child
		res=close(pipes[0][1]);

		for(int i=1;i<li->n_cmds-1;i++){
			pid=fork();
			if(pid==0){
				//Output of previous pipe to input of process
				res=dup2(pipes[i-1][0],0);
				res=close(pipes[i-1][0]);
				//Output of process to input of next pipe
				res=dup2(pipes[i][1],1);
				res=close(pipes[i][1]);
				res=execvp(li->cmds[i].args[0], li->cmds[i].args);
				exit(1);
			}
			//closing pipes in father
			res=close(pipes[i-1][0]);
			res=close(pipes[i][1]);

		}

		pid=fork();
		if(pid==0){
			res=dup2(pipes[li->n_cmds-2][0],0);
			res=close(pipes[li->n_cmds-2][0]);
			res = execvp(li->cmds[li->n_cmds-1].args[0], li->cmds[li->n_cmds-1].args);
			exit(1);
		}

		//waiting for all commands
		res=close(pipes[li->n_cmds-2][0]);
		for(int i=0;i<li->n_cmds;i++){
			res=wait(&status);
		}
		return status;
}


//Execute basic commands
void exeCommand(struct line *li){

	//If command have'nt any arguments
	if(li->cmds->n_args <1){
		fprintf(stderr,"Error : please enter a command\n");
		fprintf(stderr,"Usage : [command] [options]\n");
	}

	//execute internal command if command start by "exit" or "cd"
	else if(strcmp(li->cmds[0].args[0],"exit") == 0 || strcmp(li->cmds[0].args[0],"cd") == 0){
		cmd_interne(li);
	}

	//execute command with pipes
	else if(li->n_cmds > 1){
		int stat = handle_with_pipes(li);

		//Print pid of sub process and informations about it execution
	if(WIFEXITED(stat)){
			fprintf(stderr,"\tBG : command exited, status=%d\n", WIFSIGNALED(stat));
	}
	if(WIFSIGNALED(stat)){
			fprintf(stderr,"\tBG : command killed by signal %d\n", WTERMSIG(stat));
	}
	}

	//execute background command
	else if(li->background){
		backgroundCommand(li, 0);
	}

	//execute foreground command
	else{
		foregroundCommand(li);
	}

	return;
}
