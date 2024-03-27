all: wu

wu: src/wu_date.c src/wu_handler.c src/wu_http.c src/wu_log.c src/wu_main.c src/wu_send.c src/wu_socket.c src/wu_content.c src/wu_response.c src/wu_theme.c src/wu_upload.c
	gcc -o wu src/wu_date.c src/wu_handler.c src/wu_http.c src/wu_log.c src/wu_main.c src/wu_send.c src/wu_socket.c  src/wu_content.c src/wu_response.c src/wu_theme.c src/wu_upload.c

clean:
	rm -f wu *.o
