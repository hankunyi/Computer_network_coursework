#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sstream>

#define SERVERPORT "30567" // the port client will be connecting to
#define MAXBUFLEN 100 // max number of bytes we can get at once
#define BACKLOG 10    	// how many pending connections queue will hold

int main(int argc, char *argv[])
{
	int patient_num=0;
	int pid;

	while(patient_num<4){
		if((pid=fork()) == 0){

			
			const char* SELFPORT;
			if(patient_num==0) SELFPORT="51567";
			if(patient_num==1) SELFPORT="52567";
			if(patient_num==2) SELFPORT="53567";
			if(patient_num==3) SELFPORT="54567";
			/* bind to self UDP port*/ 
			int selfsockfd; // listen on sock_fd
			struct addrinfo selfhints, *selfservinfo;
			memset(&selfhints, 0, sizeof selfhints);
			selfhints.ai_family = AF_UNSPEC;//family type of socket
			selfhints.ai_socktype = SOCK_DGRAM;//socket type udp protocol
			selfhints.ai_flags = AI_PASSIVE; //fill in IP
			int selfrv;
			// get address information
			if ((selfrv = getaddrinfo(NULL, SELFPORT, &selfhints, &selfservinfo)) != 0) {		
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(selfrv));
				return 1;
			}
			// get the socket file descriptor
			if ((selfsockfd = socket(selfservinfo->ai_family, selfservinfo->ai_socktype, selfservinfo->ai_protocol)) == -1) {
 				perror("server: socket");
			}
			// associate socket with a port
 			if (bind(selfsockfd, selfservinfo->ai_addr, selfservinfo->ai_addrlen) == -1) {
 				close(selfsockfd);
 				perror("server: bind");
 			}
 			freeaddrinfo(selfservinfo); // all done with this structure















			/*construct UDP socket to healthcenter*/ 
			int sockfd, numbytes;
			struct addrinfo hints, *servinfo;
			int rv;
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;	 //family type of socket
			hints.ai_socktype = SOCK_DGRAM; //socket type udp protocol
			hints.ai_flags = AI_PASSIVE; 	 //use my IP
			// get address information
			if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				return 1;
			}
			// get the socket file descriptor
		 	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
				perror("client: socket");
		 	}

			/*construct TCP socket*/
			int tcpsockfd, new_fd; // listen on sock_fd, new connection on new_fd
			struct addrinfo tcphints, *tcpservinfo;
			memset(&tcphints, 0, sizeof tcphints);
			tcphints.ai_family = AF_UNSPEC;//family type of socket
			tcphints.ai_socktype = SOCK_STREAM;//socket type tcp protocol
			tcphints.ai_flags = AI_PASSIVE; //fill in IP
			struct sockaddr_storage tcptheir_addr; // connector's address information
			socklen_t tcpaddr_size;
			int yes=1;
			int tcprv;
			// get address information
			if ((tcprv = getaddrinfo(NULL, "0", &tcphints, &tcpservinfo)) != 0) {		
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(tcprv));
				return 1;
			}
			// get the socket file descriptor
			if ((tcpsockfd = socket(tcpservinfo->ai_family, tcpservinfo->ai_socktype, tcpservinfo->ai_protocol)) == -1) {
 				perror("server: socket");
			}
			// reuse the port
 			if (setsockopt(tcpsockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
 				perror("setsockopt");
 				exit(1);
 			}
			// associate socket with a port
 			if (bind(tcpsockfd, tcpservinfo->ai_addr, tcpservinfo->ai_addrlen) == -1) {
 				close(tcpsockfd);
 				perror("server: bind");
 			}
 			freeaddrinfo(tcpservinfo); // all done with this structure
			// listen other connection
			if (listen(tcpsockfd, BACKLOG) == -1) {
 				perror("listen");
 				exit(1);
 			}
			struct sockaddr_in sin;
			socklen_t len = sizeof(sin);
			if (getsockname(tcpsockfd, (struct sockaddr *)&sin, &len) == -1)
    				perror("getsockname");
    			//printf("port number %d\n", ntohs(sin.sin_port));

			std::stringstream tcpport;	
			tcpport <<  ntohs(sin.sin_port);


			//send information to healthcenter by UDP
			char hostname[MAXBUFLEN];
			gethostname(hostname, sizeof hostname);

			struct hostent *he;
			struct in_addr **addr_list;
			he = gethostbyname(hostname);
			addr_list=(struct in_addr **)he->h_addr_list;
			std::string ipaddr(inet_ntoa(*addr_list[0]));
			std::string str="";
		
			/* Patient information*/
			if(patient_num==0)  str="patient1 " +  ipaddr + " " + tcpport.str()  + " Doctor2";
			else if(patient_num==1)  str="patient2 " +  ipaddr + " " + tcpport.str()  + " Doctor1";
			else if(patient_num==2)  str="patient3 " +  ipaddr + " " + tcpport.str()  + " Doctor2";
			else if(patient_num==3)  str="patient4 " +  ipaddr + " " + tcpport.str()  + " Doctor2";


			const char *senddata = str.c_str();

	
			if ((numbytes = sendto(sockfd, senddata, strlen(senddata), 0,servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		 		perror("talker: sendto");
		 		exit(1);
		 	}
		 	freeaddrinfo(servinfo);
		 	//printf("patient: sent %d bytes to healthcenter\n", numbytes);
		 	close(sockfd);

			
			//receive information from doctor by TCP
 			tcpaddr_size = sizeof tcptheir_addr;
			// new socket descripte for connection from client
 			new_fd = accept(tcpsockfd, (struct sockaddr *)&tcptheir_addr, &tcpaddr_size);
 			if (new_fd == -1) {
 				perror("accept");
 				continue;
 			}
 			if (!fork()) { // this is the child process
				int tcpnumbytes;
				char tcpbuf[MAXBUFLEN];
				int info_num = 0;
				std::string patientinfo[3];


				close(tcpsockfd); // child doesn't need the listener


				// send infromation to client
 				if (send(new_fd, "ACK", 3, 0) == -1) {
					perror("send");
				}
				//printf("Patient%d send:ACK\n",patient_num+1);


				// recieve information from client
				if ((tcpnumbytes = recv(new_fd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 					perror("recv");
 					exit(1);
 				}
 				tcpbuf[tcpnumbytes] = '\0';
				std::string doctor_info(tcpbuf);
 				printf("patient%d: %s\n",patient_num+1,tcpbuf);


				if (send(new_fd, "ACK", 3, 0) == -1) {
					perror("send");
				}
				//printf("patient%d send:ACK\n", patient_num+1);

				printf("Patient%d joined doctor%c.\n", patient_num+1,tcpbuf[17]);


				if ((tcpnumbytes = recv(new_fd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 					perror("recv");
 					exit(1);
 				}
 				tcpbuf[tcpnumbytes] = '\0';
				if(tcpnumbytes!=0){
 					printf("patient%d: %s\n",patient_num+1,tcpbuf);
					if (send(new_fd, "ACK", 3, 0) == -1) {
						perror("send");
					}
					//printf("patient%d send:ACK\n",patient_num+1);

					std::istringstream ss(tcpbuf);
					std::string to;
					while(std::getline(ss,to,'\n')){
						patientinfo[info_num] = to;
						info_num++;						
					}

				}

				if ((tcpnumbytes = recv(new_fd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 					perror("recv");
 					exit(1);
 				}
 				tcpbuf[tcpnumbytes] = '\0';
				std::string schedule(tcpbuf);
				if(tcpnumbytes!=0){
 					printf("patient%d: %s\n",patient_num+1,tcpbuf);
					if (send(new_fd, "ACK", 3, 0) == -1) {
						perror("send");
					}
					//printf("patient%d send:ACK\n",patient_num+1);
				}
 				
				//printf("server: send OK");



				//send information to other patient if info_num>0 by TCP
				if(info_num!=0){


					/*
					for(int i=0;i<info_num;i++){
						printf("patient info:%s \n", patientinfo[i].c_str());
					}
					printf("schedule:%s",schedule.c_str());
					*/

					std::string s = patientinfo[0];
					std::istringstream iss(s);
					std::string tcptcpport;
					iss>>tcptcpport;
					iss>>tcptcpport;
					const char* port = tcptcpport.c_str();
					//printf("TCPPORT is %s", port);

					int tcptcpsockfd, tcptcpnumbytes;
					char tcptcpbuf[MAXBUFLEN];
					struct addrinfo tcptcphints, *tcptcpservinfo;
					int tcptcprv;
					memset(&tcptcphints, 0, sizeof tcptcphints);
					tcptcphints.ai_family = AF_UNSPEC;	 //family type of socket
					tcptcphints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
					tcptcphints.ai_flags = AI_PASSIVE; 	 //use my IP
					// get address information
					if ((tcptcprv = getaddrinfo(NULL, port, &tcptcphints, &tcptcpservinfo)) != 0) {
						fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(tcptcprv));
						return 1;
					}
					// get the socket file descriptor
 					if ((tcptcpsockfd = socket(tcptcpservinfo->ai_family, tcptcpservinfo->ai_socktype, tcptcpservinfo->ai_protocol)) == -1) {
						perror("client: socket");
 					}
					// connect to server
 					if (connect(tcptcpsockfd, tcptcpservinfo->ai_addr, tcptcpservinfo->ai_addrlen) == -1) {
 						close(tcptcpsockfd);
 						perror("client: connect");
 					}	
 					freeaddrinfo(tcptcpservinfo); // all done with this structure



					if ((numbytes = recv(tcptcpsockfd, tcptcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
					tcptcpbuf[numbytes] = '\0';
 					printf("patient%d: %s\n",patient_num+1,tcptcpbuf);

					
					std::string ack="ACK";			
					// send information to server
					if(ack==tcptcpbuf){
						if (send(tcptcpsockfd, doctor_info.c_str(), strlen(doctor_info.c_str()), 0) == -1) {
							perror("send");
						}
					}





					// receive message from server
 					if ((numbytes = recv(tcptcpsockfd, tcptcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcptcpbuf[numbytes] = '\0';
 					printf("patient%d: %s\n",patient_num+1,tcptcpbuf);


					if(ack==tcptcpbuf){
						std::string sendmsg;
						for(int i=1; i<info_num;i++){
							sendmsg += patientinfo[i] + "\n";
						}
					
						if (send(tcptcpsockfd, sendmsg.c_str(), strlen(sendmsg.c_str()), 0) == -1) {
							perror("send");
						}
					}


					
					if ((numbytes = recv(tcptcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcptcpbuf[numbytes] = '\0';
 					//printf("patient%d recieve: %s\n",patient_num+1,tcptcpbuf);
					if(ack==tcptcpbuf){
						std::string info;
						std::istringstream ss(schedule.c_str());
						std::string to;
						std::getline(ss,to,'\n');
						while(std::getline(ss,to,'\n')){
							info += to + "\n";
						}
						if (send(tcptcpsockfd, info.c_str(), strlen(info.c_str()), 0) == -1) {
							perror("send");
						}
					}

					if ((numbytes = recv(tcptcpsockfd, tcptcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcptcpbuf[numbytes] = '\0';
 					//printf("Doctor2 recieve: %s\n",tcptcpbuf);

					close(tcptcpsockfd);
 					return 0;

				}

 			close(new_fd);
 			}
 	
			break;
			}
		else patient_num++;
	}

	wait(NULL);

}
