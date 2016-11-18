
/*/////////////////////////////////////////////
//  	  OPERATING SYSTEMS HOMEWORK #3:
//		Multithreaded file parsing using POSIX
//				10/31/2016
//		CREATED USING CODE FROM: HOMEWORK #1
//============================================
//				WRITTEN BY:
//		     CLAYTON RAYMENT
//			    661133772	    
/////////////////////////////////////////////*/


/*Number of words that caused out-of-memory case: 108000000+ (text editor crashed)*/



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

typedef struct wordArray wordArray;
typedef struct wordStruct wordStruct;
typedef struct threadData threadData;

struct threadData{
	wordArray* wA;
	char* filename;
};

struct wordStruct{
	char* word;
	char* filename;
};

/*structure which contains the array of words and relevant information*/
struct wordArray{
	pthread_mutex_t lock;
	wordStruct* words;
	int numWords;
	int size;
};

void printMsg(char* message, unsigned int tid){
	if(tid > 0){
		printf("THREAD %u: %s\n", tid, message);
	}
	else if (tid == 0){
		printf("MAIN THREAD: %s\n", message);
	}

	fflush(stdout);
}

/*This function initializes a word array with the
//indicated starting size, which can be changed:*/
void initializeWordArray(wordArray* arr, int debug){
	int initialSize = 8;
	if(pthread_mutex_init(&(arr->lock), NULL) != 0){
		printf("MUTEX INIT FAILED\n");
	}
	if(debug){
		printMsg("Allocated initial array of 8 character pointers.", 0);
	}
	arr->words = calloc(initialSize, sizeof(wordStruct));
	arr->size = initialSize;
	arr->numWords = 0;
}

/*This function destroys a given word array:*/
void destroyWordArray(wordArray* arr){
	int i = 0;
	for(;i < arr->numWords; ++i){
		free(arr->words[i].word);
		free(arr->words[i].filename);
	}
	free(arr->words);
	arr = NULL;	
}

/*This function resizes a given word array to twice its size:*/
void resizeWordArray(wordArray* arr, int debug){
	/*wordArray* temp = NULL;*/
	arr->words = realloc(arr->words, arr->size * 2 * sizeof(wordStruct));
	arr->size = arr->size * 2;
	if(debug){
		char* msg = calloc(256, sizeof(char));
		sprintf(msg, "Re-allocated array of %d pointers.", arr->size);
		printMsg(msg, pthread_self());
		free(msg);
	}
}

/*This function takes in a character pointer and adds that word to the word array:*/
void addWordToArray(wordArray* arr, char* word, char* filename, int debug){

	int buffSize = 128;
	int index = 0;
	int tempIndex = 0;
	int slot = 0;

	/*Assign slot to thread*/
	pthread_mutex_lock(&(arr->lock));
	/*Check to see if we need to resize the word array:*/
	if(arr->numWords + 1 > arr->size){
		resizeWordArray(arr, debug);
	}
	/*get slot to put data into*/
	slot = arr->numWords;
	++arr->numWords;


	/*Add word to struct:*/
	arr->words[slot].word = calloc(buffSize, sizeof(char));
	while(word[index] != '\0')
	{
		if(isalnum(word[index])){
			arr->words[slot].word[tempIndex] = word[index];
			++tempIndex;
		}
		++index;
	}
	arr->words[slot].word[tempIndex] = '\0';
	arr->words[slot].word = realloc(arr->words[slot].word, (tempIndex + 1) * sizeof(char));
	if(debug == 1){
		char buffer[1024];
		sprintf(buffer,"Added \"%s\" at index %d.", arr->words[slot].word, slot);
		printMsg(buffer, pthread_self());
	}
	/*Add which file the word came from:*/
	index = 0;
	tempIndex = 0;
	arr->words[slot].filename = calloc(buffSize, sizeof(char));
	while(filename[index] != '\0')
	{
		if(isalnum(filename[index]) || filename[index] == '.'){
			arr->words[slot].filename[tempIndex] = filename[index];
			++tempIndex;
		}
		++index;
	}
	arr->words[slot].filename[tempIndex] = '\0';
	++tempIndex;
	arr->words[slot].filename = realloc(arr->words[slot].filename, tempIndex  * sizeof(char));
	/*printf("%d: %s\n", arr->numWords, arr->words[arr->numWords]);*/
	pthread_mutex_unlock(&(arr->lock));
}

/*This function prints out all the elements of a word array:*/
void printArray(wordArray* arr){
	char buffer[1024];
	int i = 0;
	for(;i < arr->numWords; ++i){
		sprintf(buffer, "%s (from \"%s\").", arr->words[i].word, arr->words[i].filename);
		printMsg(buffer, 0);
	}
}

/*This function reads in the input file, and adds words to the word array
//that was given as an argument to the function.*/
int readListArray(wordArray* arr, char* filename){
	int buffSize = 64;
	FILE* fp;
	fp = fopen(filename, "r");
	char* tempWord = calloc(buffSize, sizeof(char));
	while(fscanf(fp, "%s", tempWord) != EOF){
		addWordToArray(arr, tempWord, filename, 1);
	}

	return 1;
}

/*This search function goes through each word in the array and uses strstr to
//find a substring. If it matches, that word is placed into a seperate word array
//containing only words that have matching substrings:*/
void substringSearch(wordArray* arr, wordArray* matches, char* substring){
	int i = 0;
	for(;i < arr->numWords; ++i){
		if(arr->words[i].word[strlen(arr->words[i].word)] != '\0'){
		}
		if(strstr(arr->words[i].word, substring) != NULL){
			addWordToArray(matches, arr->words[i].word, arr->words[i].filename, 0);
		}
	}
}

void* threadMain(void* tD){
	char buffer[1024];
	threadData* data = (threadData*) tD;
	sprintf(buffer, "Assigned \"%s\" to child thread %lu", data->filename, pthread_self());
	printMsg(buffer, 0);
	readListArray(data->wA, data->filename);
	/*printMsg(data->filename);*/
	pthread_exit(NULL);
}

/*Search through directory, and initialize a thread for each file*/
int searchDirectory(char* directory, char* extension, wordArray* dW, threadData** data){

	DIR* p = NULL;
	struct dirent* entry = NULL;

	int n = 0;

	p = opendir(directory);
	if(p == NULL){
		perror("OPENDIR");
		return -1;
	}

	while((entry = readdir(p))){

		int l = strlen(entry->d_name);
		if(strncmp(entry->d_name + l - strlen(extension), extension, strlen(extension)) == 0){
			++n;
			(*data) = realloc((*data), n * sizeof(threadData));
			(*data)[n-1].wA = dW;
			
			(*data)[n-1].filename = calloc((l + 1), sizeof(char));
			sprintf((*data)[n-1].filename, "%s", entry->d_name);
			/*int i = 0;
			for(; i < l; ++i){
				(*data)[n-1].filename[i] = entry->d_name[i];
			}
			(*data)[n-1].filename[i] = '\0';*/
		}
	}

	closedir(p);
	return n;
}

void printSuccess(wordArray* arr, int numFiles){
	char msg[1024];
	sprintf(msg, "All done (successfully read %d words from %d files).", arr->numWords, numFiles);
	printMsg(msg, 0);
}

void createThreads(pthread_t** threads, pthread_attr_t* attr, threadData** data, int n){
	(*threads) = realloc((*threads), n * sizeof(pthread_t));
	int rc;
	int i = 0;
	pthread_mutex_lock(&(((*data)[0]).wA->lock));
	for(; i < n; ++i){
		rc = pthread_create(&((*threads)[i]), attr, threadMain, (void*) &((*data)[i]));
		if(rc){
			printf("ERROR: return code from pthread_create() is %d\n", rc);
		}
	}
	pthread_mutex_unlock(&(((*data)[0]).wA->lock));
}

/*Main function*/
int main(int argc, char* argv[]){
	if(argc != 3){
		fprintf(stderr, "Incorrect number of arguments\n");
		return -1;
	}

	wordArray directoryWords;
	initializeWordArray(&directoryWords, 1);
	threadData* directoryData = malloc(1*sizeof(threadData));
	pthread_t* threads = malloc(1 * sizeof(pthread_t));
	int n = searchDirectory(argv[1], ".txt", &directoryWords, &directoryData);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	createThreads(&threads, &attr, &directoryData, n);
	
	/*wait for threads to finish and rejoin*/
	int i = 0;
	for(; i < n; ++i){
		pthread_join(threads[i], NULL);
	}
	printSuccess(&directoryWords, n);
	/*begin substring search*/
	wordArray returnedWords;
	initializeWordArray(&returnedWords, 0);
	substringSearch(&directoryWords, &returnedWords, argv[2]);
	
	char buffer[1024];
	strcpy(buffer, "Words containing substring \"");
	strcat(buffer, argv[2]);
	strcat(buffer, "\" are:\0");
	printMsg(buffer, 0);
	printArray(&returnedWords);

	destroyWordArray(&directoryWords);
	destroyWordArray(&returnedWords);

	pthread_exit(NULL);
	return 1;
}