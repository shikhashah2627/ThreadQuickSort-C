// Server UDP which should be executed as : ./server portno
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h> // responsible for write & read function
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

// structure for packet
#pragma pack(1)
typedef struct server_packet 
	{
		int server_seq;//sequence number
		char server_buffer[1050]; // file packet
		int server_status_ack;// to send the ack packet description	1 - connection; 2 - data; 3 - ACK		
	}server;


#pragma pack(0)

// prints error message.
void error(const char *msg)
{
    perror(msg);
    exit(0);
}	


int main(int argc, char *argv[])
{
	int sock, length, n, length1,sent_count,i,rec,delaytime;
  	socklen_t fromlen;
	server serverAck,clientAck;// struct packet creation
	struct sockaddr_in server,from;
	struct timeval read_timeout,start,end; // to check timeout

	ssize_t read_bytes, // file bytes read
		sent_bytes, //sequence for the packet through socket
		sent_file_size; 
	char send_buf[1024]; // buffer size for the division of packet
	length = sizeof(server);
	sent_bytes = 0;

	char *errmsg_notfound = "File not found\n";
	char *END_FLAG = "=====END=====";

	int f; /* file handle for reading local file*/
	sent_count = 0;
	sent_file_size = 0;
	char filename[400];
	
	if (argc < 2) 
	{
		fprintf(stderr, "ERROR, no port provided\n");
     		exit(0);
   	}
   
	sock = socket(AF_INET, SOCK_DGRAM, 0); // socket creation
	
  	if (sock < 0) 
	{
		error("Opening socket");
	}

	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
   	server.sin_port=htons(atoi(argv[1]));
	
   	if (bind(sock,(struct sockaddr *)&server,length) < 0) 
	      	error("binding");
		
	// Connection Request from client
	
	n = recvfrom(sock,(struct server_packet *)&serverAck,sizeof(struct server_packet),0,(struct sockaddr *)&from,&length);

       	if (n <= 0)
		error("recvfrom");
	else
		
	{
		printf("Received a datagram packet asking for the connection stating below information: \n");
		printf("Received message: %s with sequence number %d \n",serverAck.server_buffer,serverAck.server_seq);
	}
	
	// Acknowledging the request by passing the packet with same ack number and thereafter it is increased with one
	
	serverAck.server_seq = serverAck.server_seq;
	sprintf(serverAck.server_buffer, "%s\n", "Connection ACK from server");
	serverAck.server_status_ack = 1;
	n = sendto(sock,(struct server_packet *)&serverAck,sizeof(struct server_packet),0,(struct sockaddr *)&from,length);
		
	if (n <= 0) 
		error("sendto");

	// Last Ack message from client with incremented sequence number
	
	n = recvfrom(sock,(struct server_packet *)&serverAck,sizeof(struct server_packet),0,(struct sockaddr *)&from,&length);

       	if (n <= 0) 
		error("recvfrom");
	else		
	{
		printf("Received a last step ack from client with below message \n");
		printf("Received message: %s with sequence number %d \n",serverAck.server_buffer,serverAck.server_seq);
	}
		
	// sending the file path that is received from client
	
	bzero(filename,400);	// way to initialize the array with null value
	n = recvfrom(sock,&filename,400,0,(struct sockaddr *)&from,&length);
	if (n <= 0)
		close(sock);
	else
		printf("The file path received is - %s \n",filename);
	
	// reading the asked file
	f = open(filename, O_RDONLY);
	if( f < 0) /* can't open requested file */
	{
		perror(filename);

	}
	else // get the size of file that is being sent
	{
	printf("Sending file: %s\n", filename);
	bzero(send_buf,256);
	
    	FILE *fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	size_t lengthOfFile = ftell(fp);
	fclose(fp);
	printf("filesize : %zd",lengthOfFile);	
	sendto(sock,&lengthOfFile,sizeof(lengthOfFile),0,(struct sockaddr *)&from,length);
	
	}
	int len = sizeof(struct server_packet);
	unsigned char * raw = malloc(len);
	unsigned char * client = malloc(len);
	serverAck.server_seq = 0;
	
	read_timeout.tv_sec = 3;		
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout) < 0  )
	{
		printf("please check it is being re-send \n");
	}
	
	// file getting read
	while((n = read(f, send_buf,1024)) > 0)
	{
		printf("\n %d packet is being read and sent \n \n ",serverAck.server_seq);		
		sprintf(serverAck.server_buffer, "%s", send_buf);
		serverAck.server_status_ack = 2;
		memcpy(raw, &serverAck, len);
		// sending packet to client
		n = sendto(sock,raw,len,0,(struct sockaddr*)&from,length);

		// Receiving ack from client
		rec = recvfrom(sock,client,len,0,(struct sockaddr *)&from,&length);

		if (rec <= 0)
		{
			printf("Ack packet not received \n");
			serverAck.server_seq--; 
		}
		else
		{
			memcpy(&clientAck, client, sizeof(struct server_packet));
			printf("\n ***Received Ack packet from %s : %d : for packet : %d *** \n ", inet_ntoa(from.sin_addr),
				ntohs(from.sin_port),clientAck.server_seq);
		}

		bzero(raw,len);
		serverAck.server_seq++;
		sent_file_size += strlen(send_buf);
		bzero(send_buf,1024);
		
	} close(f);
	printf("Done with the client. Sent %zu bytes in %d send(s)\n\n",sent_file_size, serverAck.server_seq);

	bzero(raw,len);
	serverAck.server_status_ack = 0;
	serverAck.server_seq = 0;
	sprintf(serverAck.server_buffer, "%s", END_FLAG);
	memcpy(raw, &serverAck, len);
	n = sendto(sock,raw,len,0,(struct sockaddr*)&from,length);

	printf("Connection meesage being sent for closure\n");
	
	bzero(client,len);
	serverAck.server_status_ack = 1;
	serverAck.server_seq = 1;
	sprintf(serverAck.server_buffer, "%s", "Closing the connection...\n");
	memcpy(client, &serverAck, len);
	n = sendto(sock,client,len,0,(struct sockaddr*)&from,length);

	close(sock);
return 0;
 }

