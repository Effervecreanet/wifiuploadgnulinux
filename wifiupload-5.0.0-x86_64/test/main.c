#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void
usage(char *progname)
{
	printf("%s: -s SIZE\n", progname);
	exit(1);

	return;
}
int main(int argc, char **argv)
{
	int fd;
	unsigned long long filesize;
	char buffer[2048];

	if (argc != 3)
		usage(argv[0]);

	memset(buffer, 'C', 2048);
	filesize = strtoll(argv[2], NULL, 0);

	fd = open(argv[2], O_CREAT | O_WRONLY);
	while (filesize > 2048) {
		write(fd, buffer, 2048);
		filesize -= 2048;
	}
	
	write(fd, buffer, filesize);
		

	close(fd);


	return 0;
}
	


	
