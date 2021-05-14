#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAXSIZE 10000

char hostname[100];
char filepath[100] = "/";
char http_request[MAXSIZE];
char buffer[MAXSIZE];

void parsing(char *argv[], char *port) {
	char *ptr;
	int path_cnt = -1, port_cnt = -1;
	
	sscanf(argv[1], "http://%s", hostname);
	ptr = strtok(hostname, "/");
	while(ptr != NULL) {
		if(path_cnt == -1) strcpy(hostname, ptr), path_cnt++;
		else {
			if(path_cnt) strcat(filepath, "/");
			strcat(filepath, ptr);
			path_cnt++;
		}
		ptr = strtok(NULL, "/");
	}

	ptr = strtok(hostname, ":");
	while(ptr!=NULL){
		if(port_cnt == -1) strcpy(hostname, ptr), port_cnt++;
		else strcpy(port, ptr);
		ptr = strtok(NULL, ":");
	}

	return;
}

int getContentLen(char *response) {
	char *ptr;
	char back_up[MAXSIZE];
	int cnt = 0, content_len = -1;
	
	strcpy(back_up, response);
	ptr = strtok(back_up, "\r\n");
	while(ptr != NULL) {
		if(cnt == 0) {
			printf("%s\n", ptr), cnt++;
		} else {
			if(strncmp(ptr, "Content-Length:", 15) == 0) {
				sscanf(ptr, "%*s %d", &content_len);
			}
			else if(strncmp(ptr, "Content-length:", 15) == 0) {
				sscanf(ptr, "%*s %d", &content_len);
			}
			else if(strncmp(ptr, "content-length:", 15) == 0) {
				sscanf(ptr, "%*s %d", &content_len);
			}
		}
		ptr = strtok(NULL, "\r\n");
	}
	return content_len;
}

int main(int argc, char *argv[]) {

	FILE *fp;
	char *file_data, *http_response;
	int sockfd, numbytes = 0;
	char port[8] = "80";
	struct addrinfo hints, *servinfo;
	int rv;
	int content_len = 0, recv_try = 0;
	int n;
	
	http_response = (char *)calloc(MAXSIZE, sizeof(char));
	if(argc != 2 || strlen(argv[1]) < 8) {
		fprintf(stderr, "usage : http_client http://hostname[:port][/path/to/file]\n");
		exit(1);
	} else {;
		if(strncmp(argv[1],"http://", 7) != 0) {
			fprintf(stderr, "usage : http_client http://hostname[:port][/path/to/file]\n");
			exit(1);
		}
	}

	parsing(argv, port);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: Name or service not known\n");
		return 1;
	}

	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror("clent: socket");
		return 2;
	}

	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(sockfd);
		perror("connect");
		exit(1);
	}

	sprintf(http_request, "GET %s HTTP/1.1\r\nHost: %s:%s\r\n\r\n", filepath, hostname, port);
	freeaddrinfo(servinfo);

	if(send(sockfd, http_request, strlen(http_request), 0) == -1) {
		perror("send");
		close(sockfd);
		exit(1);
	}
	
	do {
		n = recv(sockfd, buffer, sizeof buffer, 0);
		if(n == -1) {
			perror("recv");
			close(sockfd);
			exit(1);
		}
		buffer[n] = '\0';
		if(content_len == 0) {
			content_len = getContentLen(buffer);
			http_response = (char *)malloc(sizeof(char) * (content_len + MAXSIZE));
		}
		numbytes += n;
		strcat(http_response, buffer);
	} while(numbytes <= content_len);

	file_data = strstr(http_response, "\r\n\r\n");
	file_data += 4;

	if(content_len == -1) {
		printf("Content-Length not specified.\n");
		close(sockfd);
		return 1;
	} else {
		fp = fopen("20161640.out", "w");
		fprintf(fp, "%s", file_data);
		printf("%d bytes written to 20161640.out\n", content_len);
		fclose(fp);
	}

	close(sockfd);
	free(http_response);
	return 0;
}


