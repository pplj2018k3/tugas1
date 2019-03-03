// server code for UDP socket programming 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/time.h>

#define IP_PROTOCOL 0 
#define PORT_NO 15050 
#define PORT_CMD 15049
#define NET_BUF_SIZE 1024 
#define sendrecvflag 0 
#define nofile "File Not Found!" 

// funtion to clear buffer 
void clearBuf(char* b) 
{ 
	int i; 
	for (i = 0; i < NET_BUF_SIZE; i++) 
		b[i] = '\0'; 
} 

int printPengiriman(char* net_buf, int i, int len){
	char mau_print[NET_BUF_SIZE];
	int x=0;
	for (x; x<len; x++){
		if (net_buf[x] == EOF){
			mau_print[x-1] = '\0';
			printf("data #%d: '%s'\n", i, mau_print);
			return 1; 
		}
		else
			mau_print[x] = net_buf[x];
	}
	mau_print[len] = '\0';
	printf("data #%d: '%s'\n", i, mau_print);
	return 0;
}
int cekCommand(char* buff){
	char test[6];
	memset(test, '\0', sizeof(test));
	strncpy(test, buff, 5);
	char key_parse[11];
	memset(key_parse, '\0', sizeof(key_parse));
	strncpy(key_parse, buff, 10);
	if (strcmp(key_parse, "userlogin_") == 0){
		char key[4];
		memset(key, '\0', sizeof(key));
		char* ps = buff+10;
		strcpy(key, ps);
		if (strcmp(key, "123")==0)
			return 1;
		else 
			return 301;
	}
	if (strcmp(test, "file_")==0)
		return 2;
	return 0;
}

int isLogin(struct in_addr ip, struct in_addr list){
	int found = 0;
	for (int i=0; i< sizeof(list); i++){
		if (ip.s_addr==list.s_addr){
			found = 1;
		}
	}
	return found;
}

void listIP(struct in_addr ip){
	char str[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &ip.s_addr, str, INET_ADDRSTRLEN );
	printf("%s\n", str);
}

// driver code 
int main() 
{ 
	int sockfd, nBytes, cmdsock; 
	struct sockaddr_in addr_con; 
	struct sockaddr_in addr_cmd; 
	int addrlen = sizeof(addr_con); 

	addr_con.sin_family = AF_INET; 
	addr_con.sin_port = htons(PORT_NO); 
	addr_con.sin_addr.s_addr = INADDR_ANY; 

	char net_buf[NET_BUF_SIZE]; 
	FILE* fp; 
	char * datarcv;

	// login
	struct in_addr list;

	// socket() 
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

	if (sockfd < 0) 
		printf("Creating Socket Failed!\n"); 
	else
		printf("Socket Data Created\n"); 

	// bind() 
	if (bind(sockfd, (struct sockaddr*)&addr_con, sizeof(addr_con)) == 0) 
		printf("Successfully binded at PORT=%d!\n", PORT_NO); 
	else
		printf("Binding Port #1 Failed!\n"); 

	cmdsock = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);
	addr_con.sin_port = htons(PORT_CMD); 
	if (cmdsock < 0) 
		printf("Creating Socket Failed!\n"); 
	else
		printf("Socket Command Created\n"); 
		
	if (bind(cmdsock, (struct sockaddr*)&addr_con, sizeof(addr_con)) == 0) 
		printf("Successfully binded at PORT=%d!\n", PORT_CMD); 
	else
		printf("Binding Port #2 Failed!\n"); 

	// Set timeout
	struct timeval tv, t1, t2;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(cmdsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error");
		printf("TimeOut!!\n");
	}

	// Main process
	while (1) { 
		printf("\nWaiting connection\n");
		clearBuf(net_buf); 
		nBytes = recvfrom(sockfd, net_buf, NET_BUF_SIZE, sendrecvflag, 
				 (struct sockaddr*)&addr_con, &addrlen); 

		switch (cekCommand(net_buf)){
			case 1: // Login User
				printf("Userlogin OK!\n");
				list.s_addr = addr_con.sin_addr.s_addr;
				listIP(list);
				clearBuf(net_buf);
				strcpy(net_buf,"200");
				sendto(sockfd, net_buf, 5, sendrecvflag, 
					(struct sockaddr*)&addr_con, addrlen); 
				break;
			case 2: // Sending data
				// Check is User Logined?
				if (!isLogin(addr_con.sin_addr, list)){
					printf("User Not Logined\n");
					clearBuf(net_buf);
					strcpy(net_buf,"302");
					sendto(sockfd, net_buf, 5, sendrecvflag, 
						(struct sockaddr*)&addr_con, addrlen); 
					break;
				}//*/
				
				// Receive file name 
				printf("File Name Recieved!\n");
				char str[32];
				strcpy(str, net_buf+5);
				printf("File name: %s\n", str);
								
				// Sending File				
				fp = fopen(str, "r"); 
				if (fp == NULL) 
					printf("File open failed!\n"); 
				else
					printf("File Successfully opened!\n"); 

				clearBuf(net_buf); 
				int len, counter = 0;
				while( (len = fread(&net_buf, sizeof(char), 1024, fp))>0){
					unsigned char * datasend;
					counter++;
					datasend = (unsigned char *) malloc ((4+len)*sizeof(char));
					char datanum[4];
					sprintf(datanum, "%d", counter);
					memcpy(datasend, net_buf, len);
					memcpy(datasend+len, datanum, 4);
					int nByte = sendto(sockfd, datasend, (4+len), sendrecvflag, 
								(struct sockaddr*)&addr_con, addrlen); 
					// printf("%s\n", datasend);
					printf("Packet:%d\tlength:%d\n", counter, nByte);

					while(1){						
						// printf("buatvariable\n");
						datarcv = (char *) malloc(9*sizeof(char));
						// printf("inisiasi");
						gettimeofday(&t1, NULL);
						nBytes = recvfrom(cmdsock, datarcv, 9, sendrecvflag, 
								(struct sockaddr*)&addr_cmd, &addrlen);
						gettimeofday(&t2, NULL);

						float TO =(t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
						TO += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

						// printf("--\t%s\n", datarcv);
						char * tmp;
						tmp = (char*) malloc(5*sizeof(char));
						strcpy(tmp, datarcv);
						memcpy(datanum, datarcv+5, 4);
						// printf("%s\t %s  %s\n", datarcv, tmp, datanum);
						// printf("%d , %d\n", strcmp(tmp, "NCK"), strcmp(tmp, "ACK"));

						if (strcmp(tmp, "NCK") == 0 || (TO>99) ){
							printf("%s  %s\n", tmp, datanum);
							int nByte = sendto(sockfd, datasend, (4+len), sendrecvflag, 
										(struct sockaddr*)&addr_con, addrlen); 
							printf("Resending packet:%d\tlength:%d\n", counter, nByte);
						} else if (strcmp(tmp, "ACK") == 0){
							printf("-> ACK%s\n", datanum);
							break;
						}
					}
					
				}
				fclose(fp); //*/
				clearBuf(net_buf);
				strcpy(net_buf,"400");
				int nByte = sendto(sockfd, net_buf, 5, sendrecvflag, 
					(struct sockaddr*)&addr_con, addrlen); 
				// printf("%s %d\n", net_buf, nByte);
				break;
			case 301: // Eror Code "301: Wrong Key Login"
				printf("Wrong Key from User!\n");
				clearBuf(net_buf);
				strcpy(net_buf,"301");
				sendto(sockfd, net_buf, 5, sendrecvflag, 
					(struct sockaddr*)&addr_con, addrlen); 
				break;
			default:
				printf("Wrong Command!\n");
				break;
		}
	} 
	return 0; 
} 
