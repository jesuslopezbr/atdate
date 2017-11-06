#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 37
#define BACKLOG 10	 // how many pending connections queue will hold
#define SERVER_PORT 6898

int sockfd;

void sigint_handler(int signo)
{
	/* things to do after a Ctrl-C, before finishing the program*/
	puts("SIGINT received, closing program");
	exit(0);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr; // client addr
  struct hostent *server;
  struct hostent *hostp; // client host info
	char *h;
  char *host;
  char *p;
  char *m;
  char *mode;
  char *hostaddrp; // dotted decimal host addr string
	int i, debug, port, seconds, n, sockfd, new_fd, optval;
  uint clientlen; // byte size of client's address

  /* handler SIGINT signal*/
	signal(SIGINT, sigint_handler);

	for(i=0; i<argc; i++){
		if(strcmp(argv[i],"-h")){
			host = argv[i+1];
		}
		if(strcmp(argv[i],"-m")){
			mode = argv[i+1];
		}
		if(strcmp(argv[i],"-p")){
			port = atoi(argv[i+1]);
		}
		if(strcmp(argv[i],"-d")){
			debug = 1;
		}
	}

	printf("%s\n", mode);

  if(strcmp (mode,"ct") == 0){
      printf("TCP_Client\n");
      /* socket: create the socket */
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
              perror("ERROR opening socket");
              exit(0);
      }

      /* gethostbyname: get the server's DNS entry */
      server = gethostbyname(host);
      if (server == NULL) {
              fprintf(stderr,"ERROR, no such host: %s\n", host);
              exit(0);
      }

      /* build the server's Internet address */
      bzero((char *) &serveraddr, sizeof(serveraddr));
      serveraddr.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
      serveraddr.sin_port = htons(port);

      /* connect: create a connection with the server */
      if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
              perror("ERROR connecting");
              exit(0);
      }

      char output[4096];

			while(1){

      	n = recv(sockfd, &seconds, sizeof(int), 0);
      	if (n < 0) {
              perror("ERROR reading from socket");
              exit(0);
      	}
				if(n==0){
					exit(0);
				}

      	time_t sec_today = ntohl(seconds) - 2208988800;

      	strftime(output, 4096, "%c", localtime(&sec_today));

      	printf("%s\n", output);
			}

      close(sockfd);

    }

		if(strcmp (mode,"s") == 0){
      //Server();
			printf("TIME server running in port 6898\n");
			/* socket: create the parent socket */
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) {
							perror("ERROR opening socket");
							exit(0);
			}

			/* setsockopt: lets s rerun the server immediately after we kill it.
 			* Eliminates "ERROR on binding: Address already in * use"
 			* error. */
			optval = 1;
			setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

			/* build the server's Internet address */
			bzero((char *) &serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
			serveraddr.sin_port = htons(SERVER_PORT);

			/* bind: associate the parent socket with a port */
			if (bind(sockfd, (struct sockaddr *) &serveraddr,
				 sizeof(serveraddr)) < 0) {
							perror("ERROR on binding");
							exit(0);
			}

			/* listen: make this socket ready to accept connection requests */
			if (listen(sockfd, BACKLOG) < 0) {
							perror("ERROR on listen");
							exit(0);
			}

			/* wait for a connection request */
			while(1) {  // main accept() loop
				clientlen = sizeof(clientaddr);
				new_fd = accept(sockfd, (struct sockaddr *)&clientaddr,&clientlen);

				if (new_fd == -1) {
					perror("ERROR on accept");
					exit(0);
				}

				/* gethostbyaddr: determine who sent the message */
				hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
							  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
				if (hostp == NULL) {
								perror("ERROR on gethostbyaddr");
								exit(0);
				}
				hostaddrp = inet_ntoa(clientaddr.sin_addr);
				if (hostaddrp == NULL) {
								perror("ERROR on inet_ntoa\n");
								exit(0);
				}

				printf("Received request from Client: %s : %d\n", hostaddrp, SERVER_PORT);

				int seconds, bytes_sent;

				if (!fork()) { // this is the child process
					close(sockfd); // child doesn't need the listener
					while(1){
						seconds = time(NULL) + 2208988800;
						seconds = htonl(seconds);
						bytes_sent = send(new_fd, &seconds, sizeof(int), 0);
						if (bytes_sent == -1) {
							perror("ERROR on send");
							exit(0);
						}
						sleep(1);
					}
					close(new_fd);
					exit(0);
				}
				close(new_fd);
			}
			close(sockfd);  // parent doesn't need this
    }

		if(strcmp (mode,"cu") == 0){
      //UDP_Client();
      printf("UDP_Client\n");
			/* socket: create the socket */
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if (sockfd < 0) {
							perror("ERROR opening socket");
							exit(0);
			}

			/* gethostbyname: get the server's DNS entry */
			server = gethostbyname(host);
			if (server == NULL) {
							fprintf(stderr,"ERROR, no such host: %s\n", host);
							exit(0);
			}

			/* build the server's Internet address */
			bzero((char *) &serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
			serveraddr.sin_port = htons(DEFAULT_PORT);

			/* connect: create a connection with the server */
      if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
              perror("ERROR connecting");
              exit(0);
      }

				//UDP mandar datagrama vacio
			n = send(sockfd, NULL, 0, 0);
			if (n < 0) {
							perror("ERROR writing to socket");
							exit(0);
			}
			char output[4096];

			n = recv(sockfd, &seconds, sizeof(int), 0);
			if (n < 0) {
							perror("ERROR reading from socket");
							exit(0);
			}

			time_t sec_today = ntohl(seconds) - 2208988800;

			strftime(output, 4096, "%c", localtime(&sec_today));

			printf("%s\n", output);

    }

	return(0);
}
