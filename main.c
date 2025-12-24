#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

void handle_request(int fd);

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) handle_error("socket");

    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        handle_error("setsockopt");

    const char *HOST = "127.0.0.1";
    const int PORT = 8000;

    struct in_addr ip_addr;
    if (inet_aton(HOST, &ip_addr) == 0) {
        fprintf(stderr, "ERROR: creating addr\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr = ip_addr;

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)))
        handle_error("bind");
    printf("[INFO] Binded to %s:%d\n", HOST, PORT);

    if (listen(sfd, 5))
        handle_error("listen");
    printf("[INFO] Listening on %s:%d\n", HOST, PORT);

    while (1) {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_size = sizeof(peer_addr);
        puts("[INFO] Waiting for new connection.");
        int cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
        if (cfd == -1) handle_error("accept");
        printf("[INFO] Connection accpected\n");

        handle_request(cfd);
        if (close(cfd) == -1) perror("close");
    }

    if (close(sfd) == -1) perror("close");
}

#define BUF_CAPACITY 1024

void handle_request(int fd) {
    static char buf[BUF_CAPACITY] = {0};
    int read_bytes = read(fd, &buf, BUF_CAPACITY);
    if (read_bytes == -1) {
        perror("read");
        return;
    }
    printf("[DEBUG] Read: %d bytes\n", read_bytes);

    char *msg = "HTTP/1.1 200 OK\r\n"
                "Content-Length: 22\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<h1>Hello, World!</h1>\r\n";
    int written_bytes = write(fd, msg, strlen(msg));
    if (written_bytes == -1) {
        perror("write");
        return;
    }
    printf("[DEBUG] Wrote: %d bytes\n", written_bytes);
}
