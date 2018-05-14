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
#include <sstream>
#include <fstream>

#define SERVERPORT "30567" // the port client will be connecting to
#define MAXBUFLEN 100 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
	int doctor_num=0;
	int pid;
	char healthmsg[2][MAXBUFLEN];
	int doctor1_num = 0;
	int doctor2_num = 0;
	std::string doctor1msg[4];
	std::string doctor2msg[4];
	
	while(doctor_num<2){
		if((pid=fork()) == 0){


			
			/* bind to self port*/
			const char* SELFPORT;
			if(doctor_num==0) SELFPORT="41567";
			if(doctor_num==1) SELFPORT="42567";
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













			/* UDP to send passwd to healthcenter*/
			int sockfd, numbytes,numbytes2;
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


			char hostname[MAXBUFLEN];
			gethostname(hostname, sizeof hostname);

			struct hostent *he;
			struct in_addr **addr_list;
			he = gethostbyname(hostname);
			addr_list=(struct in_addr **)he->h_addr_list;
			std::string ipaddr(inet_ntoa(*addr_list[0]));
			std::string str="";
		
			if(doctor_num==0)  str="doctor1 aaaaa";
			else if(doctor_num==1)  str="doctor2 bbbbb";
			


			const char *senddata = str.c_str();


	
			if ((numbytes = sendto(sockfd, senddata, strlen(senddata), 0,servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		 		perror("talker: sendto");
		 		exit(1);
		 	}


			/*receive patient info*/
			if ((numbytes2 = recvfrom(sockfd, healthmsg[doctor_num], MAXBUFLEN, 0,servinfo->ai_addr, &(servinfo->ai_addrlen))) == -1) {
		 		perror("talker: sendto");
		 		exit(1);
		 	}

			healthmsg[doctor_num][numbytes2] = '\0';
		 	freeaddrinfo(servinfo);
		 	printf("doctor%d: %s\n", doctor_num+1, healthmsg[doctor_num]);
		 	close(sockfd);


		
			
			std::istringstream ss(healthmsg[doctor_num]);
			std::string to;

			if(healthmsg[doctor_num] != NULL)
			{

				while(std::getline(ss,to,'\n')){
					if(doctor_num == 0){
						doctor1msg[doctor1_num] = to;
						doctor1_num++;
					}
					else{
						doctor2msg[doctor2_num] = to;
						doctor2_num++;
					}
				}
			}

		
			/* send schedule to patient*/
			if(doctor_num == 0){
				std::string doctor1_schedule;
				std::string temp;
				std::ifstream myfile("doctor1.txt");
				while(getline(myfile,temp)){
					doctor1_schedule+= temp + '\n';
				}
		
				//printf("doctor1 schedule: \n%s \n",doctor1_schedule.c_str());
				
				
				if(doctor1_num==0) printf("Doctor1 has no peer subscribers!\n");	
				if(doctor1_num==1) {
					printf("Doctor1 has only one patient subscriber.\n");
					
					
					std::string s = doctor1msg[0];
					std::istringstream iss(s);
					std::string tcpport;
					iss>>tcpport;
					iss>>tcpport;
					iss>>tcpport;
					const char* port = tcpport.c_str();
					//printf("TCPPORT is %s", port);

					int tcpsockfd, tcpnumbytes;
					char tcpbuf[MAXBUFLEN];
					struct addrinfo tcphints, *tcpservinfo;
					int tcprv;
					memset(&tcphints, 0, sizeof tcphints);
					tcphints.ai_family = AF_UNSPEC;	 //family type of socket
					tcphints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
					tcphints.ai_flags = AI_PASSIVE; 	 //use my IP
					// get address information
					if ((tcprv = getaddrinfo(NULL, port, &tcphints, &tcpservinfo)) != 0) {
						fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(tcprv));
						return 1;
					}
					// get the socket file descriptor
 					if ((tcpsockfd = socket(tcpservinfo->ai_family, tcpservinfo->ai_socktype, tcpservinfo->ai_protocol)) == -1) {
						perror("client: socket");
 					}
					// connect to server
 					if (connect(tcpsockfd, tcpservinfo->ai_addr, tcpservinfo->ai_addrlen) == -1) {
 						close(sockfd);
 						perror("client: connect");
 					}	
 					freeaddrinfo(tcpservinfo); // all done with this structure



					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);

					
					std::string ack="ACK";			
					// send information to server
					if(ack==tcpbuf){
					
						if (send(tcpsockfd, "welcome to doctor1 group", 24, 0) == -1) {
							perror("send");
						}
					}

					
					// receive message from server
 					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);
 					close(sockfd);
 					return 0;

				}
				else{
					printf("Doctor1 has %d subscribers!\n",doctor1_num);
					std::string s = doctor1msg[0];
					std::istringstream iss(s);
					std::string tcpport;
					iss>>tcpport;
					iss>>tcpport;
					iss>>tcpport;
					const char* port = tcpport.c_str();
					//printf("TCPPORT is %s", port);

					int tcpsockfd, tcpnumbytes;
					char tcpbuf[MAXBUFLEN];
					struct addrinfo tcphints, *tcpservinfo;
					int tcprv;
					memset(&tcphints, 0, sizeof tcphints);
					tcphints.ai_family = AF_UNSPEC;	 //family type of socket
					tcphints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
					tcphints.ai_flags = AI_PASSIVE; 	 //use my IP
					// get address information
					if ((tcprv = getaddrinfo(NULL, port, &tcphints, &tcpservinfo)) != 0) {
						fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(tcprv));
						return 1;
					}
					// get the socket file descriptor
 					if ((tcpsockfd = socket(tcpservinfo->ai_family, tcpservinfo->ai_socktype, tcpservinfo->ai_protocol)) == -1) {
						perror("client: socket");
 					}
					// connect to server
 					if (connect(tcpsockfd, tcpservinfo->ai_addr, tcpservinfo->ai_addrlen) == -1) {
 						close(sockfd);
 						perror("client: connect");
 					}	
 					freeaddrinfo(tcpservinfo); // all done with this structure


					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);

					
					std::string ack="ACK";			
					// send information to server
					if(ack==tcpbuf){
					
						if (send(tcpsockfd, "welcome to doctor1 group", 24, 0) == -1) {
							perror("send");
						}
					}

					
					// receive message from server
 					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);


					if(ack==tcpbuf){
						std::string sendmsg;
						for(int i=1; i<doctor1_num; i++){
							std::string sa = doctor1msg[i];
							std::istringstream issa(sa);
							std::string str1,str2 ;
							issa>>str1;
							issa>>str2;
							issa>>str2;
							sendmsg+= str1 +" " + str2 + "\n";
							//printf("str1: %s\n", str1.c_str());
							//printf("str2: %s\n", str2.c_str());
						}
					
						if (send(tcpsockfd, sendmsg.c_str(), strlen(sendmsg.c_str()), 0) == -1) {
							perror("send");
						}
					}


					
					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);
					if(ack==tcpbuf){
						if (send(tcpsockfd, doctor1_schedule.c_str(), strlen(doctor1_schedule.c_str()), 0) == -1) {
							perror("send");
						}
					}

					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor1: %s\n",tcpbuf);


 					close(tcpsockfd);
 					return 0;
				}					




			}

			else{

				std::string doctor2_schedule;
				std::string temp;
				std::ifstream myfile("doctor2.txt");
				while(getline(myfile,temp)){
					doctor2_schedule+= temp + '\n';
				}
		
				//printf("doctor2 schedule: \n %s \n",doctor2_schedule.c_str());

				if(doctor2_num==0) printf("Doctor2 has no peer subscribers!\n");	
				if(doctor2_num==1) {
					printf("Doctor2 has only one patient subscriber.\n");
					std::string s = doctor2msg[0];
					std::istringstream iss(s);
					std::string tcpport;
					iss>>tcpport;
					iss>>tcpport;
					iss>>tcpport;
					const char* port = tcpport.c_str();
					//printf("TCPPORT is %s", port);

					int tcpsockfd, tcpnumbytes;
					char tcpbuf[MAXBUFLEN];
					struct addrinfo tcphints, *tcpservinfo;
					int tcprv;
					memset(&tcphints, 0, sizeof tcphints);
					tcphints.ai_family = AF_UNSPEC;	 //family type of socket
					tcphints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
					tcphints.ai_flags = AI_PASSIVE; 	 //use my IP
					// get address information
					if ((tcprv = getaddrinfo(NULL, port, &tcphints, &tcpservinfo)) != 0) {
						fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(tcprv));
						return 1;
					}
					// get the socket file descriptor
 					if ((tcpsockfd = socket(tcpservinfo->ai_family, tcpservinfo->ai_socktype, tcpservinfo->ai_protocol)) == -1) {
						perror("client: socket");
 					}
					// connect to server
 					if (connect(tcpsockfd, tcpservinfo->ai_addr, tcpservinfo->ai_addrlen) == -1) {
 						close(sockfd);
 						perror("client: connect");
 					}	
 					freeaddrinfo(tcpservinfo); // all done with this structure



					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);

					
					std::string ack="ACK";			
					// send information to server
					if(ack==tcpbuf){		
						if (send(tcpsockfd, "welcome to doctor2 group", 24, 0) == -1) {
							perror("send");
						}
					}

					
					// receive message from server
 					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);
 					close(sockfd);
 					return 0;

				}


				else{
					printf("Doctor2 has %d subscribers!\n",doctor2_num);
					std::string s = doctor2msg[0];
					std::istringstream iss(s);
					std::string tcpport;
					iss>>tcpport;
					iss>>tcpport;
					iss>>tcpport;
					const char* port = tcpport.c_str();
					//printf("TCPPORT is %s", port);

					int tcpsockfd, tcpnumbytes;
					char tcpbuf[MAXBUFLEN];
					struct addrinfo tcphints, *tcpservinfo;
					int tcprv;
					memset(&tcphints, 0, sizeof tcphints);
					tcphints.ai_family = AF_UNSPEC;	 //family type of socket
					tcphints.ai_socktype = SOCK_STREAM; //socket type tcp protocol
					tcphints.ai_flags = AI_PASSIVE; 	 //use my IP
					// get address information
					if ((tcprv = getaddrinfo(NULL, port, &tcphints, &tcpservinfo)) != 0) {
						fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(tcprv));
						return 1;
					}
					// get the socket file descriptor
 					if ((tcpsockfd = socket(tcpservinfo->ai_family, tcpservinfo->ai_socktype, tcpservinfo->ai_protocol)) == -1) {
						perror("client: socket");
 					}
					// connect to server
 					if (connect(tcpsockfd, tcpservinfo->ai_addr, tcpservinfo->ai_addrlen) == -1) {
 						close(sockfd);
 						perror("client: connect");
 					}	
 					freeaddrinfo(tcpservinfo); // all done with this structure


					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);

					
					std::string ack="ACK";			
					// send information to server
					if(ack==tcpbuf){
					
						if (send(tcpsockfd, "welcome to doctor2 group", 24, 0) == -1) {
							perror("send");
						}
					}

					
					// receive message from server
 					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);


					if(ack==tcpbuf){
						std::string sendmsg;
						for(int i=1; i<doctor2_num; i++){
							std::string sa = doctor2msg[i];
							std::istringstream issa(sa);
							std::string str1,str2 ;
							issa>>str1;
							issa>>str2;
							issa>>str2;
							sendmsg+= str1 +" " + str2 + "\n";
							//printf("str1: %s\n", str1.c_str());
							//printf("str2: %s\n", str2.c_str());
						}
					
						if (send(tcpsockfd, sendmsg.c_str(), strlen(sendmsg.c_str()), 0) == -1) {
							perror("send");
						}
					}


					
					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);
					if(ack==tcpbuf){
						if (send(tcpsockfd, doctor2_schedule.c_str(), strlen(doctor2_schedule.c_str()), 0) == -1) {
							perror("send");
						}
					}

					if ((numbytes = recv(tcpsockfd, tcpbuf, MAXBUFLEN-1, 0)) == -1) {
 						perror("recv");
 						exit(1);
 					}
 					tcpbuf[numbytes] = '\0';
 					printf("Doctor2: %s\n",tcpbuf);


 					close(tcpsockfd);
 					return 0;
				}					

			}			
			break;
			}
		else doctor_num++;
	}

	wait(NULL);





}
