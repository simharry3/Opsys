
//////////////////////////////////////////////
//  	  OPERATING SYSTEMS HOMEWORK #1:
//		Strings and Memory Allocation in C
//				09/07/2016
//============================================
//				WRITTEN BY:
//		     CLAYTON RAYMENT
//			    661133772	    
//////////////////////////////////////////////


/*Number of words that caused out-of-memory case: 108000000+ (text editor crashed)*/



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct wordArray wordArray;

//structure which contains the array of words and relevant information
struct wordArray{
	char** words;
	int numWords;
	int size;
};

//This function initializes a word array with the
//indicated starting size, which can be changed:
void initializeWordArray(wordArray* arr, int debug){
	int initialSize = 8;
	if(debug){
		printf("Allocated initial array of 8 character pointers.\n");
	}
	arr->words = calloc(initialSize, sizeof(char*));
	arr->size = initialSize;
	arr->numWords = 0;
}

//This function destroys a given word array:
void destroyWordArray(wordArray* arr){
	int i = 0;
	for(;i < arr->numWords; ++i){
		free(arr->words[i]);
	}
	free(arr->words);
	arr = NULL;	
}

//This function resizes a given word array to twice its size:
void resizeWordArray(wordArray* arr, int debug){
	arr->words = realloc(arr->words, arr->size * sizeof(char*) * 2);
	arr->size = arr->size * 2;
	if(debug){
		printf("Re-allocated array of %d character pointers.\n", arr->size);
	}
}

//This function takes in a character pointer and adds that word to the word array:
void addWordToArray(wordArray* arr, char* word, int debug){

	int buffSize = 128;
	int index = 0;
	int tempIndex = 0;

	//Check to see if we need to resize the word array:
	if(arr->numWords + 1 > arr->size){
		resizeWordArray(arr, debug);
	}

	arr->words[arr->numWords] = calloc(buffSize, sizeof(char));
	while(word[index] != '\0')
	{
		if(isalnum(word[index])){
			arr->words[arr->numWords][tempIndex] = word[index];
			++tempIndex;
		}
		++index;
	}
	arr->words[arr->numWords] = realloc(arr->words[arr->numWords], tempIndex  * sizeof(char));
	//printf("%d: %s\n", arr->numWords, arr->words[arr->numWords]);

	++arr->numWords;
}

//This function prints out all the elements of a word array:
void printArray(wordArray* arr){
	int i = 0;
	for(;i < arr->numWords; ++i){
		printf("%s\n", arr->words[i]);
	}
}

//This function reads in the input file, and adds words to the word array
//that was given as an argument to the function.
int readListArray(wordArray* arr, const char* filename){
	int buffSize = 10;
	FILE* fp;
	fp = fopen(filename, "r");
	char* tempWord = calloc(buffSize, sizeof(char));
	while(fscanf(fp, "%s", tempWord) != EOF){
		addWordToArray(arr, tempWord, 1);
	}
	printf("All done (successfully read %d words).\n", arr->numWords);
	return 1;
}

//This search function goes through each word in the array and uses strstr to
//find a substring. If it matches, that word is placed into a seperate word array
//containing only words that have matching substrings:
void substringSearch(wordArray* arr, wordArray* matches, char* substring){
	int i = 0;
	for(;i < arr->numWords; ++i){
		if(strstr(arr->words[i], substring) != NULL){
			addWordToArray(matches, arr->words[i], 0);
		}
	}
}

//Main function
int main(int argc, char* argv[]){
	if(argc != 3){
		fprintf(stderr, "Incorrect number of arguments\n");
		return -1;
	}

	wordArray storedWords;
	initializeWordArray(&storedWords, 1);
	readListArray(&storedWords, argv[1]);

	wordArray returnedWords;
	initializeWordArray(&returnedWords, 0);
	substringSearch(&storedWords, &returnedWords, argv[2]);
	
	printf("Words containing substring \"%s\" are:\n", argv[2]);
	printArray(&returnedWords);

	destroyWordArray(&storedWords);
	destroyWordArray(&returnedWords);

	return 1;
}