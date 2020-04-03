#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 

void closePipes(int pipes[]) {
    //close the pipe as it is no longer in use
    close(pipes[0]); //close read pipe
    close(pipes[1]); //close write pipe
}

int main(int argc, char const *argv[])
{
    int status4;

	int pipeDesc[2];  
    // Create pipe
	if (pipe(pipeDesc)==-1)  { 
        printf("SHOULD NEVER HAPPEN\n");
        perror("pipe creation has failed\n");
        exit(EXIT_FAILURE);
	} 

    // Create child
	pid_t child_pid = fork(); 
	if (child_pid < 0) { //If child creation error 
        printf("SHOULD NEVER HAPPEN\n");
	    perror("child creation failed\n");
        exit(EXIT_FAILURE);
	}  
    
    // In child
    if (child_pid == 0)  { 
        dup2(pipeDesc[1], STDOUT_FILENO); //pipe INPUT is now STDOUT

        closePipes(pipeDesc);

        char *argm1[] = {"ls", NULL};
        execv("/bin/ls", argm1);
	} else {
        dup2(pipeDesc[0], STDIN_FILENO); //pipe OUTPUT is now STDIN

        closePipes(pipeDesc);

        char *argm2[] = {"wc", "-l", NULL};
        execv("/usr/bin/wc", argm2);
    }


    return 0;
}
