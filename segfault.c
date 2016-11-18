#include <stdio.h>
#include <stdlib.h>

int main(){
	int* a = NULL;
	printf("Triggering segfault:\n");
	*a = 1;
}