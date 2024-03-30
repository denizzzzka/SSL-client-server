#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>	
#include <sys/types.h>
#include <netinet/in.h>
#include "openssl/ssl.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER 2048
#define PORT_SERVER 8888
#define IP "127.0.0.1"

// error handlers

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("socket failed");
        abort();
    }
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("bind failed");
        abort();
    }
}

void Listen(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res == -1) {
        perror("listen failed");
        abort();
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res == -1) {
        perror("accept failed");
        abort();
    }
    return res;
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res == -1) {
        perror("connect failed");
        abort();
    }
}

void Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        printf("inet_pton failed: src does not contain a character"
            " string representing a valid network address in the specified"
            " address family\n");
        abort();
    }
    if (res == -1) {
        perror("inet_pton failed");
        abort();
    }
}

void Close(int sockfd) {
    int res = close(sockfd);
    if (res == -1) {
        perror("close failed");
        abort();
    }
}

int start_connection_server(int *server) {
    *server = Socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_SERVER);
    Bind(*server, (struct sockaddr *) &adr, sizeof(adr));

    Listen(*server, 1);
    socklen_t adrlen = sizeof (adr);
    int client = Accept(*server, (struct sockaddr *) &adr, &adrlen);
    printf("Connected\n\n");
    return client;
}

SSL_CTX* init_context_server(char* certificate, char* key)	{
    SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms(); //подгружаем таблицу с алгоритмами шифровки
    method = SSLv23_server_method();	// используем данный метод
    ctx = SSL_CTX_new(method); // контекст от метода

    // инициализируем контекст по сертификатам

    SSL_CTX_use_certificate_file(ctx, certificate, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
    if (!SSL_CTX_check_private_key(ctx)) {
        perror("Certificate and key not matching\n");
        abort();
    }
    return ctx;
}
 
void MyCaps (char *str) {
    const int to_capital = 'A' - 'a';
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] >= 'a' && str[i] <= 'z'){
            str[i] = str[i] + to_capital;
        }
    }
    return;
}

void do_thing_server(SSL* ssl) {
    char buf[BUFFER];
    SSL_accept(ssl);
    int len = 0;
	while(1) {
        memset(buf, 0, BUFFER);
        if(!(len = SSL_read(ssl, buf, sizeof(buf)))) {
            break;
        }

        printf("client sent : %s", buf);
        MyCaps(buf);
        printf("sending to client: %s\n\n", buf);
        SSL_write(ssl, buf, len);
    } 

    int fd = SSL_get_fd(ssl);
    SSL_free(ssl);
    Close(fd);
}
 
SSL_CTX* init_context_client() {
    SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms(); //подгружаем таблицу с алгоритмами шифровки
    method = SSLv23_client_method();	// используем данный метод
    ctx = SSL_CTX_new(method); // контекст от метода

    return ctx;
}

int start_connection_client() {
    int client = Socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_SERVER);
    Inet_pton(AF_INET, IP, &adr.sin_addr);
    Connect(client, (struct sockaddr*)&adr, sizeof(adr));

    return client;
}

void SSL_CTX_keylog_func(const SSL *ssl, const char *line) {
    int outf = open("./Keys.txt", O_CREAT|O_RDWR, 0600);
    Close(outf);
    FILE *fd = fopen("./Keys.txt", "aw");
    fprintf(fd, "%s\n", line);
    fclose(fd);
}

void do_thing_client(SSL* ssl) {
    char buf[BUFFER];
    char input[BUFFER];

    SSL_connect(ssl);
    printf("Connected, you can shutdown client using -\n\n");
	while(1) {
	    fgets(input, BUFFER, stdin);
        if (input[0] == '-') {
            break;
        }
        
        printf("Sending to server : %s", input);
        SSL_write(ssl, input, strlen(input));
        SSL_read(ssl, buf, sizeof(buf));
        printf("Server answer : %s\n\nYour input : ", buf);
    }

    SSL_free(ssl);
    return;
}

void server_sll() {
    SSL_library_init(); // подгружаем алгоритмы шифровки
    SSL_CTX *ctx = init_context_server("mycert.pem", "mycert.pem"); // инициализируем контекст

    int server;
    int client = start_connection_server(&server); //соединяемся с клиентом

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client); //привязываем ssl к клиенту   
	        
    do_thing_server(ssl); //обрабатываем запросы
    
    Close(server);
    SSL_CTX_free(ctx);
}

void client_sll() {
    SSL_library_init();
    SSL_CTX *ctx = init_context_client();

    SSL_CTX_set_keylog_callback(ctx, SSL_CTX_keylog_func); //запоминаем ключи для дешифровки
    int server = start_connection_client();

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, server);
    do_thing_client(ssl);

    Close(server);
    SSL_CTX_free(ctx);
    return;
}

int main(int argc, char ** argv) { 
    int config;
    printf("In this program server will Caps messages from client\nWho are you\n1 - server\n2 - client\n\n");
    scanf("%d", &config);
    if (config == 1) {
        server_sll();
    } else if (config == 2) {
        client_sll();
    }

    return 0;
}