// client code for UDP socket programming 
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

// funtion to clear buffer 
void clearBuf(char* b) 
{ 
	int i; 
	for (i = 0; i < NET_BUF_SIZE; i++) 
		b[i] = '\0'; 
} 

// function to receive file 
int recvFile(char* buf, int s, FILE *fp) 
{ 
	int i; 
	char ch; 
	for (i = 0; i < s; i++) { 
		ch = buf[i]; 
		if (ch == EOF) {
		//	fputc(EOF, fp);
			return 1; 
		} else {
			//fputc(ch, fp);
			//fwrite(ch, sizeof(char), 1, fp);
			//printf("%c", ch); 
		}
	} 
	//printf("'\n");
	return 0; 
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

// driver code 
int main(int argc, char* argv[]) 
{ 
	if (argc < 2){
        fprintf(stderr, "Usage: %s <ip address>\n", argv[0]);
        exit(1);
    }

	// Inisiasi
	int sockfd, cmdsock, nBytes; 
	struct sockaddr_in addr_con, addr_cmd; 
	int addrlen = sizeof(addr_con); 
	addr_con.sin_family = AF_INET; 
	addr_con.sin_port = htons(PORT_NO); 
	addr_con.sin_addr.s_addr = inet_addr(argv[1]); // ./client 192.162.1.10

	addr_cmd.sin_family = AF_INET; 
	addr_cmd.sin_port = htons(PORT_CMD); 
	addr_cmd.sin_addr.s_addr = inet_addr(argv[1]); // ./client 192.162.1.10
	char net_buf[NET_BUF_SIZE]; 
	char * datanum;
	FILE* fp; 

	// socket() 
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 
	cmdsock = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

	if (sockfd < 0) 
		printf("Creating Socket Failed!\n"); 
	else
		printf("Socket Data Created\n"); 
	if (cmdsock < 0) 
		printf("Creating Socket Failed!\n"); 
	else
		printf("Socket Command Created\n"); 
	
	// Set timeout
	struct timeval tv, t1, t2;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	// if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	// 	perror("Error");
	// 	printf("TimeOut!!\n");
	// }

	int logined = 0;
	while (1) {
		// logining	
		while(logined==0){
			printf("Please enter key: ");
			char key[8];
			scanf("%s", key);
			char key_parse[32];
			strcpy(key_parse, "userlogin_");
			strcat(key_parse, key);
			sendto(sockfd, key_parse, 32, 
				sendrecvflag, (struct sockaddr*)&addr_con, 
				addrlen);
			clearBuf(net_buf); 
			nBytes = recvfrom(sockfd, net_buf, NET_BUF_SIZE, sendrecvflag, 
						(struct sockaddr*)&addr_con, &addrlen); 
			//printf("%s||\n", net_buf);
			if (strcmp("200", net_buf)==0){
				printf("Login Accepted! Client IP:\n");
				logined=1;
			}
			if (strcmp("301", net_buf)==0){
				printf("Wrong Key! or Session Ended\n");
				logined=0;
			}
		}

		printf("\nPlease enter file name to receive:\n"); 
		char file_name[NET_BUF_SIZE];
		scanf("%s", file_name); 
		strcpy(net_buf, "file_");
		strcat(net_buf, file_name);
		
		// save file
		char file_path[40];
		printf("Save file: ");
		scanf("%s", file_path);
		char *file_full_path;
		file_full_path = (char *) malloc((4+sizeof(file_path))*sizeof(char));
		strcpy(file_full_path, "out/");
		strcpy(file_full_path+4, file_path);
		fp = fopen(file_full_path, "w");
		if (fp == NULL) 
			printf("File open failed!\n"); 
		else
			printf("File Successfully opened!\n"); 
		//*/

		sendto(sockfd, net_buf, NET_BUF_SIZE, 
			sendrecvflag, (struct sockaddr*)&addr_con, 
			addrlen); 
		
		printf("\n---------Data Received---------\n"); 
		int i = 0;
			// receive 
		clearBuf(net_buf); 
		//printf("variable mulai\n");
		char * datarcv;
		datarcv = (char *) malloc( (NET_BUF_SIZE+4)*sizeof(char));
		//printf("variable done\n");
		nBytes = recvfrom(sockfd, datarcv, (NET_BUF_SIZE+4), sendrecvflag, 
						(struct sockaddr*)&addr_con, &addrlen);
		while ( 1 ) { 
			//printf("Masuk while");
			if (strcmp(datarcv, "302")==0){
				printf("Client Not Logined!\n");
				logined = 0;
				break;
			} else	if (strcmp(datarcv, "400") == 0){
				printf("Sending Finished\n");
				break;
			}
			else{
				/*/ debuging
				if (printPengiriman(datarcv, i, (sizeof(net_buf)+4) ) ){
					break;
				} //*/
				i++;
				//printf("data #%d len:%d", i, nBytes);
				datanum = (char*) malloc( 4*sizeof(char));
				memcpy(datanum, datarcv+(nBytes-4), 4);
				int x = atoi(datanum);
				if (x>0){
					printf("ACK%d  datalength=%d\n", x, nBytes);
					memcpy(net_buf, datarcv, (nBytes-4));
				} else 
					printf("Time Out!\n");

				while(x != i){
					printf("x=%d i=%d\n", x, i);
					printf("-> NCK%s\n", datanum);
					clearBuf(net_buf);
					strcpy(net_buf,"NCK");
					memcpy(net_buf + 5, datanum, 4);
					printf("send NACK\n");
					sendto(cmdsock, net_buf, 9, sendrecvflag,(struct sockaddr*)&addr_cmd, addrlen); 

					gettimeofday(&t1, NULL);
					bzero(datarcv, NET_BUF_SIZE+4);
					nBytes = recvfrom(sockfd, datarcv, (NET_BUF_SIZE+4), sendrecvflag, 
									(struct sockaddr*)&addr_con, &addrlen);
					gettimeofday(&t2, NULL);

					float TO =(t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
					TO += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

					datanum = (char*) malloc( 4*sizeof(char));
					memcpy(datanum, datarcv+(nBytes-4), 4);
					x = atoi(datanum);
					if (x>0){
						printf("ACK%d  datalength=%d\n", x, nBytes);
						memcpy(net_buf, datarcv, (nBytes-4));
					} else 
						printf("Time Out!\n");

					if (TO>95){
						printf("Time Out!\n");
					// 	printf("NCK%s\n", datanum);
					// 	clearBuf(net_buf);
					// 	memcpy(net_buf,"NCK", 3);
					// 	memcpy(net_buf + 3, datanum, 4);
					// 	sendto(sockfd, net_buf, 7, sendrecvflag,(struct sockaddr*)&addr_con, addrlen); 
					}
					
				}
					// if () {
						printf("-> ACK%s\n", datanum);
						fwrite(net_buf, sizeof(char), nBytes-4, fp);
						clearBuf(net_buf);
						strcpy(net_buf,"ACK");
						memcpy(net_buf + 5, datanum, 4);
						printf("Sending ACK\n");
						sendto(cmdsock, net_buf, 9, sendrecvflag,(struct sockaddr*)&addr_cmd, addrlen);
						// break;
					// }
				bzero(datarcv, NET_BUF_SIZE+4);
				nBytes = recvfrom(sockfd, datarcv, (NET_BUF_SIZE+4), sendrecvflag, (struct sockaddr*)&addr_con, &addrlen);
			}
		} 
		printf("\n-------------------------------\n"); 
		fclose(fp);
	} 
	return 0; 
} 
