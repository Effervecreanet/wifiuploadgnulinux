#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wu_date.h"
#include "wu_log.h"


char log_path[254];
char log_date_now[LOG_DATE_NOW_SIZE];
char log_wu_http[512];
char log_wu[512];

FILE *fp_wu_http;
FILE *fp_wu;


int
fopen_log(void)
{
	char *path;

	if (log_path[0] == '\0')
		strcpy(log_path, STD_LOG_PATH);

	path = strdup(log_path);
	strcat(path, LOG_WU_HTTP);
	
	fp_wu_http = fopen(path, "a+");
	free(path);

	if (fp_wu_http == NULL) {
		perror("fopen");
		return -1;
	}

	localtime_now();
	fprintf(fp_wu_http, "Wifiupload http log successfully opened at %s\n", log_date_now);

	path = strdup(log_path);
	strcat(path, LOG_WU);
	
	fp_wu = fopen(path, "a+");
	free(path);

	if (fp_wu == NULL) {
		perror("fopen");
		return -1;
	}

	localtime_now();
	fprintf(fp_wu, "Wifiupload log successfully opened at %s\n", log_date_now);

	return 1;
}


	
