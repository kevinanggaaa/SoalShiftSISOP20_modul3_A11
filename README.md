# SoalShiftSISOP20_modul3_A11
## Kevin Angga Wijaya - 05111840000024
## Julius - 05111840000082
![523985](https://user-images.githubusercontent.com/60419316/78343204-b04b8300-75c4-11ea-9141-c19820f7f6ef.jpg)

## Soal 1
**Poke\*ZONE** adalah sebuah game berbasis text terminal mirip dengan Pokemon GO.
Ketentuan permainan sebagai berikut:
 1. Menggunakan IPC-shared memory, thread, fork-exec.
 2. Kalian bebas berkreasi dengan game ini asal tidak konflik dengan requirements yang ada. (Contoh: memberi nama trainer, memberi notifikasi kalau barang di shop sudah penuh, dan lain-lain).
 3. Terdapat 2 code yaitu soal2_traizone.c dan soal2_pokezone.c.
 4. soal2_traizone.c mengandung fitur:
    1. Normal Mode (3 Menu)
       1. Cari Pokemon
          1. Jika diaktifkan maka **setiap 10 detik** akan memiliki 60% chance untuk menemui pokemon bertipe normal, rare, legendary dengan encounter rate sesuai pada tabel di bawah (Gunakan Thread).
          2. Cari pokemon hanya mengatur dia menemukan pokemon atau tidak. Untuk tipe pokemon dan pokemon tersebut shiny atau tidak diatur oleh soal2_pokezone.c.
          3. Opsi “Cari Pokemon” akan berubah jadi “Berhenti Mencari” ketika state mencari pokemon aktif.
          4. State mencari pokemon hanya bisa dihentikan ketika pokemon sudah ditemukan atau trainer memilih untuk berhenti pada menu.
          5. Jika menemui pokemon maka akan masuk ke Capture Mode.
          6. Untuk mempermudah boleh menambah menu go to capture mode untuk berpindah dari mode normal ke mode capture setelah menemukan pokemon dari thread Cari Pokemon.
       2. Pokedex
          1. Melihat list pokemon beserta Affection Point (AP) yang dimiliki.
          2. Maksimal 7 pokemon yang dimiliki.
          3. Jika menangkap lebih dari 7 maka pokemon yang baru saja ditangkap akan langsung dilepas dan mendapatkan pokedollar sesuai dengan tabel dibawah.
          4. Setiap pokemon yang dimiliki, mempunyai Affection Point (AP) dengan initial value 100 dan akan terus berkurang sebanyak **-10 AP/10 detik** dimulai dari waktu ditangkap (Gunakan Thread).
          5. Jika AP bernilai 0, maka pokemon tersebut memiliki 90% chance untuk lepas tanpa memberikan pokedollar ke trainer atau 10% chance untuk reset AP menjadi 50 AP.
          6. AP tidak akan berkurang jika dalam Capture Mode.
          7. Di Pokedex trainer juga bisa melepas pokemon yang ditangkap dan mendapat pokedollar sesuai dengan tabel dibawah.
          8. Bisa memberi berry ke semua pokemon yang dimiliki untuk meningkatkan AP sebesar +10 (1 berry untuk semua pokemon yang ada di pokedex).
       3. Shop
          1. Membeli item dari soal2_pokezone.
          2. Maksimal masing-masing item yang dapat dibeli dan dipunya oleh trainer adalah 99.
    2. Capture Mode (3 Menu)
       1. Tangkap → Menangkap menggunakan pokeball. Berhasil ditangkap maupun tidak, pokeball di inventory -1 setiap digunakan.
       2. Item → Menggunakan item sesuai dengan tabel item dibawah (hanya lullaby powder).
       3. Keluar → Keluar dari Capture Mode menuju Normal Mode.
       <br/>
    * Pokemon tersebut memiliki peluang untuk lari dari trainer sesuai dengan persentase escape rate pada tabel dibawah (gunakan thread).
5. soal2_pokezone.c mengandung fitur:
   1. Shutdown game → Mematikan program soal2_pokezone dan soal2_traizone (Gunakan fork exec).
   2. Jual Item (Gunakan Thread)
      1. Stock awal semua item adalah 100.
      2. Masing-masing item akan bertambah **+10 item/10 detik**.
      3. Maximum item yang dalam shop adalah 200.
      4. List item ada pada tabel dibawah.
      ![image](https://user-images.githubusercontent.com/60419316/78349685-2f918480-75ce-11ea-8239-610ca5333b5c.png)
      5. Gunakan thread untuk implementasi lullaby powder dibuatkan masing-masing 1 thread.
   3. Menyediakan Random Pokemon (Gunakan Thread)
      1. Setiap tipe pokemon memiliki peluang 1/8000 untuk jadi shiny pokemon. (Random dulu antara normal, rare, legendary sesuai rate mereka, baru setelah itu di random dengan peluang 1/8000 untuk tahu dia shiny atau tidak).
      2. Shiny Pokemon meningkatkan escape rate sebesar +5% (misal: tipe normal jadi 10%), capture rate berkurang menjadi -20% (misal: tipe rare jadi 30%), dan pokedollar dari melepas +5000 (misal: tipe legendary jadi 5200 pokedollar).
      3. Setiap 1 detik thread ini akan mengkalkulasi random ulang lagi untuk nantinya diberikan ke soal2_traizone.
![image](https://user-images.githubusercontent.com/60419316/78350251-13421780-75cf-11ea-96fc-21cd2ffd1fa9.png)
<br/>

### Jawaban Soal 1
1. soal1_pokezone.c
```
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
```
2. soal1_traizone.c
```
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
```

## Soal 2
Qiqi adalah sahabat MamMam dan Kaka. Qiqi , Kaka dan MamMam sangat senang bermain “Rainbow six” bersama-sama , akan tetapi MamMam sangat Toxic ia selalu melakukan Team killing kepada Qiqi di setiap permainannya. Karena Qiqi orang yang baik hati, meskipun marah Qiqi selalu berkata “Aku nggk marah!!”. Kaka ingin meredam kemarahan Qiqi dengan membuatkannya sebuah game yaitu TapTap Game. akan tetapi Kaka tidak bisa membuatnya sendiri, ia butuh bantuan mu. Ayo!! Bantu Kaka menenangkan Qiqi.
<br/>
TapTap Game adalah game online berbasis text console. Terdapat 2 program yaitu tapserver.c dan tapplayer.c
<br/>
Syarat :
- Menggunakan Socket, dan Thread
Hint :
- fwrite, fread
<br/>
Spesifikasi Game :
<br/>

### CLIENT SIDE
```
Screen 1 :
1. Login
2. Register
Choices : {your input}
```
<br/>

- Pada screen 1 kalian dapat menginputkan “login”, setelah menekan enter anda diminta untuk menginputkan username dan password seperti berikut

```
Screen 1 :
1. Login
2. Register
Choices : login
Username : { ex : qiqi }
Password : { ex : aku nggak marah!! }
```
<br/>

- Jika login berhasil maka akan menampilkan pesan “login success”, jika gagal akan menampilkan pesan “login failed” (pengecekan login hanya mengecek username dan password, maka dapat multi autentikasi dengan username dan password yang sama)
- Pada screen 1 kalian juga dapat menginputkan “register”, setelah menekan enter anda diminta untuk menginputkan username dan password sama halnya seperti login
- Pada register tidak ada pengecekan unique username, maka setelah register akan langsung menampilkan pesan “register success” dan dapat terjadi double account
- Setelah login berhasil maka anda berpindah ke screen 2 dimana menampilkan 2 fitur seperti berikut.


```
Screen 2 :
1. Find Match
2. Logout
Choices : {your input}
```
<br/>

- Pada screen 2 anda dapat menginputkan “logout” setelah logout anda akan kembali ke screen 1
- Pada screen 2 anda dapat menginputkan “find”, setelah itu akan menampilkan pesan “Waiting for player ...” print terus sampai menemukan lawan
- Jika menemukan lawan maka akan menampilkan pesan “Game dimulai silahkan tap tap secepat mungkin !!”
- Pada saat game dimulai diberikan variable health = 100,dan anda dapat men-tap (menekan space pada keyboard tanpa harus menekan enter)
- Pada saat anda men-tap maka akan menampilkan pesan “hit !!”, dan pada lawan healthnya akan berkurang sebanyak 10 kemudian pada lawan menampilkan pesan status healthnya sekarang. (conclusion : anda tidak bisa melihat status health lawan)
- Jika health anda <= 0 maka akan menampilkan pesan “Game berakhir kamu kalah”, apabila lawan anda healthnya <= 0 maka akan menampilkan pesan ”Game berakhir kamu menang”
- Setelah menang atau kalah maka akan kembali ke screen 2
</br>

### SERVER SIDE

- Pada saat program pertama kali dijalankan maka program akan membuat file akun.txt jika file tersebut tidak ada. File tersebut digunakan untuk menyimpan username dan password
- Pada saat user berhasil login maka akan menampilkan pesan “Auth success” jika gagal “Auth Failed”
- Pada saat user sukses meregister maka akan menampilkan List account yang terdaftar (username dan password harus terlihat)

### Jawaban Soal 2
1. client.c
```
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
```
2. server.c
```
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
```

## Soal 3
Buatlah sebuah program dari C untuk mengkategorikan file. Program ini akan memindahkan file sesuai ekstensinya (tidak case sensitive. JPG dan jpg adalah sama) ke dalam folder sesuai ekstensinya yang folder hasilnya terdapat di working directory ketika program kategori tersebut dijalankan.
- Semisal program dijalankan:
![image](https://user-images.githubusercontent.com/60419316/78861760-ce4f3280-7a5f-11ea-91f5-d3791657b67a.png)  
- Pada opsi -f tersebut, user bisa menambahkan argumen file yang bisa
dikategorikan sebanyak yang user inginkan seperti contoh di atas.
- Pada program kategori tersebut, folder jpg,c,zip tidak dibuat secara manual, melainkan melalui program c. Semisal ada file yang tidak memiliki ekstensi, maka dia akan disimpan dalam folder “Unknown”.
- Program kategori ini juga menerima perintah (\*) seperti di bawah;
![image](https://user-images.githubusercontent.com/60419316/78861786-e7f07a00-7a5f-11ea-8f90-f20509e46826.png)  
- Artinya mengkategori seluruh file yang ada di working directory ketika menjalankan program C tersebut.
- Selain hal itu program C ini juga menerima opsi -d untuk melakukan kategori pada suatu directory. Untuk opsi -d ini, user hanya bisa menginput 1 directory saja, tidak seperti file yang bebas menginput file sebanyak mungkin.
![image](https://user-images.githubusercontent.com/60419316/78861858-1bcb9f80-7a60-11ea-9a8f-d3fce208e630.png)  
- Hasilnya perintah di atas adalah mengkategorikan file di /path/to/directory dan hasilnya akan disimpan di working directory di mana program C tersebut berjalan (hasil kategori filenya bukan di /path/to/directory).
- Program ini tidak rekursif. Semisal di directory yang mau dikategorikan, atau menggunakan (\*) terdapat folder yang berisi file, maka file dalam folder tersebut tidak dihiraukan, cukup file pada 1 level saja.
- Setiap 1 file yang dikategorikan dioperasikan oleh 1 thread agar bisa berjalan secara paralel sehingga proses kategori bisa berjalan lebih cepat. Dilarang juga menggunakan fork-exec dan system.
- Silahkan download soal3.zip sebagai percobaan. Namun silahkan dicoba-coba sendiri untuk kemungkinan test case lainnya yang mungkin
belum ada di soal3.zip.

### Jawaban Soal 3
1. soal3.c
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <limits.h>
#include <dirent.h> 
#include <libgen.h> //library genesis
#include <ctype.h>
#include <pthread.h>

char *currExecFileName;

int is_regular_file(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

char *getExtension(char *filename) {
    char *dot = strrchr(filename, '.');
		// If dot is NULL or dot doesn't exist
    if(!dot || dot == filename) return "Unknown";
    return dot + 1;
}

char *toLower(char *string) {
	for(int i = 0; string[i]; i++){
		string[i] = tolower(string[i]);
	}
	return string;
}

void createDirIfNotExist(char *dir) {
	struct stat st = {0};

	if (stat(dir, &st) == -1) {
			mkdir(dir, 0777);
	}
}

void *moveFile(void *temp) {
	char *path;
	path = (char *)temp;
	printf("Moving file: %s\n", path);

	char *filename;
	filename = basename(path);
	char cwd[500];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		printf("Should never happen\n");
		exit(EXIT_FAILURE);
	}

	char newname[500];
	// pwd + / + de->d_name;
	char fileExt[500];
	strcpy(fileExt, getExtension(filename));
	strcpy(fileExt, toLower(fileExt));

	sprintf(newname, "%s/%s", cwd, fileExt);
	createDirIfNotExist(newname);

	sprintf(newname, "%s/%s", newname, filename);
	printf("From: %s | To: %s\n", path, newname);

	int result = rename(path , newname);
  if ( result == 0 )
    puts ( "File successfully renamed" );
  else
    perror( "Error renaming file" );
}

void processFolder(char *folderPath) {
	struct dirent *de;  // Pointer for directory entry 
	DIR *dr = opendir(folderPath); 
	if (dr == NULL) { // opendir returns NULL if couldn't open directory 
		printf("couldn't open directory: %s\n", folderPath); 
		return; 
	} 

	pthread_t thread[1000];
	int n = 0;
	while ((de = readdir(dr)) != NULL)  {
		printf("Iterating over dir: %s\n", de->d_name);
		char *filePath = malloc(500 * sizeof(char));
		sprintf(filePath, "%s%s", folderPath, de->d_name);

		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 ) {
		// Skip the first 2 directory of symlinks to . and ..
			continue;
		} else if (strcmp(de->d_name, __FILE__) == 0) {
		// Skip the current source file
			continue;
		} else if (strcmp(de->d_name, currExecFileName) == 0) {
		// Skip the current executable file
			continue;
		} else if (!is_regular_file(filePath)) {
		// Skip if it is a directory and not a 
			continue;
		}

		pthread_create(&thread[n++], NULL, moveFile, (void *) filePath);
	}

	// Wait for worker thread to finish
	for (int i = 0; i < n; i++)
	{
		pthread_join(thread[i], NULL);
	}
	
}

int main(int argc, char* argv[]) {
	currExecFileName = basename(argv[0]);
	// argc argv handling
	if (strcmp (argv[1], "-d") == 0) {
	// If -d option
		if (argc > 3) {
		// If more than 1 directory
			// Exit Error
			printf("Input unrecognized");
			exit(EXIT_FAILURE);
		}
		// process paths
		processFolder(argv[2]);
	} else if (strcmp (argv[1], "-f") == 0) {
	// If -f option
		for (int i = 2; i < argc; i++) {
		// Loop until the end of arguments
			// Process paths
			moveFile(argv[i]);
		}
	} else if (strcmp (argv[1], "*") == 0) {
	// If argument is *
		char cwd[500];
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			printf("Should never happen\n");
			exit(EXIT_FAILURE);
		}
		strcat(cwd, "/");
		// Process path
		processFolder(cwd);
	} else {
		// Exit Error
		printf("Input unrecognized");
		exit(EXIT_FAILURE);
	}
}
```

## Soal 4
Norland adalah seorang penjelajah terkenal. Pada suatu malam Norland menyusuri jalan setapak menuju ke sebuah gua dan mendapati tiga pilar yang pada setiap pilarnya ada sebuah batu berkilau yang tertancap. Batu itu berkilau di kegelapan dan setiap batunya memiliki warna yang berbeda. Norland mendapati ada sebuah teka-teki yang tertulis di setiap pilar. Untuk dapat mengambil batu mulia di suatu pilar, Ia harus memecahkan teka-teki yang ada di pilar tersebut. Norland menghampiri setiap pilar secara bergantian.
- **Batu mulia pertama**. Emerald. Batu mulia yang berwarna hijau mengkilat. Pada batu itu Ia menemukan sebuah kalimat petunjuk. Ada sebuah teka-teki yang berisi:
  1. Buatlah program C dengan nama **"4a.c"**, yang berisi program untuk melakukan perkalian matriks. Ukuran matriks pertama adalah **4x2**, dan matriks kedua **2x5**. Isi dari matriks didefinisikan **di dalam** kodingan. Matriks nantinya akan berisi angka 1-20 (**tidak perlu** dibuat filter angka).
  2. Tampilkan matriks hasil perkalian tadi ke layar.

- **Batu kedua** adalah Amethyst. Batu mulia berwarna ungu mengkilat. Teka-tekinya adalah:
  1. Buatlah program C kedua dengan nama **"4b.c"**. Program ini akan mengambil variabel **hasil perkalian** matriks dari program "4a.c" (program sebelumnya), dan tampilkan hasil matriks tersebut ke layar. 
  (**Catatan!**: gunakan shared memory)
  2. Setelah ditampilkan, berikutnya untuk setiap angka dari matriks tersebut, carilah nilai **penjumlahan faktorialnya**, dan tampilkan hasilnya ke layar dengan format seperti matriks.

Contoh: misal array [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], ...],

maka:
```
1 2 6 24
120 720 ... ...
...
```
(**Catatan!** : Harus menggunakan Thread dalam penghitungan faktorial)
- **Batu ketiga** adalah Onyx. Batu mulia berwarna hitam mengkilat. Pecahkan teka-teki berikut!
  1. Buatlah program C ketiga dengan nama **"4c.c"**. Program ini tidak memiliki hubungan terhadap program yang lalu.
  2. Pada program ini, Norland diminta mengetahui jumlah file dan folder di direktori saat ini dengan command **"ls | wc -l"**. Karena sudah belajar IPC, Norland mengerjakannya dengan semangat.
(**Catatan!** : Harus menggunakan IPC Pipes)
<br/>
Begitu batu terakhir berhasil didapatkan. Gemuruh yang semakin lama semakin besar terdengar. Seluruh tempat berguncang dahsyat, tanah mulai merekah. Sebuah batu yang di atasnya terdapat kotak kayu muncul ke atas dengan sendirinya. Sementara batu tadi kembali ke posisinya. Tanah kembali menutup, seolah tidak pernah ada lubang merekah di atasnya satu detik lalu. 
<br/>  
Norland segera memasukkan tiga buah batu mulia Emerald, Amethys, Onyx pada Peti Kayu. Maka terbukalah Peti Kayu tersebut. Di dalamnya terdapat sebuah harta karun rahasia. Sampai saat ini banyak orang memburu harta karun tersebut. Sebelum menghilang, dia menyisakan semua petunjuk tentang harta karun tersebut melalui tulisan dalam buku catatannya yang tersebar di penjuru dunia. "One Piece does exist".

### Jawaban Soal 4
1. soal4a.c
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> //library thread

void *print_message_function( void *ptr );
void *calculateMatrix( void *ptr );

// Source matrices
// Make sure it fulfils the multiplication rules of matrices
int matrix1[4][2] = {
  1, 2,
  3, 4,
  5, 6,
  7, 8 
};

int matrix2[2][5] = {
  9, 10, 11, 12, 13,
  14, 15, 16, 17, 18
};

// size of matrice
int mat1I = 4, mat1J = 2;
int mat2I = 2, mat2J = 5;
// values to be returned by thread processes
// int retVals[mat1I][mat2J];
int retVals[4][5];
// semaphone for retMatrix
int retMatLock = 0;


int main()
{
  pthread_t thread[mat1I][mat2J];//inisialisasi awal
  int iret[mat1I][mat2J];
  

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
      if(iret[i][j]) { //jika eror
          fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][j]);
          exit(EXIT_FAILURE);
      }

      // printf("pthread_create() for thread 1 returns: %d\n", iret[i][j]);
    }
  }

  for (i = 0; i < mat1I; i++) {
    for (j = 0; j < mat2J; j++) {
      pthread_join( thread[i][j], NULL );
    //   printf("thread joined\n");
    }
  }

  for (i = 0; i < mat1I; i++) {
    printf("%d", retVals[i][0]);
    for (j = 1; j < mat2J; j++) {
      printf(" %d", retVals[i][j]);
    }
    printf("\n");
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
    // printf("%d,%d multi: %d * %d = %d\n", 
    // iMat1, jMat2, matrix1[iMat1][i], matrix2[i][jMat2], sum);
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
```
2. soal4b.c
```
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
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                printf("%d ", buffer[i*5+j]);
            }
            printf("\n");
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
```
3. soal4c.c
```
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
```
### Bukti
![image](https://user-images.githubusercontent.com/60419316/78980214-a76c2b80-7b47-11ea-9a60-889f6d8ee8dd.png)
