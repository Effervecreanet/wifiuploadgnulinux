
struct request_line {
	char method[5];
	char resource[128];
	char version[sizeof("HTTP/1.1")];
};

struct hdr_nv {
	char name[32];
	char value[254];
};

struct hdr_nv_resp{
	char *name;
	char value[254];
};

int parse_rline(int susr, struct request_line *rline);
int parse_hdr_nv(int susr, struct hdr_nv hdrnv[32]);

#define HTTP_HEADER_CACHE_CONTROL       "Cache-Control"
#define HTTP_HEADER_CONTENT_LANGUAGE    "Content-Language"
#define HTTP_HEADER_DATE		"Date"
#define HTTP_HEADER_CONTENT_TYPE	"Content-Type"
#define HTTP_HEADER_CONTENT_LENGTH	"Content-Length"
#define HTTP_HEADER_CONNECTION		"Connection"
#define HTTP_HEADER_SERVER		"Server"
#define HTTP_HEADER_SET_COOKIE		"Set-Cookie"
#define HTTP_HEADER_LOCATION		"Location"

#define HTTP_HEADER_CACHE_CONTROL_VALUE		"public, max-age=0"
#define HTTP_HEADER_CONTENT_LANGUAGE_VALUE	"en-US"
#define HTTP_HEADER_SERVER_VALUE		"Wu/5.0"
#define HTTP_HEADER_CONNECTION_VALUE		"Close"

#define HTTP_STRING_DATE_SIZE sizeof("Mon, 01 Jan 1970 00:00:00 GMT")
