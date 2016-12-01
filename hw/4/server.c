#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include <pthread.h>

#include <string.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

typedef struct threadData threadData;
typedef struct dirEntList dirEntList;

struct dirEntList{
	struct dirent* entry;
	dirEntList* next;
};

struct threadData{
	int* sock;
	struct sockaddr_in* client;
	char* directory;
};

void debugPrintBuffer(char* buffer){
	int i = 0;
	for(; i < BUFFER_SIZE; ++i){
		printf("|%c", buffer[i]);
	}
	printf("\n");
}

/*This function takes in a string, and parses it by whitespace or by /n token
much like the strtok function, but safe for this application.*/
char* parseInput(char** inputString, int sizeLimit){
	int buffSize = BUFFER_SIZE;
	if(sizeLimit < 0){
		sizeLimit = buffSize;
	}
	char* output = calloc(buffSize, sizeof(char));
	int i = 0;
	while(!isalnum((*inputString)[i])){
		(*inputString)[i] = '\0';
		++i;
	}
	int j = 0;
	while((*inputString)[i] != '\n' && (*inputString)[i] != ' '){
		output[j] = (*inputString)[i];
		(*inputString)[i] = '\0';
		if(i > sizeLimit){
			/*TODO: ERROR IF LENGTH TOO LONG*/
			break;
		}
		++i;
		++j;
	}
	output[i + 1] = '\0';
	return output;
}

char** splitData(char** inputString){
	char** data = calloc(2, sizeof(char*));
	data[0] = calloc(BUFFER_SIZE, sizeof(char));
	data[1] = calloc(BUFFER_SIZE, sizeof(char));
	int split = 0;
	int i = 0;
	for(; i < BUFFER_SIZE; ++i){
		if(split == 0){
			if(inputString[0][i] == '\n'){
				split = i;
			}
			data[0][i] = (*inputString)[i];
		}
		else{
			data[1][i - (split + 1)] = (*inputString)[i];
		}
	}

	return data;
}

char* readData(char** inputString, int size){
	int i = 0;
	/*
	for(; i < BUFFER_SIZE; ++i){
		printf("%c", (*inputString)[i]);
	}
	printf("\n");
	i = 0;
	*/
	int j = 0;
	int buffSize = BUFFER_SIZE + 1;
	while((*inputString)[i] != '\n'){
		(*inputString)[i] = '\0';
		++i;
	} 

	printf("%d\n", i);
	char* output = calloc(buffSize, sizeof(char));
	for(; j < size; ++j){
		output[j] = (*inputString)[i];
		++i;
	}
	/*output[j + 1] = '\0';*/
	return output;
}

void printMsg(char* msg){
	printf("[child %d] %s", getpid(), msg);
}

int checkFilename(char* filename){
	/*check to make sure the given filename is valid*/
	char* extension = ".txt";
	char* readExtension = calloc(64, sizeof(char));
	int i = 0;
	while(filename[i] != '\0'){
		++i;
	}

	i = i - 4;
	int j = 0;
	for(; j < 4; ++j){
		readExtension[j] = filename[i + j];
	}
	if(strcmp(extension, readExtension) != 0){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

dirEntList* initDirEntListElement(struct dirent* ent, dirEntList* next){
	dirEntList* temp = malloc(sizeof(dirEntList));
	temp->entry = ent;
	temp->next = next;
	return temp;
}

int insertFile(dirEntList* root, struct dirent* entry){
	dirEntList* tmp = root;
	while(tmp->next != NULL){
		if(strcmp(tmp->next->entry->d_name, entry->d_name) > 0){
			tmp->next = initDirEntListElement(entry, tmp->next);
			return EXIT_SUCCESS;
		}
		tmp = tmp->next;
	}
	tmp->next = initDirEntListElement(entry, NULL);
	return EXIT_SUCCESS;
}

int sendMsg(threadData* td, char* msg){
	int n;
	int l = strlen(msg);
	n = send(*(td->sock), msg, l, 0);
	fflush(NULL);
	if(n != l){
		perror("send() failed");
		return EXIT_FAILURE;
	}
	else{
		char buf[2048];
		sprintf(buf, "Sent %s", msg);
		printMsg(buf);
		return EXIT_SUCCESS;
	}
}

char* listFiles(threadData* td){
	char* directory = td->directory;
	int numFiles = 0;
	DIR* p = NULL;
	dirEntList* fileList = initDirEntListElement(NULL, NULL);
	struct dirent* entry = NULL;
	p = opendir(directory);
	if(p == NULL){
		perror("OPENDIR");
		return NULL;
	}
	while((entry = readdir(p))){
		if(checkFilename(entry->d_name) == EXIT_SUCCESS){
			insertFile(fileList, entry);
			++numFiles;
		}
	}
	dirEntList* tmp = fileList;
	tmp = tmp->next;
	char buf[1024] = {'\0'};
	sprintf(buf, "%d", numFiles);
	while(tmp != NULL){
		sprintf(buf, "%s %s", buf, tmp->entry->d_name);
		tmp = tmp->next;
	}
	sprintf(buf, "%s\n", buf);
	sendMsg(td, buf);
	return NULL;
}

int writeDataToFile(char* directory, char* filename, char* data, int size){
	DIR* p = NULL;
	struct dirent* entry = NULL;

	FILE* fp = NULL;
	char path[256];

	p = opendir(directory);
	if(p == NULL){
		perror("OPENDIR");
		return -1;
	}

	/*check the filename*/
	if(checkFilename(filename) == EXIT_FAILURE){
		return -2;
	}


	/*Check if the file already exists*/
	while((entry = readdir(p))){
		if(strcmp(entry->d_name, filename) == 0)
		{
			return(-3);
		}
	}

	/*Write to the file*/
	sprintf(path, "%s%s", directory, filename);
	fp = fopen(path, "w+");
	int i = 0;
	for(; i < size; ++i){
		fputc(data[i], fp);
	}
	return EXIT_SUCCESS;
}

int readDataFromFile(char** output, threadData* td, char* filename, int offset, int size){
	char* directory = td->directory;
	(*output) = calloc(size, sizeof(char));
	FILE* fp = NULL;
	char path[256];

	sprintf(path, "%s%s", directory, filename);
	fp = fopen(path, "r");
	char c;
	int i = 0;
	while((c = fgetc(fp)) != EOF){
		if(i >= offset && (i - offset) < size){
			(*output)[i-offset] = c;
		}
		++i;
	}
	return EXIT_SUCCESS;
}

int clientConnection(void* data){
	threadData* td = (threadData*)data;
	char* buffer = calloc(BUFFER_SIZE, sizeof(char));

	int n;
	int err = 0;

	do{
		n = recv(*(td->sock), buffer, BUFFER_SIZE, 0);

		if(n < 0){
			perror("recv() failed");
		}
		else if(n == 0){
			printf("CHILD %d: Rcvd 0 from recv(); closing socket\n", getpid());
		}
		else{

			char** outs = splitData(&buffer);
			char pBuf[BUFFER_SIZE] = {'\0'};
			sprintf(pBuf, "Received %s", outs[0]);
			printMsg(pBuf);
			/*parse command TODO: USE STRTOK?*/
			char* command = parseInput(&outs[0], 4);
			/*printf("COMMAND %s\n", command);*/
			if(strcmp(command, "STORE") == 0){
				char* filename = parseInput(&outs[0], -1);
				printf("Filename: %s\n", filename);
				char* sizeC = parseInput(&outs[0], -1);
				int size = strtol(sizeC, NULL, 10);
				printf("Size: %d\n", size);
				printf("Data: %s\n", outs[1]);
				err = writeDataToFile(td->directory, filename, outs[1], size);
				if(err == -2){
					n = send(*(td->sock), "ERROR INVALID FILENAME\n", 17, 0);
					fflush(NULL);
					if(n != 17){
						perror("send() failed");
					}
				}
				else if(err == -3){
					n = send(*(td->sock), "ERROR FILE EXISTS\n", 18, 0);
					fflush(NULL);
					if(n != 18){
						perror("send() failed");
					}
				}
				else{
					n = send(*(td->sock), "ACK\n", 4, 0);
					fflush(NULL);
					if(n != 4){
						perror("send() failed");
					}
				}
			}

			else if(strcmp(command, "READ") == 0){
				char* filename = parseInput(&outs[0], -1);
				char* byteOC = parseInput(&outs[0], -1);
				char* lengthC = parseInput(&outs[0], -1);
				int byteOffset = strtol(byteOC, NULL, 10);
				int length = strtol(lengthC, NULL, 10);
				printf("FILENAME: %s\n", filename);
				printf("Byte Offset %d\n", byteOffset);
				printf("Length: %d\n", length);
				char* output;
				err = readDataFromFile(&output, td, filename, byteOffset, length);
				printf("%s\n", output);
			}
			else if(strcmp(command, "LIST") == 0){
				listFiles(td);
			}
			else{
				printf("UNKNOWN COMMAND\n");
			}

		}/*if load*/
		buffer = memset(buffer, '\0', BUFFER_SIZE);
	}while(n > 0);
	printf("CHILD %d: Bye!\n", getpid());
	close(*(td->sock));
	exit(EXIT_SUCCESS);
}

int main(){
	char* filepath = "./CrispyChickenStrips/";
	int sd = socket(PF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server;

	server.sin_family = PF_INET;
	server.sin_addr.s_addr = INADDR_ANY;

	/*//////////////////////////*/
	unsigned short port = 8173;
	/*//////////////////////////*/

	server.sin_port = htons(port);
	int len = sizeof(server);

	if(bind(sd, (struct sockaddr*)&server, len) < 0){
		perror("bind()failed");
		exit(EXIT_FAILURE);
	}

	listen(sd, 5);

	struct sockaddr_in client;
	int fromlen = sizeof(client);

	threadData data;
	data.directory = filepath;
	data.client = &client;

	int pid;

	mkdir(filepath, S_IRWXU | S_IRWXG | S_IROTH |S_IXOTH);
	while(1){
		int newsock = accept(sd, (struct sockaddr*)&client, (socklen_t*)&fromlen);
		printf("Recieved incoming connection from: %s\n", inet_ntoa(client.sin_addr));
		data.sock = &newsock;

		/*Change the following to use pthreads*/
		pid = fork();
		if(pid < 0){
			perror("fork() failed");
			return EXIT_FAILURE;
		}
		else if(pid == 0){
			#if 0
			sleep(10);
			#endif
			clientConnection((void*)&data);

		}
		else{
			close(newsock);
		}

	}

	close(sd);
	return EXIT_SUCCESS;
}
