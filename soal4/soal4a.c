#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> //library thread
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

void *print_message_function( void *ptr );
void *calculateMatrix( void *ptr );

// Source matrices
// Make sure it fulfils the multiplication rules of matrices
int matrix1[4][2] = {
  1, 2,
  4, 6, 
  3, 12,
  1, 7
};

int matrix2[2][5] = {
  1, 2, 4, 5, 6,
  4, 5, 4, 3, 2 
};

// size of matrice
int mat1I = 4, mat1J = 2;
int mat2I = 2, mat2J = 5;
// values to be returned by thread processes
// int retVals[mat1I][mat2J];
int retVals[2][5];
// semaphone for retMatrix
int retMatLock = 0;


int main()
{
  key_t key = 1234;
  int *retVals[4][5];
  pthread_t thread[mat1I][mat2J];//inisialisasi awal
  int iret[mat1I][mat2J];
  
  int shmid = shmget(key, sizeof(int[4][5]), IPC_CREAT | 0666);
  retVals[4][5] = (int *)shmat(shmid, NULL, 0);
 // Dispatch a single thread for every element in result matrix
  int i, j;
  for (i = 0; i < mat1I; i++) {
    for (j = 0; j < mat2J; j++) {
      int *message;
      message = malloc(sizeof(int) * 2);
      message[0] = i;
      message[1] = j;
      void *msgPtr = (void *)message;
      iret[i][j] = pthread_create( &thread[i][j], NULL, calculateMatrix, msgPtr); //membuat thread pertama
      if(iret[i][j]) //jika eror
      {
          fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][j]);
          exit(EXIT_FAILURE);
      }

      // printf("pthread_create() for thread 1 returns: %d\n", iret[i][j]);
    }
  }
  for (i = 0; i < mat1I; i++) {
    for (j = 0; j < mat2J; j++) {
      pthread_join( thread[i][j], NULL );
      printf("thread joined\n");
    }
  }

  for (i = 0; i < mat1I; i++) {
    printf("%d", retVals[i][0]);
    for (j = 1; j < mat2J; j++) {
      printf(", %d", retVals[i][j]);
    }
    printf("\n");
  shmdt(retVals);
 shmctl(shmid, IPC_RMID, NULL);
  }
  exit(EXIT_SUCCESS);
}

void *calculateMatrix( void *temp) {
  int *message;
  message = (int *)temp;

  int iMat1 = message[0];
  int jMat2 = message[1];
  free(message);
  int sum = 0;
  int i;
  for(i = 0; i < mat1J; i++) {
    printf("%d,%d multi: %d * %d = %d\n", 
    iMat1, jMat2, matrix1[iMat1][i], matrix2[i][jMat2], sum);
    sum += matrix1[iMat1][i] * matrix2[i][jMat2];
  }
  // while(retMatLock) {
  //   _sleep(1);
  // }
  // retMatLock = 1;
  retVals[iMat1][jMat2] = sum;
  // retMatLock = 0;
}

void *print_message_function( void *ptr )
{
    char *message;
    message = (char *) ptr;
    printf("%s \n", message);
  int i;
    for(i=0;i<10;i++){
        printf("%s %d \n", message, i);
    }
}
