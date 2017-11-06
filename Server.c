/*
 * EchoServer_conc.c
 * Aplicaciones Telematicas - Grado en Ingenieria Telematica
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define BACKLOG 10	 // how many pending connections queue will hold
#define BUFSIZE 1024
#define PORT 6898

void sigchld_handler(int s);

int main(int argc, char *argv[])
{
	time_t timer;
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  uint clientlen; // byte size of client's address
  struct sockaddr_in serveraddr; // server's addr
  struct sockaddr_in clientaddr; // client addr
  struct hostent *hostp; // client host info
  char *hostaddrp; // dotted decimal host addr string
  int optval; // flag value for setsockopt
	char *mode;
	char *m;

	switch (argc) {
	case 3:
		m = argv[1];
		mode = argv[2];
		break;
	default:
		fprintf(stderr, "usage: %s <-m> <s>\n", argv[0]);
    exit(1);
  }

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
        serveraddr.sin_port = htons(PORT);

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

	/* handler SIGCHLD signal */
	signal(SIGCHLD, sigchld_handler);

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
                //printf("server got connection from %s (%s)\n",
		//       hostp->h_name, hostaddrp);
		printf("Received request from Client: %s:%d\n", hostaddrp, PORT);

		int seconds;

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			seconds = time(NULL) + 2208988800;
			seconds = htonl(seconds);
			int bytes_sent = send(new_fd,&seconds,sizeof(int),0);
			sleep(1)
			close(new_fd);
			exit(0);
		}
		close(new_fd);

	}

		/*if(fork() == 0){	// This is a child Process
			uint32_t buffer;
			time_t nowtime;
			int bytes_sent;
			do{
				nowtime = time(NULL);
				buffer = htonl(nowtime + 2208988800);
				bytes_sent = send(new_fd,&buffer,sizeof(uint32_t),0);
				sleep(1);
					if(0) {
						char time_str[64];
						strftime(time_str,64,"%c", localtime(&nowtime));
						//printf("Enviado %s (%d bytes) a %s\n", time_str, bytes_sent, inet_ntoa((struct in_addr)client_socket.sin_addr));
					}

				}while(bytes_sent > 0);
				/*printf("Conexión terminada con %s\n",inet_ntoa((struct in_addr)client_socket.sin_addr));
				Esto no lo imprime nunca,
				usé valgrind y descubrí que cerraba el hijo con un SIGPIPE (13),
				así que añadí un handler para esa señal.
				*/
			//}
		close(sockfd);  // parent doesn't need this
	return 0;
}

/*------------------------------------------------------------------------
 * sigchld_handler - reaper of zombi child processes
 *------------------------------------------------------------------------
 */
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
