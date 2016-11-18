#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FUNC_SUCCESS 1
#define FUNC_FAILURE -1


typedef struct opStrut opStruct;

struct opStruct{
	char* operator; /*This contains the operation to be performed 0 means no operation*/
	char** data; /*This contains the string data*/
	int num;
};

/*Operations Struct Utilities: */
void initializeOpStruct(struct opStruct** op){
	*op = malloc(sizeof(struct opStruct));
	(*op)->operator = malloc(sizeof(char));
	(*op)->data = NULL;
	(*op)->num = 0;
}

void insertData(struct opStruct** op, char* dat, int len){
	int num = (*op)->num;
	(*op)->data = realloc((*op)->data, (num + 1) * sizeof(char*));
	(*op)->data[num] = dat;
	++(*op)->num;
}



/*String Parsing Utilities:*/
char* readOperationFile(const char* filename){
	FILE* fp;
	fp = fopen(filename, "r");
	if(fp == NULL){
		printf("Error: File pointer is NULL.\n");
		fflush(NULL);
		exit(EXIT_FAILURE);
	}
	size_t length = 0;
	int read;
	char* line = NULL;
	while((read = getline(&line, &length, fp)) != -1){
		if(line[0] != '#'){
			return line;
		}
	}

	exit(EXIT_FAILURE);
}

int checkParenthesis(char character){
	if(character == '('){
		return 1;
	}
	else if(character == ')'){
		return -1;
	}
	else{
		return 0;
	}
}

void checkOpStruct(struct opStruct* op){
	int pid = getpid();
	if(strstr("+-*/\0", op->operator) == NULL){
		printf("PID %d: ERROR: unknown \"%s\" operator; exiting\n", pid, op->operator);
		fflush(NULL);
		exit(EXIT_FAILURE);
	}
}

struct opStruct* parseString(char* operation){
	struct opStruct* op = NULL;
	initializeOpStruct(&op);
	int i = 0;
	int opFound = 0;
	int parValue = 0;

	/*error checking*/
	if(operation[0] != '('){
		return NULL;
	}
	++i;


	/*scan the input string and fill the operation struct*/
	for(; operation[i] != '\0'; ++i){
		if(operation[i] != ' ' && opFound == 0){
			int index = 0;
			char* tempOp = malloc(30*sizeof(char));
			while(operation[i] != ' '){
				tempOp[index] = operation[i];
				++i;
				++index;
			}
			tempOp[index + 1] = '\0';
			tempOp = realloc(tempOp, (index + 1)*sizeof(char));
			(op->operator) = tempOp;
			opFound = 1;
		}
		parValue += checkParenthesis(operation[i]);
		if(parValue > 1){
			++i;
			int index = 0;
			char* temp = malloc(30*sizeof(char));
			temp[0] = '(';
			++index;
			while(parValue > 1){
				parValue += checkParenthesis(operation[i]);
				temp[index] = operation[i];
				++i;
				++index;
			}
			temp[index + 1] = '\0';
			temp = realloc(temp, (index + 1)*sizeof(char));
			/*printf("Operation: %s\n", temp);*/
			insertData(&op, temp, index);
		}

		if(isalnum(operation[i])){ /*scan in numbers, including multi-digit numbers*/
			int index = 0;
			char* temp = malloc(30*sizeof(char));
			if(operation[i-1] == '-' && opFound == 1){
				temp[index] = '-';
				++index;
			}
			while(isalnum(operation[i])){
				temp[index] = operation[i];
				++i;
				++index;
			}
			temp[index + 1] = '\0';
			temp = realloc(temp, (index + 1)*sizeof(char));
			/*printf("Number: %s\n", temp);*/
			insertData(&op, temp, index);
		}

	}

	return op;
}

int calculateString(char* input);

int processString(char* input){
	int pid = getpid();
	printf("PID %d: My expression is \"%s\"\n", pid, input);
	fflush(NULL);
	int retVal;
	if(isalnum(input[0]) || input[0] == '-'){
		retVal = atoi(input);
		printf("PID %d: Sending \"%d\" on pipe to parent\n", pid, retVal);
		fflush(NULL);
	}
	else{
		retVal = calculateString(input);
		printf("PID %d: Processed \"%s\"; sending \"%d\" on pipe to parent\n", pid, input, retVal);
		fflush(NULL);
	}
	/*printf("RET VAL: %d\n", retVal);*/
	return retVal;
}

int calculateString(char* input){
	int pid = getpid();
	
	struct opStruct* op;
	op = parseString(input);
	checkOpStruct(op);

	char oper = *(op->operator);
	printf("PID %d: Starting \"%c\" operation\n", pid, oper);
	fflush(NULL);

	int** pipes = calloc(op->num, sizeof(char*));
	int* PIDs = calloc(op->num, sizeof(int));

	int j = 0;
	int* data = calloc(op->num, sizeof(int));
	int result;
	for(; j < op->num; ++j){
		/*CHECK FOR ZERO DIVISION*/
		if(*op->operator == '/' && j > 0){
			if(*op->data[j] == '0'){
				printf("PID %d: ERROR: division by zero is not allowed; exiting\n", pid);
				fflush(NULL);
				exit(EXIT_FAILURE);
			}
		}
		/*create new pipe:*/
		pipes[j] = calloc(2, sizeof(int));
		pipe(pipes[j]); 
		/*printf("===========FORK==========\n");*/
		PIDs[j] = fork();
		if(PIDs[j] < 0){
			exit(EXIT_FAILURE);
		}
		/*close(pipes[j][0]);*/
		/*IF CHILD THREAD, BEGIN COMPUTATION, OTHERWISE PARENT KEEPS CREATING NEW CHILDREN:*/
		if(PIDs[j] == 0){
			pid = getpid();
			int temp = processString(op->data[j]);			
			/*printf("PIPE VALUE: %d \n", temp);*/
			write(pipes[j][1], &temp, sizeof(int));
			/*printf("//////////////END FORK////////////////\n");*/
			exit(EXIT_SUCCESS);
		}
		/*int parentValue;
		read(pipes[j][0], &parentValue, sizeof(int));*/
	}
	/*check to see if there are enough arguments.
	NOTE: this is really dumb, should go in the parsing string function at the end, but it doesn't match the output unless put here.*/
	/*check to see if there are at least 2 operands:*/
	if(op->num < 2){
		printf("ERROR: not enough operands; exiting\n");
		fflush(NULL);
		exit(EXIT_FAILURE);
	}

	int i = 0;
	int status;
	for(; i < op->num; ++i){
		waitpid(PIDs[i],&status,0);
		if(status != 0){
			printf("PID %d: child %d terminated with non-zero exit status %d\n", pid, PIDs[i], status);
			fflush(NULL);
		}
		read(pipes[i][0], &data[i], sizeof(int));
	}

	/*wait for pipes to return data here*/
	result = data[0];
	/*printf("DATA[0] = %d\n", data[0]);*/
	/*printf("Operator %c\n", oper);*/
	j = 1;
	for(; j < op->num; ++j){
		/*printf("DATA[%d] = %d\n", j, data[j]);*/
		if(oper == '+'){
			result = result + data[j];
		}
		else if(oper == '-'){
			result = result - data[j];
		}
		else if(oper == '*'){
			result = result * data[j];
		}
		else if(oper == '/'){
			result = result / data[j];
		}
	}
	/*printf("RESULT: %d\n", result);*/
	return result;
}

int main(int argc, char* argv[]){
	if(argc < 2){
		perror("ERROR: Invalid arguments\nUSAGE: ./a.out <input-file>\n");
		exit(EXIT_FAILURE);
	}
	const char* filename = argv[1];

	int pid = getpid();
	char* input = readOperationFile(filename);
	printf("PID %d: My expression is \"%s\" \n", pid, input);
	fflush(NULL);
	int ans = calculateString(input);
	printf("PID %d: Processed \"%s\"; final answer is \"%d\"\n", pid, input, ans);
	fflush(NULL);

	return 1;
}
