#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "wu_http.h"
#include "wu_send.h"
#include "wu_response.h"
#include "wu_theme.h"
#include "wu_upload.h"

unsigned char *static_res0 = "index";
unsigned char *static_res1 = "credits";
unsigned char *static_res2 = "settings";
unsigned char *dynami_res0 = "upload";
unsigned char *dynami_res1 = "change-theme";

int
req_get(int susr, char *res, struct hdr_nv hdrnv[32])
{
	unsigned short cnt;
	short dark;
	struct hdr_nv_resp resphdrnv[32];
	char *tosend;
	char type[10];

	for (cnt = 0; hdrnv[cnt].name[0] != '\0'; cnt++) {
		if (strcmp(hdrnv[cnt].name, "Cookie") == 0) {
			if (strcmp(hdrnv[cnt].value, "theme=dark") == 0) {
				dark = 1;
			} else {
				dark = -1;
			}
			break;
		}
	}
	
	tosend = req_get_create_response(res, resphdrnv, dark, type);

	if (strcmp(type, "image/png") == 0) {
		return req_get_send_resp_img(susr, resphdrnv, tosend);
	} else {
		return req_get_send_resp_page(susr, resphdrnv, tosend);
	}

	return 1;
}

int
req_post_theme(int susr, struct hdr_nv hdrnv[32])
{
	int cnt;
	int ret;
	int post_clength;
	char buf[sizeof("theme=light")];

	for (cnt = 0; hdrnv[cnt].name[0] != '\0'; cnt++) {
		if (strcmp(hdrnv[cnt].name, "Content-Length") == 0) {
			post_clength = atoi(hdrnv[cnt].value);	
			if (post_clength == 0) {
				return -1;
			} else if (post_clength == sizeof("theme=dark") -1 ||
				   post_clength == sizeof("theme=light") -1) {
				memset(buf, 0, sizeof("theme=light"));
				ret = recv(susr, buf, sizeof("theme=light") -1, 0);
				if (ret == sizeof("theme=dark") - 1 ||
				    ret == sizeof("theme=light") - 1) {
					return apply_theme(susr, buf);
				}

			}
		}

	}

	return -1;
}
int
handle_req(int susr, struct request_line *rline,
			 struct hdr_nv hdrnv[32])
{

	if (strcmp(rline->method, "GET") == 0) {
		return req_get(susr, rline->resource, hdrnv);
	} else if (strcmp(rline->method, "POST") == 0) {
		if (strcmp(rline->resource, "/theme") == 0) {
			req_post_theme(susr, hdrnv);
		} else if (strcmp(rline->resource, "/upload") == 0) {
			req_post_upload(susr, hdrnv);
		}
	} else {
		return -1;
	}

	return 1;
}

