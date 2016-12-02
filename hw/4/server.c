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
#define EXT_SUCCESS 0
#define EXT_FAILURE -1

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
	while(!isalnum((*inputString)[i]) && (*inputString)[i] != '-'){
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

/*This is a function to split the data from the control part of an input sequence. Used a little bit,
but mostly for debugging*/
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

/*Prints a pretty message*/
void printMsg(char* msg){
	printf("[child %d] %s", getpid(), msg);
}

/*Checks to make sure the filename is valid*/
int checkFilename(char* filename){
	char* extensions[4] = {".txt", ".bin", ".jpg", ".png"};
	int numExt = 4;
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
	i = 0;
	for(; i < numExt; ++i){
		if(strcmp(extensions[i], readExtension) == 0){
			if (i < 1){
				return 1;
			}
			else{
				return 2;
			}
		}
	}
	return EXT_FAILURE;
}

/*Initializes the Directory list to sort the files*/
dirEntList* initDirEntListElement(struct dirent* ent, dirEntList* next){
	dirEntList* temp = malloc(sizeof(dirEntList));
	temp->entry = ent;
	temp->next = next;
	return temp;
}

/*Inserts a file into the directory list*/
int insertFile(dirEntList* root, struct dirent* entry){
	dirEntList* tmp = root;
	while(tmp->next != NULL){
		if(strcmp(tmp->next->entry->d_name, entry->d_name) > 0){
			tmp->next = initDirEntListElement(entry, tmp->next);
			return EXT_SUCCESS;
		}
		tmp = tmp->next;
	}
	tmp->next = initDirEntListElement(entry, NULL);
	return EXT_SUCCESS;
}

/*Sends a pretty message to the client and prints to stdout*/
int sendMsg(threadData* td, char* msg, int data){
	int n;
	int l = strlen(msg);
	n = send(*(td->sock), msg, l, 0);
	fflush(NULL);
	if(n != l){
		perror("send() failed");
		return EXT_FAILURE;
	}
	else{
		char buf[2048];
		if(data == 0){
			sprintf(buf, "Sent %s", msg);
		}
		else{
			sprintf(buf, "Sent %s\n", msg);
		}
		printMsg(buf);
		return EXT_SUCCESS;
	}
}

/*Used by the LIST command, this sorts and lists all files*/
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
		if(checkFilename(entry->d_name) != EXT_FAILURE){
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
	sendMsg(td, buf, 0);
	return NULL;
}


/*Used by the STORE command, this stores a file*/
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
	int isText;
	if((isText = checkFilename(filename)) == EXT_FAILURE){
		return -1;
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

	if(isText == 1){
		fp = fopen(path, "w");
	}
	else if(isText == 2){
		fp = fopen(path, "wb");
	}

	fwrite(data, sizeof(char), size, fp);

	char msg[1024] = {'\0'};
	sprintf(msg, "Stored file \"%s\" (%d bytes)\n", filename, size);
	printMsg(msg);
	return EXT_SUCCESS;
}

/*Used by READ, this reads data from a file*/
int readDataFromFile(char** output, threadData* td, char* filename, int offset, int size){
	int isText;
	if((isText = checkFilename(filename)) == EXT_FAILURE){
		return -2;
	}
	char* directory = td->directory;
	(*output) = calloc(size, sizeof(char));
	DIR* p = NULL;
	struct dirent* entry = NULL;
	FILE* fp = NULL;
	char path[256];


	int foundFile = 0;
	p = opendir(directory);
	if(p == NULL){
		perror("OPENDIR");
		return -1;
	}
	while((entry = readdir(p))){
		if(strcmp(entry->d_name, filename) == 0)
		{
			foundFile = 1;
		}
	}
	if(foundFile == 0){
		return(-4);
	}

	sprintf(path, "%s%s", directory, filename);
	if(isText == 1){
		fp = fopen(path, "r");
	}
	else if(isText == 2){
		printf("OPENING AS BINARY\n");
		fp = fopen(path, "rb");
	}

	fseek(fp, offset, SEEK_SET);
	fread((*output), sizeof(char), size, fp);
	return EXT_SUCCESS;
}


/*This function recieves data which could be larger than the buffer size*/
int receiveData(threadData* td, char** output, char* input, int size){
	(*output) = calloc(size, sizeof(char));
	int i = 0;
	while(input[i] != '\n'){
		++i;
		if(i > BUFFER_SIZE){
			return EXT_FAILURE;
		}
	}
	int inputOffset = i + 1;
	i = 0;
	int j = 0;
	for(; (j + inputOffset) < BUFFER_SIZE; ++j){
		if(j == size){
			return EXT_SUCCESS;
		}
		(*output)[j] = input[j + inputOffset];
	}


	char* buffer = calloc(BUFFER_SIZE, sizeof(char));
	int offset = j;
	int n;
	/*int err = 0;*/

	do{
		/*READ MESSAGE HERE*/
		n = 0;
		n = recv(*(td->sock), buffer, BUFFER_SIZE, 0);

		if(n < 0){
			perror("recv() failed");
		}
		else{
			i = 0;
			for(; i < n; ++i){
				(*output)[offset+i] = buffer[i];
			}
			offset += n;
		}
	}while(n > 0);

	/*End Reading Message*/
	return EXT_SUCCESS;

}

/*Standard error messages go here for error catching*/
void checkError(threadData* td, int error){
	switch(error){
		case -1:
			sendMsg(td, "ERROR INVALID REQUEST\n", 0);
			break;
		case -2:
			sendMsg(td, "ERROR INVALID FILENAME\n", 0);
			break;
		case -3:
			sendMsg(td, "ERROR FILE EXISTS\n", 0);
			break;
		case -4:
			sendMsg(td, "ERROR NO SUCH FILE\n", 0);
			break;
		default:
			sendMsg(td, "ERROR\n", 0);
	}
}

#if 1
/*The main child process for client connections*/
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
			printMsg("Client disconnected\n");
		}
		else{
			
			char** out = splitData(&buffer);
			char msg[1024];
			sprintf(msg, "Received %s", out[0]);
			printMsg(msg);

			char* command = parseInput(&buffer, 4);
			if(strcmp(command, "STORE") == 0){
				char* filename = parseInput(&buffer, -1);
				char* sizeC = parseInput(&buffer, -1);
				int size = strtol(sizeC, NULL, 10);
				if(size < 1){
					checkError(td, -1);
				}
				else{
					char* fullData = NULL;
					receiveData(td, &fullData, buffer, size);

					err = writeDataToFile(td->directory, filename, fullData, size);
					if(err != EXT_SUCCESS){
						checkError(td, err);
					}
					else{
						sendMsg(td, "ACK\n", 0);
					}
				}
			}

			else if(strcmp(command, "READ") == 0){
				char* filename = parseInput(&buffer, -1);
				char* byteOC = parseInput(&buffer, -1);
				char* lengthC = parseInput(&buffer, -1);
				int byteOffset = strtol(byteOC, NULL, 10);
				int length = strtol(lengthC, NULL, 10);
				char* output;
				if(byteOffset < 0 || length < 1){
					sendMsg(td, "ERROR INVALID BYTE RANGE\n", 0);
				}
				else{
					err = readDataFromFile(&output, td, filename, byteOffset, length);
					if(err != EXT_SUCCESS){
						checkError(td, err);
					}
					else{
						char msg[1024] = {'\0'};
						sprintf(msg, "ACK %d\n", length);
						sendMsg(td, msg, 0);
						sendMsg(td, output, 1);
					}
				}
			}
			else if(strcmp(command, "LIST") == 0){
				listFiles(td);
			}
			else{
				sendMsg(td, "ERROR UNKNOWN COMMAND\n", 0);
			}

		}/*if load*/
		buffer = memset(buffer, '\0', BUFFER_SIZE);
	}while(n > 0);
	close(*(td->sock));
	exit(EXT_SUCCESS);
}
#endif

/*Main function, handles threads and program shutdown*/
int main(int argc, char* argv[]){
	if(argc < 2){
		perror("Please enter a port number\n");
		return EXT_FAILURE;
	}
	char* filepath = "./MazdaMX5Miata/";
	int sd = socket(PF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		perror("socket() failed");
		exit(EXT_FAILURE);
	}

	struct sockaddr_in server;

	server.sin_family = PF_INET;
	server.sin_addr.s_addr = INADDR_ANY;

	/*//////////////////////////*/
	unsigned short port = strtol(argv[1], NULL, 10);
	/*//////////////////////////*/

	server.sin_port = htons(port);
	int len = sizeof(server);

	if(bind(sd, (struct sockaddr*)&server, len) < 0){
		perror("bind()failed");
		exit(EXT_FAILURE);
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
			return EXT_FAILURE;
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
	return EXT_SUCCESS;
}
