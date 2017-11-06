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

#define DEFAULT_PORT 37 // Puerto por defecto
#define BACKLOG 10	 // conexiones que soporta en la cola el servidor
#define SERVER_PORT 6898 // 6000 + 3 ultimos digitos (898)

void sigint_handler(int signo) // Ctrl-C
{
	/* Después de Ctrl-C, antes de acabar el programa*/
	puts("SIGINT received, closing program");
	exit(0);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  struct hostent *server;
  struct hostent *hostp; // info cliente host
	char *h;
  char *host;
  char *p;
  char *m;
  char *mode;
  char *hostaddrp; // addr string decimal host
	int i, debug, port, seconds, n, sockfd, new_fd, optval;
  uint clientlen; // longitud direccion cliente

	signal(SIGINT, sigint_handler); // Señal de cerrar programa

	for(i=0; i<argc; i++){ //Comprobación de argumentos
		if(strcmp(argv[i],"-h") == 0){
			host = argv[i+1];
		}
		if(strcmp(argv[i],"-m") == 0){
			mode = argv[i+1];
		}
		if(strcmp(argv[i],"-p") == 0){
			port = atoi(argv[i+1]);
		}
		if(strcmp(argv[i],"-d") == 0){
			debug = 1;
		}
	}



/**/if(strcmp (mode,"ct") == 0){ // Cliente TCP
      /* socket: crear el socket */
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
              perror("ERROR opening socket");
              exit(0);
      }

      /* gethostbyname: DNS del servidor */
      server = gethostbyname(host);
      if (server == NULL) {
              fprintf(stderr,"ERROR, no such host: %s\n", host);
              exit(0);
      }

      /* Construir direcciones del servidor internet */
      bzero((char *) &serveraddr, sizeof(serveraddr));
      serveraddr.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
      serveraddr.sin_port = htons(port);

      /* connect: crear una conexion con el servidor */
      if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
              perror("ERROR connecting");
              exit(0);
      }

      char output[4096];

			while(1){

				// recibir bytes del servidor
      	n = recv(sockfd, &seconds, sizeof(int), 0);
      	if (n < 0) {
              perror("ERROR reading from socket");
              exit(0);
      	}
				if(n==0){
					exit(0);
				}
				// Conversion de fecha y hora
      	time_t sec_today = ntohl(seconds) - 2208988800;

      	strftime(output, 4096, "%c", localtime(&sec_today));
				// Sacar fecha y hora por pantalla
      	printf("%s\n", output);
			}
			// Cierre del socket (cierre de conexion)
      close(sockfd);

    }



/**/if(strcmp (mode,"cu") == 0){ // Cliente UDP
			/* socket: crear el socket */
			sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if (sockfd < 0) {
							perror("ERROR opening socket");
							exit(0);
			}

			/* gethostbyname: DNS del servidor */
			server = gethostbyname(host);
			if (server == NULL) {
							fprintf(stderr,"ERROR, no such host: %s\n", host);
							exit(0);
			}

			/* Construir direcciones del servidor internet */
			bzero((char *) &serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
			serveraddr.sin_port = htons(DEFAULT_PORT);

			/* connect: crear una conexion con el servidor */
      if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
              perror("ERROR connecting");
              exit(0);
      }

			// Cliente UDP manda datagrama vacio
			n = send(sockfd, NULL, 0, 0);
			if (n < 0) {
							perror("ERROR writing to socket");
							exit(0);
			}
			char output[4096];
			// Recibir bytes del servidor
			n = recv(sockfd, &seconds, sizeof(int), 0);
			if (n < 0) {
							perror("ERROR reading from socket");
							exit(0);
			}
			// Conversion fecha y hora
			time_t sec_today = ntohl(seconds) - 2208988800;

			strftime(output, 4096, "%c", localtime(&sec_today));
			// Sacar por pantalla fecha y hora
			printf("%s\n", output);

    }

		

		/**/if(strcmp (mode,"s") == 0){ // Servidor

					printf("TIME server running in port 6898\n");
					/* socket: crear el socket padre */
					sockfd = socket(AF_INET, SOCK_STREAM, 0);
					if (sockfd < 0) {
									perror("ERROR opening socket");
									exit(0);
					}

					/* setsockopt: Reactivar el servidor cuando se cierra. */
					optval = 1;
					setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

					/* Construir direcciones del servidor internet */
					bzero((char *) &serveraddr, sizeof(serveraddr));
					serveraddr.sin_family = AF_INET;
					serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
					serveraddr.sin_port = htons(SERVER_PORT);

					/* bind: asociar un puerto al socket padre */
					if (bind(sockfd, (struct sockaddr *) &serveraddr,
						 sizeof(serveraddr)) < 0) {
									perror("ERROR on binding");
									exit(0);
					}

					/* listen: Hacer que el socket escuche peticiones */
					if (listen(sockfd, BACKLOG) < 0) {
									perror("ERROR on listen");
									exit(0);
					}

					/* Esperar a la conexion */
					while(1) {  // accept() loop principal
						clientlen = sizeof(clientaddr);
						new_fd = accept(sockfd, (struct sockaddr *)&clientaddr,&clientlen);

						if (new_fd == -1) {
							perror("ERROR on accept");
							exit(0);
						}

						/* gethostbyaddr: Determinar quién envió el mensaje */
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
						// Dirección IP del cliente y puerto asignado
						printf("Received request from Client: %s : %d\n", hostaddrp, SERVER_PORT);

						if (!fork()) { // Proceso hijo
							close(sockfd); // El hijo no necesita escuchar
							while(1){
								seconds = time(NULL) + 2208988800;
								seconds = htonl(seconds);
								n = send(new_fd, &seconds, sizeof(int), 0);
								if (n == -1) {
									perror("ERROR on send");
									exit(0);
								}
								// Dormir un segundo
								sleep(1);
							}
							close(new_fd);
							exit(0);
						}
						close(new_fd);
					}
					close(sockfd);  // El padre no necesita esto
		    }

	return(0);
}
