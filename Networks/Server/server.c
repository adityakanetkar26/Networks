#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>						//Inclusion of header files

int validate_input(int argc) ;
int get_function(char*, int);
int put_function(char*, int);
void recv_request(int, char*);
int parse_request(char*, char*, char*, char*, char*);

void error(const char *msg)					//Function called to display error messages
{
	perror(msg);
	exit(1);
}

int validate_input(int argc)					//Function to check whether the input by the user is valid or not
{
	if(argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		return 0;
	}
	return 1;
	
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;				//Creating a socket connection to initialize the server
	socklen_t clilen;
	char buffer[512];
	char method[4], filename[250], version[10], hostname[250];

	struct sockaddr_in serv_addr, cli_addr;
	int valid, invalid = 0;
	
	valid = validate_input(argc);
	if(valid == 0)
		exit(1);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	listen(sockfd, 5);

	while(1)						//Running the server in an infinite loop
	{
		
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		if (newsockfd < 0) 
			error("ERROR on accept");

		bzero(buffer, 512);				//Initializing all the variables to be blank initially
		bzero(method, 4);
		bzero(filename, 250);
		bzero(version, 10);
		bzero(hostname, 250);

		recv_request(newsockfd, buffer);		//Recv_request to receive the http request
		invalid = parse_request(buffer, method, filename, version, hostname);
								//Function to parse the http request
		
		if(invalid == 0)
		{
			printf("Bad Request\n");
			exit(2);
		}
		

		printf("Method:%s\n", method); 
		printf("Filename:%s\n", filename);
		printf("Version:%s\n", version);
		printf("Hostname: %s\n", hostname);
		
		if(!strcmp(method, "GET"))			//Checking the http request and calling the appropriate function
		{
			get_function(filename, newsockfd);
			
		}
		else if(!strcmp(method, "PUT"))
		{
			put_function(filename, newsockfd);
		}
		printf("\n");

		close(newsockfd);
	}

	
	close(sockfd);
	return 0; 
}

void recv_request(int newsockfd, char *buffer)			//Recv_request to read the incoming request character by character in a buffer
{
	int end, n;
	end = 0;
	char input;
	int pos = 0;
	while(end != 4)
	{
		n = recv(newsockfd, &input, 1, 0);
		if(n < 0)
			error("ERROR reading from socket");

		buffer[pos] = input;
		pos++;
		if(end == 0)
		{
		
			if(input == '\r')
				end = 1;
		}
		else if(end == 1)
		{
			if(input == '\n')
				end = 2;
			else
				end = 0;
		}			
		else if(end == 2)
		{
			if(input == '\r')
				end = 3;
			else
				end = 0;
		}			
		else if(end == 3)
		{
			if(input == '\n')
				end = 4;
			else
				end = 0;
		}			
		bzero(&input, 1);
		
	}
	buffer[pos] = '\0';
}

int parse_request(char* request, char* method, char* filename, char* version, char* hostname)
							//Function to parse this request and separate filename, method, hostname and version
{
	int i = 0;
	char *point;
	while(request[i] == '\t' || request[i] == ' ')
		i++;

	if(request[i] == '\0')
		return 0;

	point = &(request[i]);
	strncpy(method, point, 3);
	method[3] = '\0';
	i = i + 3;				

	while(request[i] == '\t' || request[i] == ' ')
		i++;

	if(request[i] == '\0')
		return 0;
	else
	{
		i++;		
		int x = 0;
		while(request[i] != ' ')
		{
			filename[x] = request[i];
			x++;
			i++;
		}
		filename[x] = '\0'; 		
		x = 0;
		
		if(request[i] == '\0')
			return 0;
		else
		{
			while(request[i] != '\r')
			{
				version[x] = request[i];
				x++;
				i++;
			}
			i++;
			if(request[i] == '\0')
				return 0;
			else
			{
				i++;
				int test;
				point = &(request[i]);
				test = strncmp(point, "host:", 5);

				if(test != 0 || request[i] == '\0')
					return 0;
				else
				{
					i = i + 5;
					
					while(request[i] == '\t' || request[i] == ' ')
						i++;
					x = 0;
					while(request[i] != '\r')
					{
						hostname[x] = request[i];
						x++;
						i++;
					}
					
					point = &(request[i]);
					int tests;
					tests = strncmp(point, "\r\n\r\n", 4);
					if(tests != 0)
						return 0;
					
				}
			}			
						
		}
	}			
	return 1;
}

int get_function(char *buffer, int newsockfd)				//Get function which is called in case of a get request
{
	FILE *fp;
	int n;
	fp = fopen(buffer,"r");
	if(fp)
	{
		char ch;
		n = write(newsockfd,"HTTP/1.1 200 OK",strlen("HTTP/1.1 200 OK"));
		if(n < 0) 
			error("ERROR writing to socket");
		
		do{
			ch = fgetc(fp);
			n = write(newsockfd, &ch, 1);
			if (n < 0) 
				error("ERROR writing to socket");
					
		}while( ch != EOF );

		fclose(fp);
	}
	else
	{
		n = write(newsockfd,"HTTP/1.1 404 NOT FOUND\n",strlen("HTTP/1.1 404 NOT FOUND\n"));
		if (n < 0) 
			error("ERROR writing to socket");
	}
	return 1;
}

int put_function(char *buffer, int newsockfd)				//Put function which is called in case of a put request
{
	char buf;
	int n;
	n = read(newsockfd, &buf, 1);
	if(n < 0)
		error("ERROR reading from socket");

	if(buf == '1')
	{
		FILE *fp;
		fp = fopen(buffer, "a");
		char ch;
		do
		{
			bzero(&ch, 1);
			n = read(newsockfd, &ch, 1);
			if (n < 0)
				error("ERROR reading from socket");
			if(ch != EOF)
				fputc(ch, fp);
					
		}while(ch != EOF);		
	
		fclose(fp);

		n = write(newsockfd,"200 OK File Created\n",strlen("200 OK FIle Created\n"));
		if (n < 0) 
			error("ERROR writing to socket");	
				
	}
	else if(buf == '0')
	{
		n = write(newsockfd," File Not Created\n",strlen("File Not Created\n"));
		if (n < 0) 
			error("ERROR writing to socket");	

	}
	return 1;
}
