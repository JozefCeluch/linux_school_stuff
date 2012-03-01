#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[])
{	
	if (argc < 2){
		fprintf(stderr, "Usage: %s <address>\n", argv[0]);
		return 1;
	}
	int res;
	char dst[16];
	char dst6[16];
	struct sockaddr_in *src;
	struct sockaddr_in6 *src6;
	struct addrinfo *result;
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	if ( (res=getaddrinfo(argv[1], "80", NULL, &result)) != 0) {
		fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(res));
		if (res == EAI_SYSTEM){
			perror("getaddrinfo()");
		}
		return 1;
	}
	
	while (result != NULL){
		if ((result->ai_family == AF_INET) || (result->ai_family == AF_UNSPEC)) {
			src = (struct sockaddr_in *)result->ai_addr; 
			inet_ntop(AF_INET, &src->sin_addr, dst, result->ai_addrlen);
			printf("IPv4: %s\n", dst);	
		}
		if ((result->ai_family == AF_INET6) || (result->ai_family == AF_UNSPEC)){
			src6 = (struct sockaddr_in6 *)result->ai_addr;
			inet_ntop(AF_INET6, &src6->sin6_addr, dst6, result->ai_addrlen);
			printf("IPv6: %s\n", dst6);
		}
		
		if ((res=getnameinfo(result->ai_addr, result->ai_addrlen,
				host, NI_MAXHOST,serv, NI_MAXSERV, NI_NAMEREQD)) == 0) {
			printf("domain: %s, service: %s\n", host, serv);
		} else {
			fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(res));
		}
		result = result->ai_next;
	}
	
	return 0;	
}
