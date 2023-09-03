void usage(char *progname);
int convert_input_addr(char *address, char *port, struct sockaddr_in *sin);
int check_hostfield(struct hdr_nv hdrnv[32], char *inhf);
