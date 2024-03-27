void req_get_send_hdr(int susr, struct hdr_nv_resp hdrnv[32]);
int req_get_send_file(int susr, char *tosend);
int req_get_send_resp_img(int susr, struct hdr_nv_resp hdrnvresp[32], char *tosend);
int req_get_send_resp_page(int susr, struct hdr_nv_resp hdrnvresp[32], char *tosend);
