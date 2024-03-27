#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wu_http.h"
#include "wu_send.h"
#include "wu_date.h"
#include "wu_response.h"

extern FILE *fp_wu_http;
extern FILE *fp_wu;

void
req_get_send_hdr(int susr, struct hdr_nv_resp hdrnv[32])
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

int
req_get_send_resp_img(int susr, struct hdr_nv_resp hdrnvresp[32], char *tosend)
{
	int fd, ret;
	unsigned int nbytesent;
	char buf[1024];
	struct stat statbuf;

	ret = stat(tosend, &statbuf);
	if (ret < 0) {
		perror("stat");
		return -1;
	}

	sprintf(hdrnvresp[3].value, "%u", statbuf.st_size);

	if (send(susr, "HTTP/1.1 200 Ok\r\n", 17, 0) < 0) {
		perror("send");
		return -1;
	}

	req_get_send_hdr(susr, hdrnvresp);

	fd = open(tosend, O_RDONLY);
	if (fd < 0) {
		perror("send");
		return -1;
	}

	for (ret = 1; ret > 0; ) {
		ret = read(fd, buf, 1024);
		ret = send(susr, buf, ret, 0);
		nbytesent += ret;
	}

	close(fd);

	return nbytesent;
}

int
req_get_send_resp_page(int susr, struct hdr_nv_resp hdrnvresp[32], char *tosend)
{
	int nbytesent = 0;
	char *buf1, *buf2;
	char *pchr;
	struct stat statbuf;
	int ret;
	char *loginname;
	char minhr[10];
	size_t buf2size;
	int fd;
	
	pchr = strrchr(tosend, '/');
	pchr++;
	if (strcmp(pchr, "index") == 0) {
		ret = stat(tosend, &statbuf);
		if (ret < 0) {
			perror("stat");
			return -1;
		}
		buf1 = malloc(statbuf.st_size + 1);
		memset(buf1, 0, statbuf.st_size + 1);

		fd = open(tosend, O_RDONLY);
		if (fd < 0) {
			perror("open");
			return -1;
		}
		read(fd, buf1, statbuf.st_size);
		close(fd);

		loginname = getlogin();
		wu_display_minhr(minhr);

		buf2size = statbuf.st_size + strlen(minhr) + strlen(loginname);
		buf2 = malloc(buf2size);
		memset(buf2, 0, buf2size);

		sprintf(buf2, buf1, loginname, minhr);
		sprintf(hdrnvresp[3].value, "%u", strlen(buf2));
		
		if (send(susr, "HTTP/1.1 200 Ok\r\n", 17, 0) < 0) {
			perror("send");
			return -1;
		}

		req_get_send_hdr(susr, hdrnvresp);

		nbytesent = send(susr, buf2, buf2size, 0);

	} else if (strcmp(pchr, "credits") == 0 ||
		   strcmp(pchr, "settings") == 0) {
		ret = stat(tosend, &statbuf);
		if (ret < 0) {
			perror("stat");
			return -1;
		}
		buf1 = malloc(statbuf.st_size + 1);
		memset(buf1, 0, statbuf.st_size + 1);

		fd = open(tosend, O_RDONLY);
		if (fd < 0) {
			perror("open");
			return -1;
		}
		read(fd, buf1, statbuf.st_size);
		close(fd);

		wu_display_minhr(minhr);

		buf2size = statbuf.st_size + strlen(minhr);
		buf2 = malloc(buf2size);
		memset(buf2, 0, buf2size);

		sprintf(buf2, buf1, minhr);
		sprintf(hdrnvresp[3].value, "%u", strlen(buf2));
		
		if (send(susr, "HTTP/1.1 200 Ok\r\n", 17, 0) < 0) {
			perror("send");
			return -1;
		}

		req_get_send_hdr(susr, hdrnvresp);

		nbytesent = send(susr, buf2, buf2size, 0);


	}

	return nbytesent;
}
