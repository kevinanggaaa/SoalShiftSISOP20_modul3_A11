#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h> //library thread
#define PORT 8080

typedef struct Sesh_s{
	int sock;
	char recvBuffer[1024];
	char sendBuffer[1000];
	char loginSes[1024];
	int health;
} Sesh_t;

void *clientStart(void *temp);
void clientEnd(Sesh_t *sesh);
void getInput(Sesh_t *sesh);


typedef struct Game_s {
	Sesh_t *seshA;
	Sesh_t *seshB;
} Game_t;

void *startListener(void *temp) {
	Game_t *game = (Game_t *)temp;
	Sesh_t *seshA = game->seshA;
	Sesh_t *seshB = game->seshB;
	send(seshA->sock, "game start", 16, 0);
	
	while(1) {
		// Read for input
		memset(seshA->recvBuffer, 0, sizeof(seshA->recvBuffer));
		read(seshA->sock, seshA->recvBuffer, 1024);
		// Stop loop if any of them have reached health <= 0
		if (seshA->health <= 0 || seshB->health <= 0) {
			break;
		}
		if (strcmp(seshA->recvBuffer, "hitting") == 0) {
			// send hit message
			send(seshA->sock, "hit", 3, 0);
			// Prepare health message to send to B
			seshB->health -= 10;
			sprintf(seshB->sendBuffer, "%d", seshB->health);
			// Send message health decreased
			send(seshB->sock, seshB->sendBuffer, strlen(seshB->sendBuffer), 0);
		}
	}
	// end session
	// Send game over message to clients
	if (seshA->health <= 0) {
		send(seshA->sock, "end lose", 8, 0);
	} else {
		send(seshA->sock, "end win", 7, 0);
	}
	
	// Listen for more input
	getInput(seshA);
}

void gameStart(Sesh_t *seshA, Sesh_t *seshB) {
	// Set health
	seshA->health = 100;
	seshB->health = 100;

	pthread_t thread;
	Game_t *game = malloc(sizeof(Game_t));
	game->seshA = seshA;
	game->seshB = seshB;
	int iret = pthread_create(&thread, NULL, startListener, (void *) game); //membuat thread pertama
	if (iret) { // jika error
		fprintf(stderr,"Error - pthread_create() return code: %d\n", iret);
		exit(EXIT_FAILURE);
	}

	game = malloc(sizeof(Game_t));
	game->seshA = seshB;
	game->seshB = seshA;
	startListener(game);
}

typedef struct WaitOpp_s{
	Sesh_t *sesh;
} WaitOpp_t;
WaitOpp_t *waitOpp;

void opponentCall(Sesh_t *sesh) {
	// Signal to waiting thread that there's a new client finding
	Sesh_t *seshA = waitOpp->sesh;
	Sesh_t *seshB = sesh;
	waitOpp = NULL;

	// Start the game
	// LET THE GAMES BEGII1I1I1I1I1IN
	gameStart(seshA, seshB);

	free(waitOpp);
}

void opponentWait(Sesh_t *sesh) {
	// Initialize WaitOpp struct
	waitOpp = malloc(sizeof(WaitOpp_t));
	waitOpp->sesh = sesh;

	// Kill current thread as it is no longer in use
	pthread_exit(NULL);
}

void findSwitch(Sesh_t *sesh) {
	if (waitOpp == NULL) {
	// If there's no client in the waiting list, 
	// put client in waiting list
		opponentWait(sesh);
	} else {
	// If There's already a client in the waiting list,
	// join the client
		opponentCall(sesh);
	}
}

void recvUserPass(Sesh_t *sesh) {
	// Recieve Username
	memset(sesh->recvBuffer, 0, sizeof(sesh->recvBuffer));
	read(sesh->sock, sesh->recvBuffer, 1024);
	strcpy(sesh->loginSes, sesh->recvBuffer);
	// Recieve Password
	memset(sesh->recvBuffer, 0, sizeof(sesh->recvBuffer));
	read(sesh->sock, sesh->recvBuffer, 1024);
	strcat(sesh->loginSes," - ");
	strcat(sesh->loginSes, sesh->recvBuffer);
}

void registerSwitch(Sesh_t *sesh) {
	// Get user password from user
	recvUserPass(sesh);
	// Write user password to akun.txt
	FILE* fp = fopen ("akun.txt","a");
	fprintf(fp, "%s\n", sesh->loginSes);
	fclose(fp);

	// Print every entry on akun.txt 
	// because the problem tells us to do it
	char line[1000];
	fp = fopen ("akun.txt","r");
	while (fgets(line, sizeof(line), fp) != NULL) {
		printf("%s", line);
	}
	fclose(fp);

	send(sesh->sock, "register success", 16, 0);

	// Listen for more input
	getInput(sesh);
}

void loginSwitch(Sesh_t *sesh) {
	// Get user password from user
	recvUserPass(sesh);

	// Check user password is in database
	FILE* fp = fopen ("akun.txt", "r");
	int flag = 0;
	char line[1000];

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, sesh->loginSes) != NULL) {
			flag = 1;
			printf("Auth success\n");
			send(sesh->sock, "login success", 13, 0);
			break;
		}
	}

	if(flag == 0){
		printf("Auth Failed\n");
		send(sesh->sock, "login failed", 12, 0);
	}

	fclose(fp);
	// Listen for more input
	getInput(sesh);
}

void logoutSwitch(Sesh_t *sesh) {
	memset(sesh->loginSes, 0, sizeof(sesh->loginSes));
	// Listen for more input
	getInput(sesh);
}

// handle for login | register | logout | find
void getInput(Sesh_t *sesh) {
	// Read request
	memset(sesh->recvBuffer, 0, sizeof(sesh->recvBuffer));
	read(sesh->sock, sesh->recvBuffer, 1024);

	if((strcmp(sesh->recvBuffer, "login") == 0)){
	// if user chooses login
		loginSwitch(sesh);
	} else if ((strcmp(sesh->recvBuffer, "register")) == 0) {
	// If user chooses register
		registerSwitch(sesh);
	} else if ((strcmp(sesh->recvBuffer, "find")) == 0) {
	// If input is not correct
		findSwitch(sesh);
	} else if ((strcmp(sesh->recvBuffer, "logout")) == 0) {
		logoutSwitch(sesh);
	}
}

void clientEnd(Sesh_t *sesh) {
	free(sesh);
}

void *clientStart(void *temp) {
	Sesh_t *sesh;
	sesh = (Sesh_t *)temp;

	// Listen input
	getInput(sesh);
}

void createSocket(int *server_fd, struct sockaddr_in * address) {
	int opt = 1;
	// create socket
	if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// If socket opt permissionss error
	if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// socket setup
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons( PORT );
	
	// bind socket to addr
	if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address))<0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// socket listen from addr
	if (listen(*server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char const *argv[]) {
	int keep_connection = 1;
	int server_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	// Recieve buffer
	char buffer[1024];

	createSocket(&server_fd, &address);
	pthread_t *thread;
	Sesh_t *sesh;
	while(keep_connection) { 
		int new_socket;
		// accept incoming connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		thread = malloc(sizeof(pthread_t));
		sesh = malloc(sizeof(Sesh_t));
		sesh->sock = new_socket;
		int iret = pthread_create(thread, NULL, clientStart, (void *) sesh); //membuat thread pertama
		if(iret) //jika eror
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", iret);
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
