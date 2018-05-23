#include <netinet/in.h>
#include <time.h>
#include <pthread.h>

#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>

#define port "8080"
// Directory to put in your files you want to host.
#define webroot "/Users/luipokit/Desktop/comp4621_project"
#define MAXTHREAD (20)


int get_file_size(int fd)
{
	struct stat stat_struct;
	if(fstat(fd, &stat_struct) == -1)
		return(1);
	return (int)stat_struct.st_size;
}
void send_new(int fd,char *msg)
{
	int len = strlen(msg);
	if(send(fd,msg,len,0) == -1)
	{
		printf("Error in send\n");
	}
}
int recv_new(int fd,char *buffer)		// This function recieves the buffer untill a "End of line" byte is recieved
{
	#define EOL "\r\n"
	#define EOL_SIZE 2
	char *p = buffer;			// we'll be using a pointer to the buffer than to mess with buffer directly
	int eol_matched=0;			// this is used to see that the recieved byte matched the buffer byte or not 
	while(recv(fd,p,1,0)!=0)		// start recv bytes 1 by 1
	{
		if( *p == EOL[eol_matched])	// if the byte matched the first eol byte that is '\r'
		{	
			++eol_matched;		
			if(eol_matched==EOL_SIZE)	// if both the bytes matches the EOL
			{
				*(p+1-EOL_SIZE) = '\0';	// End the string
				return(strlen(buffer));	// return the bytes recieved 
			}
		}
		else
		{
			eol_matched=0;			
		}
		p++;					// increment the pointer to recv next byte
	}
	return(0);
}

//get the file extension
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

//get content function
const char* get_file_content(const char *filename){
	int c;
	FILE *file;
	file = fopen(filename, "r");
	if (file) {
		while ((c = getc(file)) != EOF)
		putchar(c);	
		fclose(file);
	}
	printf("\r\n");
}

//request_func
void* request_func(void *args)
{
	/* get the connection fd */
	int connfd = (int)args;
    char buff[100] = {0};

	printf("heavy computation\n");
	printf("\r\n");
	sleep(10);
       
  	/* prepare for the send buffer */ 
    snprintf(buff, sizeof(buff) - 1, "This is the content sent to connection %d\r\n", connfd);

	/* write the buffer to the connection */
	write(connfd, buff, strlen(buff));
	
	close(connfd);
}
			
int connection(int fd)
{
	char request[500],resource[500],*ptr;		
	int fd1,length;
	if(recv_new(fd,request) == 0)
	{
		printf("Recieve failed\n");
	}
	// Checking for a valid browser request
	ptr = strstr(request," HTTP/");
	if(ptr == NULL)
	{
		printf("NOT HTTP!!\n");
	}
	else
	{
		*ptr=0;
		ptr=NULL;
	
		
		if( strncmp(request,"GET ",4) == 0)
		{
			ptr=request+4;
		}
		if(strncmp(request,"HEAD ",5) == 0)
			ptr=request+5;
		if(ptr==NULL)
		{
			printf("Unknown Request !!! \n");
		}
		else
		{
			if( ptr[strlen(ptr) - 1] == '/' )
			{
				strcat(ptr,"index.html");
			}
			strcpy(resource,webroot);
			//strcpy(resource,SERVER_PORT);

			strcat(resource,ptr);
			fd1 = open(resource,O_RDONLY,0);

			//print open file
			//printf("Opening \"%s\"\n",resource);

			if(fd1 == -1)
			{
				printf("404 File not found Error\n");
				send_new(fd,"HTTP/1.1 404 Not Found\r\n");
				send_new(fd,"Server : pklui/Private\r\n\r\n");
				send_new(fd,"<html><head><title>404 not found error!! :( </head></title>");
				send_new(fd,"<body><h1>404 file not found</center></h1><br><h1>You are in Kit's Server.</h1><br><h1>Please try again.</h1></body></html>");
			}
			else
			{
				//print header
				if(strcmp(get_filename_ext(resource), "html") == 0){
					printf("HTTP/1.1 200 OK\r\n");
					printf("Content-Type: text/html\r\n");
					printf("Content-Length: %d\r\n", get_file_size(fd1));
					printf("Connection: Keep-Alive\r\n");
					printf("\r\n");
					printf("%s\r\n", get_file_content(resource));
				}else if(strcmp(get_filename_ext(resource), "pdf") == 0){
					printf("HTTP/1.1 200 OK\r\n");
					printf("Content-Type: application/pdf\r\n");
					printf("Content-Length: %d\r\n", get_file_size(fd1));
					printf("Connection: Keep-Alive\r\n");
					printf("\r\n");
				}else if(strcmp(get_filename_ext(resource), "jpeg") == 0){
					printf("HTTP/1.1 200 OK\r\n");
					printf("Content-Type: img/jpeg\r\n");
					printf("Content-Length: %d\r\n", get_file_size(fd1));
					printf("Connection: Keep-Alive\r\n");
					printf("\r\n");
				}else if(strcmp(get_filename_ext(resource), "css") == 0){
					printf("");
				}else{
					printf("Cant support this file extension");
				}

				send_new(fd,"HTTP/1.1 200 OK\r\n");
				send_new(fd,"Server : pklui/Private\r\n\r\n");
				if( ptr == request+4 ) // if it is a get request
				{
					if( (length = get_file_size(fd1)) == -1 )
						printf("Error getting size \n");
					if( (ptr = (char *)malloc(length) ) == NULL )
						printf("Error allocating memory!!\n");
					read(fd1,ptr,length);
			
					if(send(fd,ptr,length,0) == -1)
					{
						printf("Send err!!\n");
					}
					free(ptr);
				}
			}
			close(fd);
		}
	}
	shutdown(fd,SHUT_RDWR);
}	


int main()
{
	int listenfd,connfd;
	//new (comp4621)
	int listenfd1,connfd1;

	//IP address (comp4621)
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len = sizeof(struct sockaddr_in);
	
	char ip_str[INET_ADDRSTRLEN] = {0};
    int threads_count = 0;
    pthread_t threads[MAXTHREAD];

	//initialize server socket (comp4621)
	listenfd1 = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd1 <0){
		printf("Error: init socket\n");
        return 0;
	}
	//initialize server address (IP:port) (comp4621)
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; /* IP address: 0.0.0.0 */
    servaddr.sin_port = htons(8080); /* port number */

    //bind the socket to the server address (comp4621)
    if (bind(listenfd1, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
        printf("Error: bind\n");
        return 0;
    }
    int LISTENNQ = 5;
    if (listen(listenfd1, LISTENNQ) < 0) {
        printf("Error: listen\n");
        return 0;
    }

    printf("Server is open for listening on port 8080\n");
    //keep processing incoming requests (comp4621)
    while (1) {
      	//accept an incoming connection from the remote side (comp4621)
        connfd1 = accept(listenfd1, (struct sockaddr *)&cliaddr, &len);
        if (connfd1 < 0) {
            printf("Error: accept\n");
            return 0;
        }

        //print client (remote side) address (IP : port) (comp4621)
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
        printf("Incoming connection from %s : %hu with fd: %d\n", ip_str, ntohs(cliaddr.sin_port), connfd1);

        connection(connfd1);

		/* create dedicate thread to process the request */
		if (pthread_create(&threads[threads_count], NULL, request_func, (void *)connfd1) != 0) {
			printf("Error when creating thread %d\n", threads_count);
			return 0;
		}
		printf("The number of threads : %d\r\n", threads_count);
		if (++threads_count >= MAXTHREAD) {
			break;
		}
    }

    printf("Nax thread number reached, wait for all threads to finish and exit...\n");
	for (int i = 0; i < MAXTHREAD; ++i) {
		pthread_join(threads[i], NULL);
	}

	printf("Thanks for using my server!!!\n");
	

	return(0);
}
