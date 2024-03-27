#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "wu_http.h"

void
req_post_send_hdr(int susr, struct hdr_nv_resp hdrnv[32])
{
	int cnt = 0;

	for (cnt = 0; hdrnv[cnt].name != NULL; cnt++) {
		send(susr, hdrnv[cnt].name, strlen(hdrnv[cnt].name), 0);
		send(susr, ": ", 2, 0);
		send(susr, hdrnv[cnt].value, strlen(hdrnv[cnt].value), 0);
		send(susr, "\r\n", 2, 0);
	}

	send(susr, "\r\n", 2, 0);

	return;
}

void
req_post_theme_hdr_nv(struct hdr_nv_resp hdrnv[32], char *cookie)
{
	int cnt = 0;
	time_t now = time(NULL);
	struct tm *tmv;
	char expires[HTTP_STRING_DATE_SIZE];

	hdrnv[cnt].name = HTTP_HEADER_CACHE_CONTROL;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CACHE_CONTROL_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_CONNECTION;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CONNECTION_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_SERVER;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_SERVER_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_CONTENT_LENGTH;
	strcpy(hdrnv[cnt++].value, "0");

	hdrnv[cnt].name = HTTP_HEADER_CONTENT_LANGUAGE;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CONTENT_LANGUAGE_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_SET_COOKIE;
	tmv = gmtime(&now);
	tmv->tm_year++;
	strftime(expires, HTTP_STRING_DATE_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tmv);

	strcpy(hdrnv[cnt].value, cookie);
	strcat(hdrnv[cnt].value, "; Expires=");
	strcat(hdrnv[cnt++].value, expires);

	hdrnv[cnt].name = HTTP_HEADER_LOCATION;
	strcpy(hdrnv[cnt].value, "/");

	return;
}


int
apply_theme(int susr, char *theme)
{
	struct hdr_nv_resp hdrnv[32];

	printf("Changing theme.\n");

	memset(hdrnv, 0, sizeof(struct hdr_nv_resp) * 32);

	req_post_theme_hdr_nv(hdrnv, theme);

	if (send(susr, "HTTP/1.1 301 Moved Permanently\r\n", 31, 0) <= 0)
		return -1;
	
	req_post_send_hdr(susr, hdrnv);


}
