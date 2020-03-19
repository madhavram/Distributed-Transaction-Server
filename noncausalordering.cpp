/*  Project-2 Advance Operating Systems
 * Title: Clocks, Multicast and Commits
 * Author: Madhav Ram Rajappan Seetharaman
 * Student ID: FJ37459
 * Assignment-2 Version with noncausalordering
 */

#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#define PORT 12345
#define IP "225.0.0.37"
#define INUSE "/home/madhav/os-project2/file_counter.txt"
using namespace std;

void *sender(void *);
void *receiver(void *);

int m_sock, p_sock;
int logical_clock, process, p, m, send_clock, send_port;
struct sockaddr_in p_addr, port_master, m_addr, m_r_addr;
struct ip_mreq mr;
char * msg_input;
char mesg_bcast[256];

struct message_multicast{
	int processid;
	char rv_msg[256];
	int arr[3];
};

struct message_multicast message_array[50];

int arr1[3] = {0,0,0};
int n = 3;


int main(int argc, char *argv[])
{
	u_int on = 1;
	char message1[256];
	char message2[256];
	char message3[256];
	char message4[256];
	char buffer1[256];
	int m1, m2, m3;
	int initial_value, drift, send_drift;
	unsigned int rank = atoi(argv[1]);
	msg_input = argv[2];
	pthread_t sender_th, receiver_th;
	

	//Creation of multicast socket
	m_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_sock < 0)
	{
		cout << "Cannot create socket" << endl;
		return 0;
	}
	
	//Setting up destination address	
	memset((char*) &m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(IP);
	m_addr.sin_port = htons(PORT);

	//set up destination address on receiver side
	memset((char*) &m_r_addr, 0, sizeof(m_r_addr));
	m_r_addr.sin_family = AF_INET;
	m_r_addr.sin_addr.s_addr =htonl(INADDR_ANY);
	m_r_addr.sin_port = htons(PORT);

	//allow multiple sockets to use the same PORT number
	if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Unable to allow multiple sockets to use the same PORT number" << endl;
		return 0;
	}

	//bind to destination address
	m = bind(m_sock, (struct sockaddr *) &m_r_addr, sizeof(m_r_addr));
	if(m < 0)
	{
		cout << "Cannot bind" << endl;
		return 0;
	}

	//use setsockopt() to request the kernel to join the multicast group
	mr.imr_multiaddr.s_addr = inet_addr(IP);
	mr.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if(setsockopt(m_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
	{
		cout << "Error in setsockopt" << endl;
		return 0;
	}
	
	//create point to point socket
	p_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//set destination address
	p_addr.sin_family = AF_INET;
	p_addr.sin_addr.s_addr = INADDR_ANY;
	p_addr.sin_port = htons(INADDR_ANY);

	//Request to use same address
	if(setsockopt(p_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Cannot reuse address" << endl;
		return 0;
	}

	//Bind point to point socket
	p = bind(p_sock, (struct sockaddr *) &p_addr, sizeof(p_addr));
	if(p < 0)
	{
		cout << "Cannot bind point to point socket" << endl;
		return 0;
	}
	

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if(setsockopt(p_sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))<0)
	{
	  cout << "Timeout eror" << endl;  
	}

	
	//Create threads for sender and reciever
	//*rank = malloc(sizeof(rank));
	if(pthread_create(&sender_th,NULL,sender,&rank)<0)
	{
	  cout << "Cannot create a thread for sender" << endl; 
	  return 1;
	}
	if(pthread_create(&receiver_th,NULL,receiver,&rank)<0)
	{
	  cout << "Cannot create a thread for receiver" << endl;
	  return 1;
	}
	pthread_join(sender_th,NULL);

	pthread_join(receiver_th,NULL);
	return 0;
}
//Thread for sending the mesages to all the processes
void *sender(void *r)
{
	unsigned int rank1 = *((unsigned int *) r);
	int p1 = 0, p2 = 0, p3 = 0;
	int process;
	cout << "Rank: " << rank1 << endl;
	cout << "Message Input: " << msg_input << endl;
	while(1)
	{
		for (int i = 0; i < 3; i++)
		{
			if(i == rank1)
			{
				if(i == 0)
				{
					p1++;
					process = 1;
					cout << "Process: " << process;
					arr1[i] = p1;
				}
				if(i == 1)
				{
					p2++;
					process = 2;
					cout << "Process: " << process;
					arr1[i] = p2;
				}
				if(i == 2)
				{
					p3++;
					process = 3;
					cout << "Process: " << process;
					arr1[i] = p3;
				}
			}
		}
		//cout << process << endl;
		char buffer1[256];
		sprintf(buffer1,"%d,%d,%d,%d,%s", process, arr1[0], arr1[1], arr1[2], msg_input);
		cout << "Message: " << buffer1 << endl;
		int send1 = sendto(m_sock, buffer1, sizeof(buffer1), 0, (struct sockaddr *) &m_addr, sizeof(m_addr));
		sleep(10);
	}
	return 0;
}

void *receiver(void *r1)//thread for receiving the messages from all the processes
{
	char buffer2[256];
	//char rvbuffer[256];
	int count = 0;
	int processid;
	char rv_msg[256];
	int arr[3];
	int num = 0;
	int varr1[3] = {0,0,0}, varr2[3] = {0,0,0}, varr3[3] = {0,0,0};
	char *variable, *message, *arr2[3];
	socklen_t addr=sizeof(m_r_addr);
	//int receive1 = recvfrom(m_sock, buffer2, sizeof(buffer2), 0, (struct sockaddr *) &m_r_addr, &addr);
	while(recvfrom(m_sock, buffer2, sizeof(buffer2), 0, (struct sockaddr *) &m_r_addr, &addr) >= 0)
	{
		count++;
		cout << endl ;
		cout << "Non-Causal ordering-----" << endl;
		cout << "Message received: " << buffer2 << endl;
		cout << "Count: " << count << endl;
				
		message = strtok(buffer2, ",");
		
		if(message != NULL)
		{
			processid = atoi(message);
			cout << "Processid: " << processid << endl;
			message = strtok(NULL, ",");
			arr[0] = atoi(message);
			message = strtok(NULL, ",");
			arr[1] = atoi(message);
			message = strtok(NULL, ",");		
			arr[2] = atoi(message);
			message = strtok(NULL, " ,");
			strcpy(rv_msg, message);
			cout << endl ;
			cout << "Message: " << rv_msg << endl;
			cout << "------Vector value: " << arr[0] << "," << arr[1] <<"," << arr[2] << endl;
			//exit(1);		
		}
		sleep(10);
		bzero(buffer2, 256);
	}
	
	
}
/*Refences and Citations:
 * Below are the resources I utilized as references for implementation of this project
 * 1. Unix System Call setsockopt() and getsockopt():
 *    https://www.tutorialspoint.com/unix_system_calls/setsockopt.htm,
 *    https://stackoverflow.com/questions/4233598/about-setsockopt-and-getsockopt-function
 * 2. Multithreading- https://www.bogotobogo.com/cplusplus/multithreading_ipc.php
 * 3. https://stackoverflow.com/questions/18050065/specifying-port-number-on-client-side
 * 4. How to put int values in a char array:
 *    https://social.msdn.microsoft.com/Forums/vstudio/en-US/da286445-72b3-4b0c-87f7-4703121f9974/how-to-put-int-values-to-char-array?forum=vcgeneral
 * 5. http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
 * 6. https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.2.0/com.ibm.zos.v2r2.bpxbd00/getsn.htm
 * 7. https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getsockname
 * */
