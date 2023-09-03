int handle_req(int susr, struct request_line *rline,
				struct hdr_nv hdrnv[32]);
int req_get(int susr, char *res, struct hdr_nv hdrnv[32]);
int req_post(int susr, char *res, struct hdr_nv hdrnv[32]);

