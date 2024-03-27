#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wu_http.h"
#include "wu_response.h"
#include "wu_content.h"

extern const struct wu_resource wu_res[19];


void
req_get_set_content_length(char *tosend, char *value)
{
	int err;
	struct stat statbuf;

	/* printf("tosend: |%s|\n", tosend); */
	err = stat(tosend, &statbuf);
	if (err < 0) {
		perror("stat");
		return;
	}	

	sprintf(value, "%ul", statbuf.st_size);

	return;
}

void
req_get_create_hdr_nv(struct hdr_nv_resp hdrnv[32], char *type)
{
	int cnt = 0;
	time_t now = time(0);

	hdrnv[cnt].name = HTTP_HEADER_CACHE_CONTROL;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CACHE_CONTROL_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_CONNECTION;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CONNECTION_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_SERVER;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_SERVER_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_CONTENT_LENGTH;
	memset(hdrnv[cnt++].value, 0, 254);

	hdrnv[cnt].name = HTTP_HEADER_CONTENT_TYPE;
	strcpy(hdrnv[cnt++].value, type);

	hdrnv[cnt].name = HTTP_HEADER_CONTENT_LANGUAGE;
	strcpy(hdrnv[cnt++].value, HTTP_HEADER_CONTENT_LANGUAGE_VALUE);

	hdrnv[cnt].name = HTTP_HEADER_DATE;
	strftime(hdrnv[cnt++].value, HTTP_STRING_DATE_SIZE, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

	return;
}

char*
req_get_create_response(char *res, struct hdr_nv_resp hdrnv[32], short dark, char type[10])
{
	int cnt;
	char *tosend;

	memset(hdrnv, 0, sizeof(struct hdr_nv_resp) * 32);
	
	for (cnt = 0; cnt < 19; cnt++) {
		if (strcmp(wu_res[cnt].resource, res) == 0)
			break;
	}

	if (dark > 0) {
		tosend = (char*) wu_res[cnt].local_resource_dark;
	} else {
		tosend = (char*) wu_res[cnt].local_resource_light;
	}

	memset(type, 0, 10);
	strcpy(type, wu_res[cnt].type);

	req_get_create_hdr_nv(hdrnv, type);
	
	return tosend;
}
