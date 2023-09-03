void req_get_set_content_length(char *tosend, char *value);
void req_get_create_hdr_nv(struct hdr_nv_resp hdrnv[32], char *type);
char* req_get_create_response(char *res, struct hdr_nv_resp hdrnv[32], short dark, char type[10]);
void set_content_length(char *tosend, char *value);
