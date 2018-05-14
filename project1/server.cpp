/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>

#define PORT "26567" 	// the port connect to
#define BACKLOG 10    	// how many pending connections queue will hold
#define MAXDATASIZE 100	// the maximum size of message

int main(void)
{
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;//family type of socket
	hints.ai_socktype = SOCK_STREAM;//socket type tcp protocol
	hints.ai_flags = AI_PASSIVE; //fill in IP
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t addr_size;
	int yes=1;
	int rv;
	// get address information
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {		
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// get the socket file descriptor
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
 		perror("server: socket");
	}
	// reuse the port
 	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
 		perror("setsockopt");
 		exit(1);
 	}
	// associate socket with a port
 	if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
 		close(sockfd);
 		perror("server: bind");
 	}
 	freeaddrinfo(servinfo); // all done with this structure
	// listen other connection
	if (listen(sockfd, BACKLOG) == -1) {
 		perror("listen");
 		exit(1);
 	}
 	//waiting for connection
 	while(1) { // main accept() loop
 		addr_size = sizeof their_addr;
		// new socket descripte for connection from client
 		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
 		if (new_fd == -1) {
 			perror("accept");
 			continue;
 		}
 		if (!fork()) { // this is the child process
			int numbytes;
			char buf[MAXDATASIZE];
			close(sockfd); // child doesn't need the listener
			// recieve information from client
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
 				perror("recv");
 				exit(1);
 			}
 			buf[numbytes] = '\0';
 			printf("%s\n",buf);
 			// send infromation to client
 			if (send(new_fd, "OK!", 3, 0) == -1) {
				perror("send");
			}
			//printf("server: send OK");
 			close(new_fd);
 			exit(0);
 		}
 	close(new_fd); // parent doesn't need this
	}
return 0;
}
