#include <stdio.h>
#include <stdlib.h>

typedef struct wordElement wordElement;
typedef struct nameArray nameArray;

struct nameArray{
	char** names;
	int length;
};

struct wordElement{
	char* word;
	wordElement* next;
	wordElement* previous;
	int length;
};

wordElement* initWordElement(wordElement* prev){
	wordElement w;
	w.next = NULL;
	w.previous = prev;
	w.length = 0;
}

void printArray(nameArray* arr){
	for(int i = 0; i < arr->length; ++i){
		printf("%s\n", arr->names[i]);
	}
}

int readList(wordElement* w){
	const char* filename = "nameFile.txt";
	FILE* fp;

	w->word = calloc(255, sizeof(char));
	fp = fopen(filename, "r");
	while(fgets(w->word, 255, fp)){
		int len = 0;
		while(w->word[len] != NULL && len < 255){
			++len;
		}
		w->length = len;
		w->word = realloc(w->word, len);
		//printWord(w);
		w->next = initWordElement(w);
		w = w->next;
	}

	return 1;
	//create space for next word:
}

void initializeNameArrayElement(nameArray* arr){

	printf("Length: %d\n", arr->length);
	arr->names = realloc(arr->names, arr->length + 1);
	arr->names[arr->length] = calloc(255, sizeof(char));
	++(arr->length);
}

int readListArray(nameArray* arr){
	const char* filename = "nameFile.txt";
	FILE* fp;
	fp = fopen(filename, "r");
	int index = 0;
	
	while(fgets(arr->names[index], 255, fp)){
		int len = 0;
		while(arr->names[index][len] != NULL && len < 255){
			++len;
		}
		printf("INDEX: %d\n", index);
		arr->names[index] = realloc(arr->names[index], len);
		initializeNameArrayElement(arr);
		++index;
	}

	return 1;
	//create space for next word:
}

int sortList(wordElement* w){
	
}
int main(){
	//INITIALIZE:
	nameArray storedNames;
	storedNames.names = calloc(1, sizeof(char*));
	storedNames.names[0] = calloc(255, sizeof(char));
	storedNames.length = 1;
	//READ:
	readListArray(&storedNames);
	//printArray(&storedNames);
	//SORT:
}