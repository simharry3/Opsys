OBJS = memDevice.o MemoryManager.o process.o pageReference.o
CC = g++
CFLAGS = -Wall -std=c++98 -c
LFLAGS = -Wall

p2: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o p2

memDevice.o: memDevice.cpp memDevice.h
	$(CC) $(CFLAGS) memDevice.cpp

MemoryManager.o: MemoryManager.cpp memDevice.h
	$(CC) $(CFLAGS) MemoryManager.cpp

process.o: process.cpp process.h
	$(CC) $(CFLAGS) process.cpp

pageReference.o: pageReference.cpp pageReference.h
	$(CC) $(CFLAGS) pageReference.cpp
