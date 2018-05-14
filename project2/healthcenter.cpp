#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>

#define PORT "30567" 	// the port connect to
#define BACKLOG 10    	// how many pending connections queue will hold
#define MAXBUFLEN 100	// the maximum size of message




int main(void)
{
	int sockfd, numbytes; // listen on sock_fd
	struct addrinfo hints, *servinfo;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;//family type of socket
	hints.ai_socktype = SOCK_DGRAM;//socket type udp protocol
	hints.ai_flags = AI_PASSIVE; //fill in IP
	struct sockaddr_storage their_addr; // connector's address information
	char buf[MAXBUFLEN];
	socklen_t addr_len;
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

	char hostname[MAXBUFLEN];
	gethostname(hostname, sizeof hostname);
	//printf("IP address : %s\n", hostname); 

	struct hostent *he;
	struct in_addr **addr_list;
	he = gethostbyname(hostname);
	//printf(" IP addresses:");
	addr_list=(struct in_addr **)he->h_addr_list;
	for(int i = 0; addr_list[i] != NULL; i++) {
 		//printf("%s ", inet_ntoa(*addr_list[i]));
	}
 	//printf("\n");


	//printf("\nlistener: waiting to recvfrom...\n");

	/* receive patient*/
	char patientmsg[4][MAXBUFLEN];
	int patientnum=0;
	while(patientnum!=4){
	 	addr_len = sizeof their_addr;
	 	if ((numbytes = recvfrom(sockfd, patientmsg[patientnum], MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
	 		perror("recvfrom");
	 		exit(1);
	 	}
	 	// printf("listener: got packet from %s\n",inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
	 	// printf("listener: packet is %d bytes long\n", numbytes);
	 	patientmsg[patientnum][numbytes] = '\0';
	 	printf("healthcenter: %s\n", patientmsg[patientnum]);
		printf("patient%c register successfully \n", patientmsg[patientnum][7]);
		if(patientnum==3) 	printf("Registration of peers completed! Run the doctors!\n");
		patientnum++;
		
	}

	

	char passwd1[256];
	char passwd2[256];
	std::ifstream myfile("healthcenter.txt");
	if(myfile.is_open()){
		myfile.getline(passwd1,256);
		passwd1[13]='\0';
		myfile.getline(passwd2,256);
		passwd2[13]='\0';	
		myfile.close();
	}


	/* send doctor information*/
	char doctormsg[2][MAXBUFLEN];
	int doctornum=0;
	while(doctornum!=2){
	 	addr_len = sizeof their_addr;
	 	if ((numbytes = recvfrom(sockfd, doctormsg[doctornum], MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
	 		perror("recvfrom");
	 		exit(1);
	 	}
	 	// printf("listener: got packet from %s\n",inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
	 	// printf("listener: packet is %d bytes long\n", numbytes);
	 	doctormsg[doctornum][numbytes] = '\0';
	 	printf("healthcenter: %s\n", doctormsg[doctornum]);
		//printf("patient%c register successfully \n", patientmsg[patientnum][7]);

		if(strcmp(doctormsg[doctornum], passwd1)==0){
			printf("doctor1 logged in successfully\n");
			std::string sendmsg("");
			for(int i=0; i<4; i++){
				std::string str(patientmsg[i]);
				std::string str2("Doctor1");
				std::size_t found = str.find(str2);
				if (found!=std::string::npos) sendmsg += str +"\n";
			}
	
				if ((numbytes = sendto(sockfd, sendmsg.c_str(), strlen(sendmsg.c_str()), 0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
			 		perror("talker: sendto");
			 		exit(1);
			 }	

		}
		else if(strcmp(doctormsg[doctornum], passwd2)==0){
			 printf("doctor2 logged in successfully\n");
			
			std::string sendmsg("");
			for(int i=0; i<4; i++){
				std::string str(patientmsg[i]);
				std::string str2("Doctor2");
				std::size_t found = str.find(str2);
				if (found!=std::string::npos) sendmsg += str +"\n";
			}
			if ((numbytes = sendto(sockfd, sendmsg.c_str(), strlen(sendmsg.c_str()), 0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
		 		perror("talker: sendto");
		 		exit(1);
		 	}


		}
		else printf("password error\n");


		doctornum++;
	}


 	close(sockfd);
 	return 0;
}


