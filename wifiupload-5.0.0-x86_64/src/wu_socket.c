#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "wu_log.h"
#include "wu_date.h"

extern char log_wu_http[512];
extern char log_wu[512];
extern char log_date_now[LOG_DATE_NOW_SIZE];

int
bind_input_addr(struct sockaddr_in *input_addr)
{
	int s;
	struct protoent *pent;

	pent = getprotobyname("tcp");
	if (pent == NULL)
		return -1;

	s = socket(AF_INET, SOCK_STREAM, pent->p_proto);

	if (bind(s, (struct sockaddr*)input_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return -1;
	}
	
	listen(s, 7);

	return s;
}

int
accept_conn(int s)
{
	struct sockaddr_in usr_sin;
	int usr_s;
	socklen_t slen;

	slen = sizeof(struct sockaddr_in);
	memset(&usr_sin, 0, slen);

	usr_s = accept(s, (struct sockaddr*)&usr_sin, &slen);
	if (usr_s < 0) {
		perror("accept");
		return -1;
	}
	
	memset(log_wu_http, 0, 512);
	memset(log_wu, 0, 512);
	
	strcat(log_wu_http, inet_ntoa(usr_sin.sin_addr));
	strcat(log_wu_http, " - - [");

	localtime_now();
	strcat(log_wu_http, log_date_now);
	strcat(log_wu_http, "] ");
	
	return usr_s;
}
