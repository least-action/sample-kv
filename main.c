#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define PORT 1234
#define BUFFER_SIZE 64

int main()
{
    printf("program started.\n");

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
        perror("epoll_create1 error");
        exit(EXIT_FAILURE);
    }
  
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("failed to create server socket");
        close(epfd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("failed to bind");
        close(server_fd);
        close(epfd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("failed to listen");
        close(server_fd);
        close(epfd);
        exit(EXIT_FAILURE);
    }

    printf("server is running on port %d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("failed to accept");
            continue;
        }
        printf("client connected from: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                printf("client disconnected\n");
                break;
            }

            if (write(client_fd, buffer, bytes_read) != bytes_read) {
                perror("failed to write");
                break;
            }
        }

        close(client_fd);
    }

    close(server_fd);
    close(epfd);

    printf("server sucessfully terminated.\n");

    return 0;
}

