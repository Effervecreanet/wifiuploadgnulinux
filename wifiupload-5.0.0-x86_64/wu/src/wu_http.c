#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "wu_http.h"





int
parse_rline(int susr, struct request_line *rline)
{
	char c;
	int8_t nbyte;
	char *p1;

	memset(rline, 0, sizeof(struct request_line));
	p1 = rline->method;
	do {
		nbyte = recv(susr, &c, 1, 0);	
		if (nbyte <= 0)
			return -1;
		if (c == ' ')
			break;
		*p1++ = c;	
	} while (strlen(rline->method) <= 5);

	p1 = rline->resource;

	do {
		nbyte = recv(susr, &c, 1, 0);
		if (nbyte <= 0)
			return -1;
		else if (c == ' ')
			break;
		*p1++ = c;
	} while(strlen(rline->resource) <= 128);

	if (recv(susr, rline->version, sizeof("HTTP/1.1") - 1, 0) != sizeof("HTTP/1.1") - 1) {
		return -1;
	} else if (strcmp(rline->version, "HTTP/1.1") != 0) {
		return -1;
	}

	c = 0;

	if (recv(susr, &c, 1, 0) != 1 || c != '\r')
		return -1;
	if (recv(susr, &c, 1, 0) != 1 || c != '\n')
		return -1;

	/*
	printf("rline->method: %s\n", rline->method);
	printf("rline->resource: %s\n", rline->resource);
	printf("rline->version: %s\n", rline->version);
	*/

	return 1;
}

int
parse_hdr_nv(int susr, struct hdr_nv hdrnv[32])
{
	int8_t nbyte;
	char c;
	char *p1, *p2;
	unsigned int cnt;
	unsigned short cnt1;


	memset(hdrnv, 0, sizeof(struct hdr_nv) * 32);

	for (cnt1 = 0, cnt = 0; cnt1 < 32; ) {
		for (p1 = (hdrnv + cnt1)->name; cnt < 32; ) {
				if (recv(susr, &c, 1, 0) != 1)
					return -1;
				if (c == ':')
					break;
				*(p1 + cnt++) = c;
		}	

		if (recv(susr, &c, 1, 0) != 1 || c != ' ')
			return -1;

		for (cnt = 0, p2 = (hdrnv + cnt1)->value; cnt < 254; ) {
				if (recv(susr, &c, 1, 0) != 1)
					return -1;
				if (c == '\r')
					break;
				*(p2 + cnt++) = c;
		}

		if (recv(susr, &c, 1, 0) != 1 || c != '\n')
			return -1;

		if (recv(susr, &c, 1, 0) == 1 && c == '\r') {
			if (recv(susr, &c, 1, 0) == 1 && c == '\n') {
				break;
			} else {
				return -1;
			}
		} else {
			*(hdrnv + ++cnt1)->name = c;
			cnt = 1;
		}
	}
	
	

	return 1;
}
