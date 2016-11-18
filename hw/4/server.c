#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

typedef struct threadData threadData;

struct threadData{
	int* sock;
	struct sockaddr_in* client;
};


int clientConnection(void* data){
	threadData* td = (threadData*)data;

	int n;
	char buffer[BUFFER_SIZE];
	do{
		n = recv(*(td->sock), buffer, BUFFER_SIZE, 0);

		if(n < 0){
			perror("recv() failed");
		}
		else if(n == 0){
			printf("CHILD %d: Rcvd 0 from recv(); closing socket\n", getpid());
		}
		else{
			buffer[n] = '\0';
			
			printf("CHILD %d: Rcvd message from %s: %s\n", 
								getpid(), inet_ntoa((struct in_addr)td->client->sin_addr), buffer);
			n = send(*(td->sock), "ACK\n", 4, 0);
			fflush(NULL);
			if(n != 4){
				perror("send() failed");
			}
		}
	}while(n > 0);
	printf("CHILD %d: Bye!\n", getpid());
	close(*(td->sock));
	exit(EXIT_SUCCESS);
}

int main(){
	int sd = socket(PF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server;

	server.sin_family = PF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	unsigned short port = 8172;

	server.sin_port = htons(port);
	int len = sizeof(server);

	if(bind(sd, (struct sockaddr*)&server, len) < 0){
		perror("binde()fialed");
		exit(EXIT_FAILURE);
	}

	listen(sd, 5);
	printf("Started server; istening to port: %d\n", ntohs(server.sin_port));

	struct sockaddr_in client;
	int fromlen = sizeof(client);

	threadData data;
	data.client = &client;

	int pid;

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
