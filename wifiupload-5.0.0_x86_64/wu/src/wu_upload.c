#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "wu_http.h"
#include "wu_response.h"
#include "wu_content.h"
#include "wu_theme.h"
#include "wu_date.h"

extern const struct wu_resource wu_res[20];
extern FILE *fp_wu;

int
recv_MIME(int susr, char *buffer)
{
	int cnt;
	char c;

	memset(buffer, 0, 2048);
	for (cnt = 0; cnt < 2044; cnt++) {
		if (recv(susr, &c, 1, 0) != 1)
			return -1;
		if (c == '\r') {
			*buffer++ = c;
			if (recv(susr, &c, 1, 0) != 1) {
				return -1;
			}	
			if (c == '\n') {
				*buffer++ = c;
				if (recv(susr, &c, 1, 0) != 1) {
					return -1;
				}
				if (c == '\r') {
					*buffer++ = c;
					if (recv(susr, &c, 1, 0) != 1) {
						return -1;
					}
					if (c == '\n') {
						*buffer = c;
						return cnt + 3;
					}
				}
			}
		}
		*buffer++ = c;

	}


	return -1;
}

int
parse_MIME(char *bufMIME, char *filename)
{
	char *pfilename;
	char *pfilenameend;
	
	memset(filename, 0, 254);
	pfilename = strstr(bufMIME, "filename=\"");
	if (pfilename == NULL)
		return -1;

	pfilename += sizeof("filename=\"") -1;
	if (*pfilename == '\0')
		return -1;
	
	pfilenameend = strchr(pfilename, '\"');
	if (pfilenameend == NULL)
		return -1;
	*pfilenameend = '\0';	

	strncpy(filename, pfilename, 253);

	return 1;
}

unsigned short
checkget_ctype_boundary_len(char *ctype)
{
	char *pboundary;
	unsigned short boundarylen;

	pboundary = strstr(ctype, "boundary=");
	if (pboundary == NULL)
		return 0;

	pboundary += sizeof("boundary=") -1;

	boundarylen  = strlen(pboundary);
	if (boundarylen > 0)
		return boundarylen;
	else
		return 0;
}

void
tohumanreadable(unsigned long long filesize, char *hr)
{
	unsigned char *units[4] = {"o", "Ko", "Mo", "Go"};
	char **punits = (char**)&units;	
	
	memset(hr, 0, 10);
	if (filesize <= 1024) {
		sprintf(hr, "%llu %s", filesize, *punits);
		return;
	}
	
	double d_filesize = filesize;
	for (;;) {
		if (d_filesize <= 1024) {
			sprintf(hr, "%.1f %s", (double)d_filesize, *punits);
			break;
		}
		d_filesize /= 1024;
		punits++;
	}




	return;
}

int
receive_file(int susr, char *filename, unsigned long long filesize, unsigned short boundarylen)
{
	int fd;
	char path[254];
	char buffer[1024];
	int ret;
	unsigned long long filesizebak = filesize;
	char humanreadable1[10];
	char humanreadable2[10];
	char humanreadable3[10];
	char progressbar[100];
	unsigned short percent = 1;
	time_t now;
	unsigned int txcur = 0;

	memset(path, 0, 254);
	strcpy(path, "./Downloads/");
	strcat(path, filename);

	fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	memset(progressbar, ' ', 99);
	progressbar[0] = '|';
	progressbar[99] = '|';
	now = time(0);
	memset(humanreadable3, 0, 10);
	humanreadable3[0] = '-';
	humanreadable3[1] = '-';

	while (filesize > 0) {
		memset(buffer, 0, 1024);
		if (filesize < 1024) {
			ret = recv(susr, buffer, filesize,  0);
			if (ret <= 0) {
				perror("recv");
				return -1;
			}
			if (write(fd, buffer, filesize) <= 0) {
				perror("write");
				return -1;
			}
			break;
		}
		ret = recv(susr, buffer, 1024, 0);
		if (ret <= 0) {
			perror("recv");
			return -1;
		}
		if (write(fd, buffer, ret) <= 0) {
			perror("write");
			return -1;
		}
		filesize -= ret;

		txcur += ret;
		if (time(0) > now) {
			now = time(0);	
			tohumanreadable(txcur, humanreadable3);
			txcur = 0;
		} 

		tohumanreadable(filesizebak - filesize, humanreadable1);
		tohumanreadable(filesizebak, humanreadable2);
		
		if (abs(filesize - filesizebak) > (filesizebak / 100) * percent) {
			if (percent < 99)
				progressbar[percent++] = '=';
			else
				percent++;
		}

		printf("%hu %% %s %s / %s total %s/s            \r", percent, progressbar,
									  humanreadable1,
									  humanreadable2,
									  humanreadable3);
		
	}

	close (fd);

	return 1;
}

void
calc_elapsedtime(char *elapsedtime, time_t now1, time_t now2)
{
	unsigned short elapsed_time;

	memset(elapsedtime, 0, 10);
	elapsed_time = now2 - now1;
	
	if (elapsed_time < 2) {
		sprintf(elapsedtime, "1 sec", elapsed_time);
	} else if (elapsed_time < 60) {
		sprintf(elapsedtime, "%hu sec", elapsed_time);
	} else if (elapsed_time < 3600) {
		sprintf(elapsedtime, "%hu min", elapsed_time / 60);
	} else {	
		sprintf(elapsedtime, "%hu hr", elapsed_time / 3600);
	}

	return;
}

void
calc_averageTX(char *averageTX, unsigned long long filesize, time_t now1, time_t now2)
{
	unsigned int elapsedtime;
	float average;

	memset(averageTX, 0, 20);

	elapsedtime = now2 - now1;
	if (elapsedtime == 0)
		elapsedtime = 1;

	average = filesize / elapsedtime;

	if (average < 1024) {
		average /= 1024;
		sprintf(averageTX, "%.1f Ko/s", average);
	} else {
		average /= 1024;
		average /= 1024;
		sprintf(averageTX, "%.1f Mo/s", average);
	}
	
	return;
}

void
req_post_create_hdr_nv(struct hdr_nv_resp hdrnv[32], char *type)
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


int
req_post_upload_success(int susr, char *filename, char *filesize,
				  char *elapsedtime, char *averageTX, bool theme_dark)
{
	struct hdr_nv_resp hdrnv[32];
	struct hdr_nv_resp *phdrnv;
	int fd;	
	struct stat statbuf;
	char *buffer_tosend;
	char *buffer_tosend_filled;
	unsigned short buftosendfilled_size;
	char *tosend;
	char minhr[10];

	if (send(susr, "HTTP/1.1 200 Ok\r\n", 17, 0) < 0) {
		perror("send");
		return -1;
	}

	memset(hdrnv, 0, sizeof(struct hdr_nv_resp) * 32);
	req_post_create_hdr_nv(hdrnv, "text/html");

	if (theme_dark == true) { // success
		tosend = (char*) wu_res[14].local_resource_dark;
	} else {
		tosend = (char*) wu_res[14].local_resource_light;
	}

	if (stat(tosend, &statbuf) < 0) {
		perror("stat");
		return -1;
	}

	buffer_tosend = malloc(statbuf.st_size);
	memset(buffer_tosend, 0, statbuf.st_size);

	fd = open(tosend, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}
	if (read(fd, buffer_tosend, statbuf.st_size) < 0) {
		perror("read");
		return -1;
	}
	close(fd);

	wu_display_minhr(minhr);
	buftosendfilled_size = statbuf.st_size + strlen(filename) + strlen(elapsedtime)
					       + strlen(averageTX) + strlen(filesize) +
						 strlen(minhr) + 4;

	buffer_tosend_filled = malloc(buftosendfilled_size);
	memset(buffer_tosend_filled, 0, buftosendfilled_size);

	sprintf(buffer_tosend_filled, buffer_tosend, filename,
						     filesize,
						     elapsedtime,
						     averageTX,
						     minhr);	
	free(buffer_tosend);

	for (phdrnv = hdrnv; *phdrnv->name != '\0'; phdrnv++) {
		if (strcmp(phdrnv->name, "Content-Length") == 0) {
			sprintf(phdrnv->value, "%u", strlen(buffer_tosend_filled));
			break;
		}


	}

	req_post_send_hdr(susr, hdrnv);
	
	// printf("%s", buffer_tosend_filled);
	
	if (send(susr, buffer_tosend_filled, strlen(buffer_tosend_filled), 0) < 0) {
		perror("send");
		return -1;
	}

	free(buffer_tosend_filled);



	return 1;
}


void req_post_upload(int susr, struct hdr_nv *hdrnv)
{
	struct hdr_nv *phdrnv;
	char bufMIME[2048];
	unsigned short boundarylen;
	unsigned short MIMElen;
	unsigned long long clen;
	unsigned long long int filesize;
	char filename[254];
	char c;
	char humanreadable[10];
	time_t now1, now2;
	char elapsedtime[10];
	char averageTX[20];
	bool theme_dark = true;

	for (phdrnv = hdrnv; *phdrnv->name != '\0'; phdrnv++) {
		if (strcmp(phdrnv->name, "Content-Type") == 0) {
			boundarylen = checkget_ctype_boundary_len(phdrnv->value);
			if (!boundarylen)
				return;
			break;
		}


	}

	for (phdrnv = hdrnv; *phdrnv->name != '\0'; phdrnv++) {
		if (strcmp(phdrnv->name, "Cookie") == 0) {
			if (strcmp(phdrnv->value, "theme=dark") == 0) {
				break;
			} else if (strcmp(phdrnv->value, "theme=light") == 0) {
				theme_dark = false;
			}
			break;
		}


	}
	for (phdrnv = hdrnv; *phdrnv->name != '\0'; phdrnv++) {
		if (strcmp(phdrnv->name, "Content-Length") == 0) {
			clen = strtoll(phdrnv->value, NULL, 0);
			if (!clen)
				return;
			break;
		}


	}
	
	MIMElen = recv_MIME(susr, bufMIME);
	if (MIMElen < 0)
		return;
	if (parse_MIME(bufMIME, filename) < 0)
		return;
	filesize = clen - MIMElen - boundarylen - 13;

	tohumanreadable(filesize, humanreadable);

	printf("Start download of: %s(%s)\n", filename, humanreadable);
	
	now1 = time(0);
	if (receive_file(susr, filename, filesize, boundarylen) < 0) {
		fprintf(fp_wu, "Failed to upload %s (%s).\n", filename, humanreadable);
		return;
	}
	
	boundarylen += 8;
	while(boundarylen--)
		if (recv(susr, &c, 1, 0) != 1)
			return;

	now2 = time(0);
	calc_elapsedtime(elapsedtime, now1, now2);

	calc_averageTX(averageTX, filesize, now1, now2);

	if (req_post_upload_success(susr, filename, humanreadable, elapsedtime, averageTX, theme_dark) < 0) 
		fprintf(fp_wu, "Failed to upload %s (%s).\n", filename, humanreadable);
	else
		fprintf(fp_wu, "Successfully uploaded %s (%s).\n", filename, humanreadable);


	printf("\n");

	return;
}
