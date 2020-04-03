#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h> //library thread

#define PORT 8080

void screenOneStart();
void screenOneInputSwitch();
void screenTwoStart();
void screenTwoInputSwitch();
void getChoices();

static struct termios old, new;

char sendBuffer[1000], recvBuffer[1024];
char chosenString[1000];
int sock = 0;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

void *listenSpacebars(void *arg) {
	while (1) {
		if (getchar() == 32) {
			send(sock, "hitting", 7, 0);
		}
	}
}

void *printWaitingPlayer(void *arg) {
	// TODO print every second
	while (1) {
		printf("Waiting for player ...\n");
		sleep(1);
	}
}

void gameInit() {
	pthread_t printThread;
	int iretPrint = pthread_create(&printThread, NULL, printWaitingPlayer, NULL);
	if (iretPrint) { // jika error
		fprintf(stderr, "Error - pthread_create()\n");
		exit(EXIT_FAILURE);
	}

	// Wait till recv "game start" message
	memset(recvBuffer, 0, sizeof(recvBuffer));
	while (strcmp(recvBuffer, "game start") != 0) {
		read(sock, recvBuffer, 1024);
	}

	// Recieved game start message
	pthread_cancel(printThread); //stop printing waiting for player
	initTermios(1);
	printf("Game dimulai silahkan tap tap secepat mungkin !\n");

	pthread_t spacebarThread;
	// Handle spacebar inputs
	int iret = pthread_create(&spacebarThread, NULL, listenSpacebars, NULL); //membuat thread pertama
	if (iret) { // jika error
		fprintf(stderr,"Error - pthread_create() return code: %d\n", iret);
		exit(EXIT_FAILURE);
	}

	// Handle recieving messages from server
	while (1) {
		memset(recvBuffer, 0, sizeof(recvBuffer));
		read(sock, recvBuffer, 1024);
		// Stop recieving spacebars
		if (strcmp(recvBuffer, "hit") == 0) {
		// If it's the player getting hit
			printf("hit !!\n");
		} else if (strcmp(recvBuffer, "end win") == 0) {
		// If the game has ended and client won
			printf("Game berakhir kamu menang\n");
			// end game
			break;
		} else if (strcmp(recvBuffer, "end lose") == 0) {
		// If the game has ended and client lost
			printf("Game berakhir kamu kalah\n");
			// end game
			break;
		} else {
		// Else it's the amount of health the player have left
			printf("%s\n", recvBuffer);
		}
	}
	// Game has ended
	resetTermios();
	pthread_cancel(spacebarThread);
	screenTwoStart();
}

void findMatchSwitch() {
	send(sock, "find", 4, 0);
	gameInit();
}

void sendUserPass() {
	// Get username of user
	printf("    Username : ");
	scanf("%s", sendBuffer);
	// Send username to server
	send(sock, sendBuffer, strlen(sendBuffer), 0);
	memset(sendBuffer, 0, sizeof(sendBuffer));

	// Get password of user
	printf("    Password : ");
	scanf("%s", sendBuffer);
	// Send password to server
	send(sock, sendBuffer, strlen(sendBuffer), 0);
	memset(sendBuffer, 0, sizeof(sendBuffer));
}

void loginSwitch() {
	send(sock, "login", 5, 0);
	// Send username and password to server
	sendUserPass();

	// Read server response (login success|login failed)
	memset(recvBuffer, 0, sizeof(recvBuffer));
	read(sock, recvBuffer, 1024);
	if (strcmp(recvBuffer, "login success") == 0) {
	// Login success
		printf("login success\n");
		screenTwoStart();
	} else {
	// Login unsuccess
		printf("login failed\n");
		screenOneStart();
	}
}

void registerSwitch() {
	send(sock, "register", 8, 0);
	// Send username and password to server
	sendUserPass();

	// Read server response (register success|register failed)
	memset(recvBuffer, 0, sizeof(recvBuffer));
	read(sock, recvBuffer, 1024);
	if (strcmp(recvBuffer, "register success") == 0) {
	// register success
		printf("register success\n");
		screenOneStart();
	} else {
	// register unsuccess
		printf("register failed\n");
		printf("THIS IS NOT SUPPOSED EVER HAPPEN\n");
		screenOneStart();
	}
}

void screenTwoInputSwitch() {
	if (strcmp(chosenString, "find") == 0) {
	// If chosen find match
		// Go to finding match
		findMatchSwitch();
	} else if (strcmp(chosenString, "logout") == 0) {
	// if choose to logout
		// Go to first screen
		send(sock, "logout", 6, 0);
		screenOneStart();
	} else {
		printf("Input unrecognized\n");
		getChoices();
		screenTwoInputSwitch();
	}
}

void screenTwoStart() {
	printf("1.  Find match\n");
	printf("2.  Logout\n");
	getChoices();
	screenTwoInputSwitch();
}

void screenOneInputSwitch() {
	if (strcmp(chosenString, "login") == 0) {
		// Show login screen
		loginSwitch();
	} else if (strcmp(chosenString, "register") == 0) {
		// Show register screen
		registerSwitch();
		// Go back to main screen
		screenOneStart();
	} else {
		printf("Input unrecognized\n");
		getChoices();
		screenOneInputSwitch();
	}
}

void screenOneStart() {
	printf("1.  Login\n");
	printf("2.  Register\n");
	getChoices();
	screenOneInputSwitch();
}

void getChoices() {
	printf("    Choices : ");
	memset(chosenString, 0, sizeof(chosenString));
	scanf("%s", chosenString);
}

int main(int argc, char const *argv[]) {
	int keep_connection = 1;
	struct sockaddr_in address;
	struct sockaddr_in serv_addr;

	// create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}
  
	memset(&serv_addr, '0', sizeof(serv_addr));
  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// addr
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	// connect to server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}

	while(keep_connection) { 
		screenOneStart();
	}
	return 0;
}
