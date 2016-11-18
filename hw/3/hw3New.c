
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

/*thread data passed to each thread*/
struct threadData{
	wordArray* wA;
	char* filename;
	char* directory;
};

/*struct for each word, contains the filename*/
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

/*prints a nice message formatted with the thread id*/
void printMsg(char* message, long unsigned int tid){
	if(tid > 0){
		printf("THREAD %lu: %s\n", tid, message);
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
		perror("MUTEX INIT FAILED\n");
	}
	if(debug){
		printMsg("Allocated initial array of 8 pointers.", 0);
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
		msg = NULL;
	}
}

/*This function takes in a character pointer and adds that word to the word array:*/
void addWordToArray(wordArray* arr, char* word, char* filename, int debug){

	int buffSize = 128;
	int index = 0;
	int tempIndex = 0;
	int slot = 0;

	char* w;
	char* f;

	/*Add word to struct:*/
	w = calloc(buffSize, sizeof(char));
	while(word[index] != '\0')
	{
		if(isalnum(word[index])){
			w[tempIndex] = word[index];
			++tempIndex;
		}
		++index;
	}
	w[tempIndex] = '\0';
	w = realloc(w, (tempIndex + 1) * sizeof(char));

	/*Add which file the word came from:*/
	index = 0;
	tempIndex = 0;
	f = calloc(buffSize, sizeof(char));
	while(filename[index] != '\0')
	{
		if(isalnum(filename[index]) || filename[index] == '.'){
			f[tempIndex] = filename[index];
			++tempIndex;
		}
		++index;
	}
	f[tempIndex] = '\0';
	++tempIndex;
	f = realloc(f, tempIndex  * sizeof(char));


	/*Assign slot to thread*/
	pthread_mutex_lock(&(arr->lock));
	/*Check to see if we need to resize the word array:*/
	if(arr->numWords + 1 > arr->size){
		resizeWordArray(arr, debug);
	}
	/*get slot to put data into*/
	slot = arr->numWords;
	++arr->numWords;

	/*Put data into slot*/
	arr->words[slot].word = w;
	arr->words[slot].filename = f;

	pthread_mutex_unlock(&(arr->lock));
	
	/*print info to stdout*/
	if(debug == 1){
		char buffer[1024];
		sprintf(buffer,"Added \"%s\" at index %d.", w, slot);
		printMsg(buffer, pthread_self());
	}

	w = NULL;
	f = NULL;
}

/*This function prints out all the elements of a word array:*/
void printArray(wordArray* arr){
	char buffer[1024];
	int i = 0;
	for(;i < arr->numWords; ++i){
		sprintf(buffer, "%s (from \"%s\")", arr->words[i].word, arr->words[i].filename);
		printMsg(buffer, 0);
	}
}

/*This function reads in the input file, and adds words to the word array
//that was given as an argument to the function.*/
int readListArray(wordArray** arr, char* filename, char* directory){
	int buffSize = 64;
	char buffer[1024];
	FILE* fp;
	sprintf(buffer, "%s/%s", directory, filename);
	fp = fopen(buffer, "r");
	char* tempWord = calloc(buffSize, sizeof(char));
	while(fscanf(fp, "%s", tempWord) != EOF){
		addWordToArray(*arr, tempWord, filename, 1);
	}
	free(tempWord);
	tempWord = NULL;

	fclose(fp);
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

/*main thread that each thread runs*/
void* threadMain(void* tD){
	char buffer[1024];
	threadData* data = (threadData*) tD;
	sprintf(buffer, "Assigned \"%s\" to child thread %lu.", data->filename, pthread_self());
	printMsg(buffer, 0);
	readListArray(&(data->wA), data->filename, data->directory);
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
			(*data)[n-1].directory = calloc(strlen(directory) + 1, sizeof(char));
			strcpy((*data)[n-1].directory, directory);
			(*data)[n-1].filename = calloc((l + 1), sizeof(char));
			sprintf((*data)[n-1].filename, "%s", entry->d_name);
		}
	}

	closedir(p);
	return n;
}

/*print out the success message*/
void printSuccess(wordArray* arr, int numFiles){
	char msg[1024];
	sprintf(msg, "All done (successfully read %d words from %d files).", arr->numWords, numFiles);
	printMsg(msg, 0);
}


/*loop through each filename and create the threads*/
void createThreads(pthread_t** threads, pthread_attr_t* attr, threadData** data, int n){
	(*threads) = calloc(n, sizeof(pthread_t));
	int rc;
	int i = 0;
	for(; i < n; ++i){
		rc = pthread_create(&((*threads)[i]), attr, threadMain, (void*) &((*data)[i]));
		if(rc){
			fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", rc);
		}
	}
}

/*Main function*/
int main(int argc, char* argv[]){
	if(argc != 3){
		perror("Incorrect number of arguments\n");
		return -1;
	}

	wordArray directoryWords;
	initializeWordArray(&directoryWords, 1);
	threadData* directoryData = malloc(1*sizeof(threadData));
	pthread_t* threads = NULL;
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

	i = 0;
	for(; i < n; ++i){
		free(directoryData[i].filename);
		free(directoryData[i].directory);
		directoryData[i].wA = NULL;
		directoryData[i].filename = NULL;
	}

	free(directoryData);
	free(threads);

	directoryData = NULL;
	threads = NULL;

	pthread_exit(NULL);
	return 1;
}