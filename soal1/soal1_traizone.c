#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NORMAL_MODE 0
#define POKEDEX 1
#define SHOP 2
#define CAPTURE_MODE 3

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

typedef struct Pokemon_s{
	int released;
	char name[20];
	int type;
	int ap;
	int price;
}Pokemon_t;

typedef struct Inventory_s{
	int pokeball;
	int lullabyPowder;
	int berry;
	int pokedollar;
}Inventory_t;

typedef struct PokeBag_s{
	Pokemon_t * pokemon[7];
	int count;
}PokeBag_t;

typedef struct State_s{
	int gameState;
	int isRunning;
	int cariPokemon;
	int jumlahLullabyActive;
	Inventory_t * inventory;
	PokeBag_t * pokeBag;
	RandPokemon_t * randPokemon;
	Shop_t * sharedShop;
}State_t;

typedef struct WildPokemonParam_s{
	State_t * state;
	int isRunning;
}WildPokemonParam_t;

typedef struct PokemonParam_s{
	Pokemon_t * pokemon;
	State_t * state;
}PokemonParam_t;

void * cariPokemonThread(void * param);
void * pokemonThread(void * param);
void * wildpokemonThread(void * param);
void * lullabyThread(void * param);
void addApPokemon(Pokemon_t * pokemon, int ap);
int addPokemon(State_t * pokeBag, Pokemon_t * pokemon);
Pokemon_t * tamePokemon(RandPokemon_t * wildPokemon);
void removePokemon(PokeBag_t * pokeBag, int index);
void normalMode(State_t *);
void printNormalMenu(State_t *state);
void pokedex(State_t *);
void printPokedexMenu(State_t *state);
void shop(State_t *);
void printShopMenu(Shop_t *sharedShop, Inventory_t *inventory);
void captureMode(State_t *);
void printCaptureMenu(RandPokemon_t *pokemon);

void normalMode(State_t * state){
	printNormalMenu(state);
	
	int choice;
	scanf("%d", &choice);

	if(choice == 1){
		if(state->cariPokemon){
			state->cariPokemon = 0;
		}else{
			state->cariPokemon = 1;
			pthread_t thread;
			if(pthread_create(&thread, NULL, &cariPokemonThread, (void *)state) < 0){
				printf("can't create thread\n");
				exit(EXIT_FAILURE);
			}
		}
	}else if(choice == 2){
		state->gameState = POKEDEX;
	}else if(choice == 3){
		state->gameState = SHOP;
	}
}

void printNormalMenu(State_t *state) {
	printf("--MAIN MENU--\n");
	if(state->cariPokemon){
		printf("1. Berhenti Mencari\n");
	}else{
		printf("1. Cari Pokemon\n");
	}
	printf("2. Pokedex\n");
	printf("3. Shop\n");
	printf("4. Refresh\n");
	printf("Choice : ");
}

void pokedex(State_t * state){
	printPokedexMenu(state);
	int choice;
	scanf("%d", &choice);
	if(choice == 1){
		if(state->inventory->berry){
			if(state->pokeBag->count){
				printf("Telah dikasi makan\n");
				state->inventory->berry -= 1;
				for(int i = 0; i < state->pokeBag->count; i++){
					addApPokemon(state->pokeBag->pokemon[i], 10);
				}
			}else{
				printf("Tidak punya pokemon\n");
			}
		}else{
			printf("Tidak punya berry\n");
		}
	}else if(choice == 2){
		printf("index pokemon yang mau dilepas : ");
		int pokemon;
		scanf("%d", &pokemon);
		if(pokemon <= 0 || pokemon > state->pokeBag->count){
			printf("Tidak ada yang dilepas\n");
		}else{
			int price = state->pokeBag->pokemon[pokemon-1]->price;
			removePokemon(state->pokeBag, pokemon-1);
			state->inventory->pokedollar+=price;
			printf("Pokemon dilepas, Pokedollar anda sekarang adalah %d\n", state->inventory->pokedollar);
		}
	}else if(choice == 3){
		state->gameState = NORMAL_MODE;
	}
}

void printPokedexMenu(State_t *state) {
	printf("--POKEDEX--\n");
	if(state->pokeBag->count == 0){
		printf("Anda tidak mempunyai pokemon\n");
	}else{
		for(int i = 0; i < state->pokeBag->count; i++){
			printf("---------------\n");
			printf("%d. %s\n", i+1, state->pokeBag->pokemon[i]->name);
			printf("TYPE : %d\n", state->pokeBag->pokemon[i]->type);
			printf("AP : %d\n", state->pokeBag->pokemon[i]->ap);
			printf("Pokedollar : %d\n", state->pokeBag->pokemon[i]->price);
			printf("---------------\n");
		}
	}
	printf("Berry : %d\n", state->inventory->berry);
	printf("---------------\n");
	printf("1. Beri makan\n");
	printf("2. Lepas pokemon\n");
	printf("3. Kembali\n");
	printf("4. Refresh\n");
	printf("Choice : ");
}

void shop(State_t * state){
	Shop_t * sharedShop = state->sharedShop;
	Inventory_t * inventory = state->inventory;
	printShopMenu(sharedShop, inventory);
	int choice;
	scanf("%d", &choice);
	if(choice == 2){
		if(inventory->pokedollar < sharedShop->lullabyPowderPrice){
			printf("Pokedollar tidak cukup\n");
		} else if (inventory->lullabyPowder >= 99){
			printf("Anda tidak bisa menampung lullabyPowder lagi!\n");
		}else{
			if(sharedShop->lullabyPowder){
				printf("Telah dibeli 1 lullabyPowder\n");
				inventory->lullabyPowder++;
				inventory->pokedollar -= sharedShop->lullabyPowderPrice;

				while(state->sharedShop->isLocked) usleep(10000);
				state->sharedShop->isLocked = 1;
				state->sharedShop->lullabyPowder--;
				state->sharedShop->isLocked = 0;
			}else{
				printf("Stock lullaby powder habis\n");
			}
		}
	}else if(choice == 3){
		if(inventory->pokedollar < sharedShop->pokeballPrice){
			printf("Pokedollar tidak cukup\n");
		} else if (inventory->pokeball >= 99){
			printf("Anda tidak bisa menampung pokeball lagi!\n");
		}else{
			if(sharedShop->pokeball){
				printf("Telah dibeli 1 pokeball\n");
				inventory->pokeball++;
				inventory->pokedollar -= sharedShop->pokeballPrice;

				while(state->sharedShop->isLocked) usleep(10000);
				state->sharedShop->isLocked = 1;
				state->sharedShop->pokeball--;
				state->sharedShop->isLocked = 0;
			}else{
				printf("Stock pokeball habis\n");
			}
		}
	}else if(choice == 4){
		if(inventory->pokedollar < sharedShop->berryPrice){
			printf("Pokedollar tidak cukup\n");
		} else if (inventory->berry >= 99){
			printf("Anda tidak bisa menampung berry lagi!\n");
		}else{
			if(sharedShop->berry){
				printf("Telah dibeli 1 berry\n");
				inventory->berry++;
				inventory->pokedollar -= sharedShop->berryPrice;

				while(state->sharedShop->isLocked) usleep(10000);
				state->sharedShop->isLocked = 1;
				state->sharedShop->berry--;
				state->sharedShop->isLocked = 0;
			}else{
				printf("Stock berry habis\n");
			}
		}
	}else if(choice == 5){
		state->gameState = NORMAL_MODE;
	}
}

void printShopMenu(Shop_t *sharedShop, Inventory_t *inventory) {
	printf("--SHOP--\n");
	printf("List item : \n");
	printf("1. Lullaby Powder | Stock : %d | Price : %d Pokedollar\n", sharedShop->lullabyPowder, sharedShop->lullabyPowderPrice);
	printf("2. Pokeball | Stock : %d | Price : %d Pokedollar\n", sharedShop->pokeball, sharedShop->pokeballPrice);
	printf("3. Berry | Stock : %d | Price : %d Pokedollar\n", sharedShop->berry, sharedShop->berryPrice);
	printf("----------------\n");
	printf("You have :\n");
	printf("1. Lullaby Powder | Stock : %d\n", inventory->lullabyPowder);
	printf("2. Pokeball | Stock : %d\n", inventory->pokeball);
	printf("3. Berry | Stock : %d\n", inventory->berry);
	printf("4. Pokedollar | Stock : %d\n", inventory->pokedollar);
	printf("----------------\n");
	printf("Aksi :\n");
	printf("1. Refresh\n");
	printf("2. Beli Lullabypowder\n");
	printf("3. Beli Pokeball\n");
	printf("4. Beli Berry\n");
	printf("5. Kembali\n");
	printf("choice : ");
}

void captureMode(State_t * state){
	RandPokemon_t * pokemon = state->randPokemon;
	pokemon->isLocked = 1;
	pthread_t wild;

	WildPokemonParam_t * param = malloc(sizeof(WildPokemonParam_t));
	param->state = state;
	param->isRunning = 1;

	if(pthread_create(&wild, NULL, &wildpokemonThread, (void *)param) < 0){
		printf("can't create thread\n");
		exit(EXIT_FAILURE);
	}

	int choice;
	while(pokemon->isLocked){
		printCaptureMenu(pokemon);
		scanf("%d", &choice);
		if(!pokemon->isLocked){
			printf("Pokemon telah kabur\n");
			break;
		}
		if(choice == 1){

			if(state->inventory->pokeball > 0){
				state->inventory->pokeball--;
				int random = rand()%(pokemon->capRateB);
				if(random < pokemon->capRateA + state->jumlahLullabyActive * 2){

					param->isRunning = 0;

					Pokemon_t * tamedPokemon = tamePokemon(pokemon);
					if(addPokemon(state, tamedPokemon)){
						pthread_t t;
						PokemonParam_t * param = malloc(sizeof(PokemonParam_t));
						param->state = state;
						param->pokemon = tamedPokemon;
						if(pthread_create(&t, NULL, &pokemonThread, param) < 0){
							printf("can't create thread\n");
							exit(EXIT_FAILURE);
						}
						printf("Tertangkap\n");
						state->gameState = NORMAL_MODE;
						pokemon->isLocked = 0;
					}else{
						printf("Tertangkap akan tetapi slot full jadi terlepas\n");
						free(tamedPokemon);
						pokemon->isLocked = 0;
						state->gameState = NORMAL_MODE;
					}

				}else{
					printf("Tidak kena\n");
				}
			}else{
				printf("Tidak punya pokeball\n");
			}

		}else if(choice == 2){
			
			if(state->inventory->lullabyPowder){

				state->inventory->lullabyPowder--;
				pthread_t lullaby;
				if(pthread_create(&lullaby, NULL, &lullabyThread, state) < 0){
					printf("can't create thread\n");
					exit(EXIT_FAILURE);
				}

			}else{
				printf("Tidak punya lullaby powder\n");
			}

		}else if(choice == 3){
			pokemon->isLocked = 0;
			state->gameState = NORMAL_MODE;
		}
	}
	pokemon->isLocked = 0;
}

void printCaptureMenu(RandPokemon_t *pokemon) {
		printf("--CAPTURE MODE--\n");
		printf("Pokemon :\n");
		printf("Nama : %s\n", pokemon->name);
		printf("Type : %d\n", pokemon->type);
		printf("Price : %d\n", pokemon->price);
		printf("Aksi :\n");
		printf("1. Tangkap\n");
		printf("2. Gunakan lullaby powder\n");
		printf("3. Keluar\n");
		printf("choice : ");
}

int main(){
	srand(time(0));

	Inventory_t * inventory = malloc(sizeof(Inventory_t));
	memset(inventory, 0, sizeof(Inventory_t));
	inventory->pokeball = 100;

	PokeBag_t * pokeBag = malloc(sizeof(PokeBag_t));
	memset(pokeBag, 0, sizeof(PokeBag_t));

	State_t * state = malloc(sizeof(State_t));
	memset(state, 0, sizeof(state));
	state->pokeBag = pokeBag;
	state->inventory = inventory;

	key_t key = 1111;
	RandPokemon_t * randPokemon;
	int shmid =  shmget(key, sizeof(RandPokemon_t), IPC_CREAT | 0666);
	randPokemon = shmat(shmid, NULL, 0);

	state->randPokemon = randPokemon;

	key_t key2 = 2222;
	Shop_t * sharedShop;
	int shmid2 =  shmget(key2, sizeof(Shop_t), IPC_CREAT | 0666);
	sharedShop = shmat(shmid2, NULL, 0);

	state->sharedShop = sharedShop;

	int * isRunning = &state->isRunning;
	*isRunning = 1;
	int * gameState = &state->gameState;
	while(*isRunning){
		if(*gameState == NORMAL_MODE){
			normalMode(state);
		}else if(*gameState == POKEDEX){
			pokedex(state);
		}else if(*gameState == SHOP){
			shop(state);
		}else if(*gameState == CAPTURE_MODE){
			captureMode(state);
		}
	}

	shmdt(randPokemon);
	shmctl(shmid, IPC_RMID, NULL);

	shmdt(sharedShop);
	shmctl(shmid2, IPC_RMID, NULL);

	return 0;
}

void addApPokemon(Pokemon_t * pokemon, int ap){
	pokemon->ap += ap;
}

int addPokemon(State_t * state, Pokemon_t * pokemon){
	if(state->pokeBag->count == 7) {
		state->inventory->pokedollar += pokemon->price;
		return 0;
	} else {
		state->pokeBag->pokemon[state->pokeBag->count++] = pokemon;
		return 1;
	}
}

void removePokemon(PokeBag_t * pokeBag, int index){
	// pokeBag->pokemon[index-1]->released = 1;
	free(pokeBag->pokemon[index]);
	// Push back
	for(int i = index; i < pokeBag->count-1; i++){
		pokeBag->pokemon[i] = pokeBag->pokemon[i + 1];
	}
	pokeBag->pokemon[--pokeBag->count] = NULL;
}

Pokemon_t * tamePokemon(RandPokemon_t * wildPokemon){
	Pokemon_t * newPokemon = malloc(sizeof(Pokemon_t));
	memset(newPokemon, 0, sizeof(newPokemon));
	strcpy(newPokemon->name, wildPokemon->name);
	newPokemon->type = wildPokemon->type;
	newPokemon->ap = 100;
	newPokemon->price = wildPokemon->price;
	return newPokemon;
}

void * wildpokemonThread(void * param){
	WildPokemonParam_t * parameter = (WildPokemonParam_t *)param;
	State_t * state = parameter->state;
	while (state->isRunning && parameter->isRunning){
		sleep(20);
		if(!parameter->isRunning || !state->isRunning){
			break;
		}
		if(state->jumlahLullabyActive) {
			continue;
		}
		int random = rand()%(state->randPokemon->escapeRateB);
		if(random < state->randPokemon->escapeRateA){
			state->randPokemon->isLocked = 0;
			break;
		}
	}
	
}

void * cariPokemonThread(void * param){
	State_t * state = (State_t *) param;
	while(state->cariPokemon){
		if(rand()%10 <= 5){
			state->cariPokemon = 0;
			state->gameState = CAPTURE_MODE;
			break;
		}
		sleep(10);
	}
}

void * pokemonThread(void * param){
	PokemonParam_t * p = (PokemonParam_t *)param;
	while (p->state->isRunning && !p->pokemon->released){
		sleep(10);
		if(p->state->gameState == CAPTURE_MODE){
			continue;
		}
		p->pokemon->ap -= 10;
		if(p->pokemon->ap == 0){
			int random = rand()%10;
			if(random == 0){
				p->pokemon->ap = 50;
			}else{
				//lepas
				for(int i = 0; i < p->state->pokeBag->count; i++){
					if(p->state->pokeBag->pokemon[i] == p->pokemon){
						removePokemon(p->state->pokeBag, i);
						break;
					}
				}
			}
		}
	}
	
}

void * lullabyThread(void * param){
	State_t * state = (State_t *)param;
	state->jumlahLullabyActive++;
	sleep(10);
	state->jumlahLullabyActive--;
}
