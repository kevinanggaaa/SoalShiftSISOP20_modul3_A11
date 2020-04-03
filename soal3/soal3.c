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
