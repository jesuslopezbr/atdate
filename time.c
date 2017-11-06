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

#define PORT 37

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
	char *h;
  char *host;
  char *p;
  char *m;
  char *mode;
	int port,seconds,n;
  struct hostent *server;

        /* handler SIGINT signal*/
	signal(SIGINT, sigint_handler);

        /* check command line arguments */
	switch (argc) {
	case 7://Consulta TCP
    h = argv[1];
		host = argv[2];
		p = argv[3];
    port = atoi(argv[4]);
    m = argv[5];
    mode = argv[6];

    if(strcmp (h,"-h") == 0 && strcmp (p,"-p") == 0 && strcmp (m,"-m") == 0 && strcmp (mode,"ct") == 0){
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

      n = recv(sockfd, &seconds, sizeof(int), 0);
      if (n < 0) {
              perror("ERROR reading from socket");
              exit(0);
      }

      time_t sec_today = ntohl(seconds) - 2208988800;

      strftime(output, 4096, "%+", localtime(&sec_today));

      printf("%s\n", output);

      close(sockfd);

    }else{

      fprintf(stderr, "usage: %s <-h> <host> <-p> <port> <-m> <ct>\n", argv[0]);
  		exit(1);

    }
		break;
  case 3://Servidor
    m = argv[1];
    mode = argv[2];

    if(strcmp (m,"-m") == 0 && strcmp (mode,"s") == 0){
      //Server();
      printf("Server\n");
    }else{
      fprintf(stderr, "usage: %s <-m> <s>\n", argv[0]);
  		exit(1);
    }
    break;
  case 5://Consulta UDP
    h = argv[1];
    host = argv[2];
    m = argv[3];
    mode = argv[4];

    if(strcmp (h,"-h") == 0 && strcmp (m,"-m") == 0 && strcmp (mode,"cu") == 0){
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
			serveraddr.sin_port = htons(PORT);

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

			strftime(output, 4096, "%+", localtime(&sec_today));

			printf("%s\n", output);

    }else{
      fprintf(stderr, "usage: %s <-h> <host> <-m> <cu>\n", argv[0]);
  		exit(1);
    }
    break;
	default:
		fprintf(stderr, "usage: %s <-h> <host> <-p> <port> <-m> <mode> <-d>\n", argv[0]);
		exit(1);
  }
	return(0);
}
