#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 						//Inclusion of header files


int validate_input(int argc, char *argv);
int get_function(char*, int);
int put_function(char*, int);


void error(const char *msg)					//Function called to display error messages
{
	perror(msg);
	exit(0);
}

int validate_input(int argc, char *argv)			//Function to check whether the input by the user is valid or not
{
	if(argc < 5) 
	{
		fprintf(stderr,"usage ./client hostname port GET/PUT filename\n");
		return 0;
	}

	if(strcmp( argv, "PUT") && strcmp( argv, "GET")) 
	{
		fprintf(stderr, "Wrong Command: GET/PUT missing\n");
		return 0;
	}
	return 1;
	
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;					//Creating a socket connection to connect to the server	
	struct sockaddr_in serv_addr;
	struct hostent *server;

	int valid;
	
	valid = validate_input(argc, argv[3]);			//Call the validate_input function
	if(valid == 0)
		exit(1);

	portno = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");


	int size;						//Generating the http request to send to the server
	size = sizeof(argv[4]) + sizeof(argv[1]) + 40;
	char request[size];
	bzero(request, size);
	strcat(request, argv[3]);
	strcat(request, " ");
	strcat(request, "/");
	strcat(request, argv[4]);
	strcat(request, " HTTP/1.1\r\n");
	strcat(request, "host: ");
	strcat(request, argv[1]);
	strcat(request, "\r\n\r\n");
	printf("%s\n",request);

	n = send(sockfd, request, strlen(request), 0);		//Sending the http request generated to the server
	if (n == -1) 
		error("ERROR writing to socket");

	if(!strcmp(argv[3], "GET"))				//Calling the get function in case of a GET request
	{
		get_function(argv[4], sockfd);
	}
	else							//Calling the put function in case of a PUT request
	{
		put_function(argv[4], sockfd);
	}

	close(sockfd);
	return 0;
}

int get_function(char* filename, int sockfd)			//Get function to receive/get a file from the server
{
	char buffer[256];
	int n;
	bzero(buffer,256);
	n = read(sockfd,buffer, 15);
	if (n < 0)
		error("ERROR reading from socket");

	if(!strcmp(buffer, "HTTP/1.1 404 NO"))				//Error message displayed if the file is not found
	{
		printf("404 NOT FOUND");
	}
	else
		printf("%s",buffer);
	printf("\n");
	printf("here\n");

	if(!strcmp(buffer, "HTTP/1.1 200 OK"))				//Retrieving the file character by character if it is present on the server
	{
		char ch;
		do
		{
			bzero(&ch, 1);
			n = recv(sockfd, &ch, 1, 0);
			if (n < 0)
				error("ERROR reading from socket");

			if(ch != EOF)
				printf("%c", ch);

		}while(n != 0);	

	}
	return 1;
}

int put_function(char *arg, int sockfd)				//Put function to put a file onto the server
{
	int n;
	char buffer[256];
	bzero(buffer, 256);
	FILE *fp;
	fp = fopen(arg, "r");
	char x;
	if(fp)							//Checking whether the file to be copied is actually present or not
	{
		x = '1';
		n = write(sockfd, &x, 1);
		if (n < 0) 
			error("ERROR writing to socket");

		char ch ;
		do
		{
			ch = fgetc(fp);
			n = write(sockfd, &ch, 1);
			if(n < 0) 
				error("ERROR writing to socket");

		}while(ch != EOF);
		fclose(fp);
	}
	else							//Intimidating the server that the file was not present with the client
	{	
		x = '0';
		n = write(sockfd, &x, 1);
		if (n < 0) 
			error("ERROR writing to socket");
		printf("Unable to open file\n");
	}

	n = read(sockfd, buffer, 255);
	if (n < 0)
		error("ERROR reading from socket");
	printf("%s\n", buffer);

	return 1;

}
