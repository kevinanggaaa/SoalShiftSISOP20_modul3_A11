#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<pthread.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<sys/ipc.h>

int row = 4;
int column = 5;
key_t shmKey = 1234;

void closePipes(int pipes[]) {
    //close the pipe as it is no longer in use
    close(pipes[0]); //close read pipe
    close(pipes[1]); //close write pipe
}

void sharedMemoryInit() {

}

void runProgramOne() {
    // prepare pipe
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
        // Link stdout to pipe input
        dup2(pipeDesc[1], STDOUT_FILENO); //pipe INPUT is now STDOUT

        closePipes(pipeDesc);

        // Run program 1
        char *argv[] = {"4a",NULL};
        execv("./4a.exe", argv);
    } else {
    // In parent
        wait(NULL);
        // get shared memory
        int shmid = shmget(shmKey, sizeof(int)*20, IPC_CREAT | 0666);
        int *buffer = (int *)shmat(shmid, NULL, 0);

        // Make pipe output go to STDIN
        dup2(pipeDesc[0], STDIN_FILENO); //pipe OUTPUT is now STDOUT
        // Get input from last program
        int in;
        for (int i = 0; i < 20; i++) {
            scanf("%d", &in);
            // printf("in: %d\n", in);
            buffer[i] = in;
            // printf("buffer: %d\n", buffer[i]);
        }
        // Detatch from shared memory
        shmdt(buffer); 
    }

}

void *calcSum(void *temp) {
    int *msg = (int *)temp;
    int num = msg[0];
    long res = num*(num + 1)/2;
    return (void *)res;
}

int main(int argc, char const *argv[])
{
    // Make child
    // Create child
	pid_t child_pid = fork(); 
	if (child_pid < 0) { //If child creation error 
        printf("SHOULD NEVER HAPPEN\n");
	    perror("child creation failed\n");
        exit(EXIT_FAILURE);
	}  
    
    // In child
    if (child_pid == 0)  { 
        // Run program 4a.exe
        runProgramOne();
        exit(EXIT_SUCCESS);
    } else {
        // Wait for program one to finish
        wait(NULL);

        // Get from output of program one on shared memory
        int shmid = shmget(shmKey, sizeof(int)*20, IPC_CREAT | 0666);
        int (*buffer)[5];
        buffer = (int (*)[5])shmat(shmid, NULL, 0);
        // Calculate using threads
        pthread_t threads[4][5];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                int *msg = malloc(sizeof(int));
                *msg = buffer[i][j];
                int iret = pthread_create(&threads[i][j], NULL, calcSum, (void *) msg);
                if (iret) {
                    perror("failed to create thread");
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                int retVal;
                pthread_join(threads[i][j], (void **)&retVal);
                buffer[i][j] = retVal;
            }
        }
        
        // Get input from last program
        int in;
        for (int i = 0; i < 4; i++) {
            printf("%d", buffer[i][0]);
            for (int j = 1; j < 5; j++) {
                printf(" %d", buffer[i][j]);
            }
            printf("\n");
        }
        shmdt(buffer);
        shmctl(shmid,IPC_RMID,NULL); 
    }
    return 0;
}
