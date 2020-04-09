#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct RandPokemon_s{
	int isLocked;
	char name[20];
	int type;
	int capRateA;
	int capRateB;
	int escapeRateA;
	int escapeRateB;
	int price;
}RandPokemon_t;

typedef struct Shop_s{
	int isLocked;
	int lullabyPowder;
	int lullabyPowderPrice;
	int pokeball;
	int pokeballPrice;
	int berry;
	int berryPrice;
}Shop_t;

typedef struct State_s{
	RandPokemon_t * randPokemon;
	Shop_t * sharedShop;
}State_t;

void * randomizePokemonThread(void * param);
void * shopThread(void * param);

int main(int argc, const char * argv[]){
	srand(time(0));

	State_t * state = malloc(sizeof(State_t));
	memset(state, 0, sizeof(State_t));

	//shared memory
	key_t key = 1111;
	RandPokemon_t * randPokemon;
	int shmid =  shmget(key, sizeof(RandPokemon_t), IPC_CREAT | 0666);
	randPokemon = shmat(shmid, NULL, 0);
	memset(randPokemon, 0, sizeof(randPokemon));

	state->randPokemon = randPokemon;

	key_t key2 = 2222;
	Shop_t * sharedShop;
	int shmid2 =  shmget(key2, sizeof(Shop_t), IPC_CREAT | 0666);
	sharedShop = shmat(shmid2, NULL, 0);
	memset(sharedShop, 0, sizeof(sharedShop));

	state->sharedShop = sharedShop;
	
	pthread_t pokemonThread;

	if(pthread_create(&pokemonThread, NULL, &randomizePokemonThread, (void*)state ) < 0){
		printf("can't create thread\n");
		exit(EXIT_FAILURE);
	}

	pthread_t shop;

	if(pthread_create(&shop, NULL, &shopThread, (void*)state ) < 0){
		printf("can't create thread\n");
		exit(EXIT_FAILURE);
	}
	printf("--POKEZONE--\n");
	printf("1. Exit\n");
	printf("Choice : ");
	int choice;
	scanf("%d", &choice);
	pid_t ppp = getpid();
	if(choice == 1){
		char * argv[] = {"pkill", "traizone.exe", NULL};
		execv("/usr/bin/pkill", argv);
	}


	pthread_join(pokemonThread, NULL);
	pthread_join(shop, NULL);

	shmdt(randPokemon);
    shmctl(shmid, IPC_RMID, NULL);

	shmdt(sharedShop);
    shmctl(shmid2, IPC_RMID, NULL);
	return 0;
}

RandPokemon_t *genPoke(char *name, int type, int capA, int capB, int escA, int escB, int price) {
	RandPokemon_t *retPoke = malloc(sizeof(RandPokemon_t));
	strcpy(retPoke->name, name);
	retPoke->type = type;
	retPoke->capRateA = capA;
	retPoke->capRateB = capB;
	retPoke->escapeRateA = escA;
	retPoke->escapeRateB = escB;
	retPoke->price = price;
	return retPoke;
}

void * randomizePokemonThread(void * param){
	State_t * state = (State_t *)param;
	RandPokemon_t * randPokemon = state->randPokemon;
	char normalName[5][20] = { "Bulbasaur", "Charmander", "Squirtle", "Rattata", "Caterpie" };
	char rareName[5][20] = { "Pikachu", "Eevee", "Jigglypuff", "Snorlax", "Dragonite" };
	char legendaryName[5][20] = { "Mew", "Mewtwo", "Moltres", "Zapdos", "Articuno" };

	while(1){
		while (randPokemon->isLocked) {
			sleep(1);
		}
		
		int random = rand()%20;
		// Clear pokemon pointer
		RandPokemon_t *newPoke;
		// Start random
		if(random <= 15){
		//normal
			random = rand()%5;
			newPoke = genPoke(normalName[random], 0, 7, 10, 1, 20, 80);
		}else if(random <= 18){
		//rare
			random = rand()%5;
			newPoke = genPoke(rareName[random], 1, 5, 10, 1, 10, 100);
		}else{
		//legendary
			random = rand()%5;
			newPoke = genPoke(legendaryName[random], 2, 3, 10, 1, 5, 200);
		}

		random = rand()%8000;
		if(random == 0){
		// Shiny
			newPoke->capRateA -= 2;
			newPoke->escapeRateA += 1;
			newPoke->price += 5000;
		}
		// Switch randPokemon to new data
		memcpy(randPokemon, newPoke, sizeof(RandPokemon_t));
		free(newPoke);

		sleep(1);
	}
}

void * shopThread(void * param){
	State_t * state = (State_t *)param;
	state->sharedShop->isLocked = 1;
	state->sharedShop->lullabyPowder = 100;
	state->sharedShop->pokeball = 100;
	state->sharedShop->berry = 100;
	state->sharedShop->lullabyPowderPrice = 60;
	state->sharedShop->pokeballPrice = 15;
	state->sharedShop->berryPrice = 5;
	state->sharedShop->isLocked = 0;
	fflush(stdout);
	while(1){
		sleep(10);
		while(state->sharedShop->isLocked) usleep(10000);
		state->sharedShop->isLocked = 1;

		state->sharedShop->berry += 10;
		if(state->sharedShop->berry > 200) {
			state->sharedShop->berry = 200;
		}
		state->sharedShop->pokeball += 10;
		if(state->sharedShop->pokeball > 200) {
			state->sharedShop->pokeball = 200;
		}
		state->sharedShop->lullabyPowder += 10;
		if(state->sharedShop->lullabyPowder > 200) {
			state->sharedShop->lullabyPowder = 200;
		}

		state->sharedShop->isLocked = 0;
		fflush(stdout);
	}
}
