/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT "26567" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	 //family type of socket
	hints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
	hints.ai_flags = AI_PASSIVE; 	 //use my IP
	// get address information
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// get the socket file descriptor
 	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
		perror("client: socket");
 	}
	// connect to server
 	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
 		close(sockfd);
 		perror("client: connect");
 	}	
 	freeaddrinfo(servinfo); // all done with this structure
	
	// send information to server
	if (send(sockfd, "Hankun Yi", 9, 0) == -1) {
		perror("send");
	}
	// receive message from server
 	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
 		perror("recv");
 		exit(1);
 	}
 	buf[numbytes] = '\0';
 	printf("%s\n",buf);
 	close(sockfd);
 	return 0;
}
