/* UDP client in the internet domain which will run as ./client IP_of_client portno file_path*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

// structure for packet
#pragma pack(1)
typedef struct packet_client
{
	int seq;			   //sequence number
	char client_buff[1050]; // file packet
	int status_ack;		   // to send the ack packet description
} client;
#pragma pack(0)

void error(const char *);

int main(int argc, char *argv[])
{
	int sock, n, filesizecheck = 0 ,fd;
	unsigned int length, length1;

	struct sockaddr_in server, from;
	//struct hostent *hp;
	int hostvalue;

	client c1,ack;

	char *END_FLAG = "=====END=====";
	
	char buffer[1024];
	char *filename;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		error("socket");
	else
		printf("Socket %d is booked \n", atoi(argv[2]));

	//by default a char format is passed, hence a conversion is required.

	server.sin_port = htons(atoi(argv[2]));
	server.sin_family = AF_INET;
	hostvalue = inet_pton(AF_INET, argv[1], &(server.sin_addr)); //binding with server

	// assigning the value for packet as it is an acknoledgement packet
	
	c1.seq = 1;
	sprintf(c1.client_buff, "%s\n", "Connection ACK from client");
	c1.status_ack = 1;
	length = sizeof(struct packet_client);
	length1 = sizeof(struct sockaddr_in);

	//hp = gethostbyname(argv[1]);

	if (hostvalue == 0)
		error("Unknown host");
	else
	{
		printf("Connection created at client and sending packet to server for ACK \n");
		n = sendto(sock, (struct packet_client *)&c1, length, 0, (struct sockaddr *)&server, length1);
		if (n <= 0)
			error("Sendto");
	}

	//Receiveing the ack from server

	n = recvfrom(sock, (struct packet_client *)&c1, length, 0, (struct sockaddr *)&from, &length1);
	if (n < 0)
		error("recvfrom");
	else
		printf("Received message from server is : %s \n", c1.client_buff);

	// sending the ack to server with incrementing sequence number -- last step of 3-way handshake

	c1.seq = c1.seq + 1;
	sprintf(c1.client_buff, "%s\n", "Connection ACK from client with incremented sequence number");
	c1.status_ack = 1;

	n = sendto(sock, (struct packet_client *)&c1, length, 0, (struct sockaddr *)&server, length1);

	if (n <= 0)
		error("Sendto");

	// sending the file path to be read
	filename = argv[3];
	n = sendto(sock, filename, strlen(filename), 0, (struct sockaddr *)&server, length1);
	if (n <= 0)
		error("Sendto");
	else
		printf("filename being sent: %s \n", filename);

	//sleep(10);
	//recv_file(sock);
	size_t filesize;
	recvfrom(sock, &filesize, sizeof(filesize), 0, (struct sockaddr *)&from, &length1);
	printf("the file size is %zd \n", filesize);

	c1.seq = 0;
	int len = sizeof(struct packet_client);
	unsigned char *clientdata = malloc(len);
	unsigned char *serverack = malloc(len);
	
	int count = 0;

	FILE *fp = fopen("receivedfile.txt", "w");
	if (fp <= 0)
	{
		printf("File not opened");
	}
	else
	{
		
	while (n = recvfrom(sock, clientdata, len, 0, (struct sockaddr *)&from, &length1)) 
	{
		memcpy(&c1, clientdata, sizeof(struct packet_client));
		printf("\n ***Received packet from %s : %d*** \n ", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
		printf("\n ***Data for Packet %d:***\n" , c1.seq);
		
		// sending server ack
		ack.seq = c1.seq;
		ack.status_ack = 3;
		strncpy(ack.client_buff, "Packet Receieved", 100);
		
		memcpy(serverack,&ack,sizeof(struct packet_client));

		//n = sendto(sock,serverack,len,0,(struct sockaddr *)&server, length1);

        	if (!(strcmp(c1.client_buff, END_FLAG))) 
		{
			printf("File Received, please close the connection \n");
           		break;
       		}
		
		// testing of packet drop
		//if (c1.seq == 1900 && count == 0)
	//	{
	//		count = count + 1;
	//		printf("trying to drop so nothing to be done with count value %d",count);
			
	//		sleep(5);
	//	}		
	//	else
	//	{
       			fprintf(fp, "%s", c1.client_buff);
			n = sendto(sock,serverack,len,0,(struct sockaddr *)&server, length1);
	//	}
   	}fclose(fp);
	}	
   	
	bzero(clientdata,512);
	n = recvfrom(sock, clientdata, len, 0, (struct sockaddr *)&from, &length1);
	if (n <= 0)
		error("recvfrom");
	else 
	{
		memcpy(&ack,clientdata,sizeof(struct packet_client));
		if(ack.status_ack == 1)
		{
			printf("Request received for closing the termination -- Hence sending the packet to terminate the connection.. \n");
			
		}
		else
		{
			printf("Not received proper packet for closing the connection \n");
		}
	}
	printf("Closing the connection \n");
	close(sock);
	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
