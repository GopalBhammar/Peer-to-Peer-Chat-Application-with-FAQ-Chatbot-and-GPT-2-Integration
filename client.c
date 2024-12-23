#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int create_socket(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[-]Socket error");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]Connection error");
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");
    return sock;
}

void handle_server_message(int sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Server disconnected.\n");
        close(sock);
        exit(EXIT_SUCCESS);
    } else {
        printf("Server:\n%s\n", buffer);
    }
}

void send_user_command(int sock) {
    char command[BUFFER_SIZE];
    printf("user > ");
    fgets(command, BUFFER_SIZE, stdin);
    command[strcspn(command, "\n")] = '\0'; // Remove newline character
    send(sock, command, strlen(command), 0);
    if (strcmp(command, "/logout") == 0) {
        printf("Bye!! Have a nice day\n");
        close(sock);
        exit(EXIT_SUCCESS);
    }
}

int main() {
    char *ip = "127.0.0.1";
    int port = 5566;
    int sock = create_socket(ip, port);
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        if (select(sock + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("[-]Select error");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(sock, &read_fds)) {
            handle_server_message(sock);
        }
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            send_user_command(sock);
        }
    }
    return 0;
}
