#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "wu_http.h"
#include "wu_main.h"
#include "wu_socket.h"
#include "wu_log.h"
#include "wu_handler.h"
#include "wu_content.h"

#define WU_NOT_INSTALLED "./pages/dark/index"

extern char log_path[254];
extern char log_wu_http[512];
extern char log_wu[512];
extern FILE *fp_wu;
extern FILE *fp_wu_http;

extern const struct wu_resource wu_res[19];
unsigned char wu_res_path[64];

void
usage(char *progname)
{
	printf("%s: -a address -p port [-l LOG_PATH]\n", progname);

	exit(0);
}

int
convert_input_addr(char *address, char *port, struct sockaddr_in *sin)
{
	struct in_addr inp;

	if (inet_aton(address, &inp) < 0) {
		printf("Bad user input\n");
		return -1;
	}

	memset(sin, 0, sizeof(struct sockaddr_in));
	memcpy(&sin->sin_addr, &inp, sizeof(struct in_addr));
	sin->sin_port = htons(atoi(port));
	sin->sin_family = AF_INET;

	return 1;
}

void*
sgn_usr_int(int sgn)
{
	printf("Closing log... quitting app\n");
	fclose(fp_wu_http);
	fclose(fp_wu);
	system("tput cnorm");

	exit(0);

	return NULL;
}

void
wu_is_installed(void)
{
	int fd;

	memset(wu_res_path, 0, 64);

	fd = open(WU_NOT_INSTALLED, O_RDONLY);
	if (fd > 0) {
		strcpy(wu_res_path, "./");
		close(fd);
	} else {
		strcpy(wu_res_path, "/usr/share/wu/");
	}

	return;
}

int
match_resource(char *res)
{
	int cnt;

	if (res && *res == '/' && *(res + 1) == '\0')
		return 2;
	else if (res && strcmp(res, "/quit") == 0)
		return 3;

	for (cnt = 0; cnt < 19; cnt++) {
		if (strcmp(wu_res[cnt].resource, res) == 0)
			return 1;
	}

	return -1;
}

int
check_hostfield(struct hdr_nv hdrnv[32], char *inhf)
{
	int cnt;

	for (cnt = 0; hdrnv[cnt].name[0] != '\0'; cnt++) {
		if (strcmp(hdrnv[cnt].name, "Host") == 0) {
			if (strcmp(hdrnv[cnt].value, inhf) == 0) {
				return 1;
			}
		}
	}

	return -1;
}

void
log_entry(struct request_line *rline, unsigned int nbytesent, bool hostfield)
{
	char buf[32];

	strcat(log_wu_http, "\"");
	strcat(log_wu_http, rline->method);
	strcat(log_wu_http, " ");
	strcat(log_wu_http, rline->resource);
	strcat(log_wu_http, " ");
	strcat(log_wu_http, rline->version);
	strcat(log_wu_http, "\"");

	if  (!hostfield) {
		memset(buf, 0, 32);
		sprintf(buf, " 400 0");

		strcat(log_wu_http, buf);

	} else if (strcmp(rline->method, "GET") == 0) {
		memset(buf, 0, 32);
		sprintf(buf, " 200 %u", nbytesent);

		strcat(log_wu_http, buf);
	} else if (strcmp(rline->method, "POST") == 0) {
		memset(buf, 0, 32);
		strcpy(buf, " 301");

		strcat(log_wu_http, buf);
	}

	fprintf(fp_wu_http, log_wu_http); 
	fprintf(fp_wu_http, "\n");

	memset(log_wu_http, 0, 512);
			
	return;
}

void
download_dir_exist(void)
{
	struct stat statbuf;

	if (stat("./Downloads", &statbuf) == 0) {
		return;
	} else {
		printf("Downloads directory does'nt exist, creating it.\n");
		if (mkdir("./Downloads", 0755) < 0) {
			printf("Cannot create Downloads directory, exiting.\n");
			exit(1);
		}
	}


	return;
}

int main(int argc, char **argv)
{
	struct sockaddr_in sin;
	char opt;
	int sserv, susr, ret;
	struct request_line rline;
	struct hdr_nv hdrnv[32];
	struct wu_resource wures_local[31];
	unsigned int nbytesent;
	char input_hostfield[32];
	bool hostfield = false;

	memset(log_path, 0, 254);

	if (argc < 5)
		usage(argv[0]);
	while((opt = getopt(argc, argv, "a:p:l:")) != -1) {
		switch(opt) {
			case 'a':
				break;
			case 'p':
				break;
			case 'l':
				strncpy(log_path, optarg, 254);	
				if (fopen_log() < 0)
					return 0;
				break;

			default:
				usage(argv[0]);
				break;
		}
	} 
	
	wu_is_installed();

	if (convert_input_addr(argv[2], argv[4], &sin) < 0)
		usage(argv[0]);

	memset(log_wu_http, 0, 512);

	memset(input_hostfield, 0, 32);
	strcpy(input_hostfield, argv[2]);
	if (strcmp(argv[4], "80") != 0) {
		strcat(input_hostfield, ":");
		strcat(input_hostfield, argv[4]);
	}

	if ((sserv = bind_input_addr(&sin)) < 0)
		return 0;


	signal(SIGINT, (void*)sgn_usr_int);
	signal(SIGPIPE, (void*)sgn_usr_int);

	system("tput civis");


	printf("Starting wifiupload on %s:%s...\n", argv[2], argv[4]);

	download_dir_exist();

	for (;;) {
		susr = accept_conn(sserv);

		if (parse_rline(susr, &rline) < 0)
			continue;

		ret = match_resource(rline.resource);
		switch(ret) {
			case 3:
				printf("Closing log... quitting app\n");
				fclose(fp_wu_http);
				fclose(fp_wu);
				system("tput cnorm");
				exit(0);
			case 2:
				strcpy(rline.resource, "/index");
				break;
			case 1:
				break;
			case -1:
			default:
				continue;
		}

		if (parse_hdr_nv(susr, (struct hdr_nv*)&hdrnv) < 0)
			continue;

		ret = check_hostfield(hdrnv, input_hostfield);
		if (ret > 0) {
			hostfield = true;

			ret = handle_req(susr, &rline, (struct hdr_nv*)&hdrnv);

		}

		log_entry(&rline, ret, hostfield);
		close(susr);
	}





	return 1;
}
