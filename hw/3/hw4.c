
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
	wordStruct* words;
	int numWords;
	int size;
};

void printMsg(char* message){
	unsigned int tid = pthread_self();
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
	if(debug){
		printMsg("Allocated initial array of 8 character pointers.");
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
	arr->words = realloc(arr->words, arr->size * sizeof(wordStruct) * 2);
	arr->size = arr->size * 2;
	if(debug){
		char* msg = malloc(256 * sizeof(char));
		char* num = malloc(16 * sizeof(char));
		strcat(msg, "Re-allocated array of ");
		sprintf(num,"%d", arr->size);
		strcat(msg,  num);
		strcat(msg, " character pointers.");
		msg = realloc(msg, strlen(msg));
		printMsg(msg);
		free(msg);
		free(num);
	}
}

/*This function takes in a character pointer and adds that word to the word array:*/
void addWordToArray(wordArray* arr, char* word, char* filename, int debug){

	int buffSize = 128;
	int index = 0;
	int tempIndex = 0;

	/*Check to see if we need to resize the word array:*/
	if(arr->numWords + 1 > arr->size){
		resizeWordArray(arr, debug);
	}

	/*Add word to struct:*/
	arr->words[arr->numWords].word = calloc(buffSize, sizeof(char));
	while(word[index] != '\0')
	{
		if(isalnum(word[index])){
			arr->words[arr->numWords].word[tempIndex] = word[index];
			++tempIndex;
		}
		++index;
	}
	arr->words[arr->numWords].word = realloc(arr->words[arr->numWords].word, tempIndex  * sizeof(char));

	/*Add which file the word came from:*/
	index = 0;
	tempIndex = 0;
	arr->words[arr->numWords].filename = calloc(buffSize, sizeof(char));
	while(filename[index] != '\0')
	{
		if(isalnum(filename[index]) || filename[index] == '.'){
			arr->words[arr->numWords].filename[tempIndex] = filename[index];
			++tempIndex;
		}
		++index;
	}
	arr->words[arr->numWords].filename = realloc(arr->words[arr->numWords].filename, tempIndex  * sizeof(char));
	/*printf("%d: %s\n", arr->numWords, arr->words[arr->numWords]);*/

	++arr->numWords;
}

/*This function prints out all the elements of a word array:*/
void printArray(wordArray* arr){
	int i = 0;
	for(;i < arr->numWords; ++i){
		printf("%s  ---  %s\n", arr->words[i].word, arr->words[i].filename);
	}
}

/*This function reads in the input file, and adds words to the word array
//that was given as an argument to the function.*/
int readListArray(wordArray* arr, char* filename){
	int buffSize = 10;
	FILE* fp;
	fp = fopen(filename, "r");
	char* tempWord = calloc(buffSize, sizeof(char));
	while(fscanf(fp, "%s", tempWord) != EOF){
		addWordToArray(arr, tempWord, filename, 1);
	}

	char* msg = malloc(256 * sizeof(char));
	char* num = malloc(16 * sizeof(char));
	strcat(msg, "All done (successfully read ");
	sprintf(num,"%d", arr->numWords);
	strcat(msg,  num);
	strcat(msg, " words)");
	msg = realloc(msg, strlen(msg));
	printMsg(msg);
	return 1;
}

/*This search function goes through each word in the array and uses strstr to
//find a substring. If it matches, that word is placed into a seperate word array
//containing only words that have matching substrings:*/
void substringSearch(wordArray* arr, wordArray* matches, char* substring){
	int i = 0;
	for(;i < arr->numWords; ++i){
		if(strstr(arr->words[i].word, substring) != NULL){
			addWordToArray(matches, arr->words[i].word, arr->words[i].filename, 0);
		}
	}
}

void* threadMain(void* tD){
	threadData* data = (threadData*) tD;
	printMsg(data->filename);
	readListArray(data->wA, data->filename);
	pthread_exit(NULL);
}

/*Search through directory, and initialize a thread for each file*/
int searchDirectory(char* directory, char* extension, wordArray* dW, threadData** data){

	DIR* p;
	struct dirent* entry;

	int n = 0;

	p = opendir(directory);
	if(p == NULL){
		perror("OPENDIR");
		return -1;
	}

	while((entry = readdir(p))){
		int l = strlen(entry->d_name);
		if(strncmp(entry->d_name + l - strlen(extension), extension, strlen(extension)) == 0){
			char* fn = entry->d_name;
			puts(fn);
			++n;
			(*data) = realloc((*data), n * sizeof(threadData));
			(*data)[n-1].wA = dW;
			(*data)[n-1].filename = realloc((*data)[n-1].filename, (strlen(entry->d_name) + 1) * sizeof(char));
			int i = 0;
			for(; i < strlen(entry->d_name); ++i){
				(*data)[n-1].filename[i] = entry->d_name[i];
			}
			(*data)[n-1].filename[i] = '\0';
		}
	}

	closedir(p);
	return n;
}

void createThreads(pthread_t** threads, pthread_attr_t* attr, threadData** data, int n){
	(*threads) = realloc((*threads), n * sizeof(pthread_t));
	int rc;
	int i = 0;
	for(; i < n; ++i){
		rc = pthread_create(&((*threads)[i]), attr, threadMain, (void*) &((*data)[i]));
		if(rc){
			printf("ERROR: return code from pthread_create() is %d\n", rc);
		}
	}
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

	printf("FOUND = %s\n", directoryData[0].filename);
	/*printf("FOUND = %s\n", directoryData[1].filename);*/

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	createThreads(&threads, &attr, &directoryData, n);
	/*wordArray storedWords;
	initializeWordArray(&storedWords, 1);
	readListArray(&storedWords, argv[1]);

	wordArray returnedWords;
	initializeWordArray(&returnedWords, 0);
	substringSearch(&storedWords, &returnedWords, argv[2]);
	
	printf("Words containing substring \"%s\" are:\n", argv[2]);
	printArray(&returnedWords);

	destroyWordArray(&storedWords);
	destroyWordArray(&returnedWords);*/

	int i = 0;
	for(; i < n; ++i){
		pthread_join(threads[i], NULL);
	}
	printf("FOUND = %s\n", directoryData[0].filename);
	/*printf("FOUND = %s\n", directoryData[1].filename);*/
	pthread_exit(NULL);
	return 1;
}