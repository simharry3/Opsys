#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

void childProcess(int file){
	printf("Child process %d created\n", getpid());
	char* data = "TEST DATA\n";
	write(file, data, strlen(data));
}


void parentProcess(int file){
	char* buff[1025];
	int n;
	printf("Process %d created child process\n", getpid());
	n = read(file, buff, 1024);
	buff[n] = 0;
	printf("%s\n", buff);
}

int main(int argv, char* argc[]){
	pid_t pid;
	int myPipe[2];
	char* data = "TEST DATA PACKET SUCCESSFUL\n";
	pipe(myPipe);
	pid = fork();

	if(pid == 0){
		close(myPipe[1]);
		childProcess(myPipe[0]);
	}
	else{
		close(myPipe[0]);
		parentProcess(myPipe[1]);
	}

	return 1;
}