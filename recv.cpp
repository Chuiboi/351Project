#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<iostream>
#include "msg.h"    /* For the message struct */


/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";
void ctrlCSignal(int);
void cleanUp(const int&, const int&, void*);

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	std::cout << "entered init\n";
	/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		 3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V object (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	std::cout << "1\n";
	key_t key;
	std::cout << "2\n";
	key = ftok("keyfile.txt",'a');
	std::cout << "3\n";
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644| IPC_CREAT); //creating or checking if the shared memory location exists
	std::cout << "4\n";
	msqid = msgget(key, 0666| IPC_CREAT); //creating or checking if the message queue exists
	std::cout << "5\n";
	sharedMemPtr = shmat(shmid, (void *)0, 0);	//assigning the location of shared memory to a void pointer

	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	/* TODO: Attach to the shared memory */
	/* TODO: Create a message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	std::cout << "I have finished init\n";
}


/**
 * The main loop
 */
void mainLoop()
{
	//signal(SIGINT, ctrlCSignal);
	std::cout << "I have entered mainLoop\n";
	/* Declare a message */
	message msg;
	/* The size of the mesage */
	int msgSize = 0;
	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");

	/* Error checks */
	if(!fp)
	{
		perror("fopen");
		exit(-1);
	}

    /* TODO: Receive the message and get the message size. The message will 
     * contain regular information. The message will be of SENDER_DATA_TYPE
     * (the macro SENDER_DATA_TYPE is defined in msg.h).  If the size field
     * of the message is not 0, then we copy that many bytes from the shared
     * memory region to the file. Otherwise, if 0, then we close the file and
     * exit.
     *
     * NOTE: the received file will always be saved into the file called
     * "recvfile"
     */
	for(;;)
	{
		std::cout << "I have entered the for\n";
		msgrcv(msqid, &msg, sizeof(struct message)-sizeof(long), 1, 0);
		std::cout << msg.mtype << " " << msg.size << " message recieved\n";
		msgSize = msg.size;
	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */

		while(msgSize != 0)
		{
			std::cout << "I have entered the while\n";
		/* If the sender is not telling us that we are done, then get to work */
			if(msgSize != 0)
			{
			/* Save the shared memory to file */
				if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
				{
					perror("fwrite");
				}

			/* TODO: Tell the sender that we are ready for the next file chunk. 
 			 * I.e. send a message of type RECV_DONE_TYPE (the value of size field
 			 * does not matter in this case). 
 			 */
				msg.mtype = RECV_DONE_TYPE;
				msg.size = 0;
				msgsnd(msqid, &msg, sizeof(struct message)-sizeof(long),0);
			}
		/* We are done */
			else
			{
			/* Close the file */
				std::cout << "I am ready to close\n";
				fclose(fp);
				cleanUp(shmid, msqid, sharedMemPtr);
				exit(0);
			}
		}
	}
}



/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	std::cout << "I am cleaning up/n";
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr);
	/* TODO: Deallocate the shared memory chunk */
	shmctl(shmid, IPC_RMID, NULL);
	/* TODO: Deallocate the message queue */
	msgctl(msqid, IPC_RMID, NULL);
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	std::cout << "I got csignal/n";
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
	exit(0);
}

int main(int argc, char** argv)
{

	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
//	signal(SIGINT, ctrlCSignal);

	/* Initialize */
	init(shmid, msqid, sharedMemPtr);

	/* Go to the main loop */
	mainLoop();

	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
	cleanUp(shmid, msqid, sharedMemPtr);
	return 0;
}
