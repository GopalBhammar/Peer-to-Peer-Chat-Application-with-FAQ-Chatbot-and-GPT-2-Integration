#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <uuid/uuid.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define FAQS_FILE "FAQs.txt"

typedef struct {
    int socket;
    uuid_t uuid;
    int chatbot_status;
    char history_file[100];
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int num_clients = 0;

int server_socket, max_fd;
struct sockaddr_in server_addr;
fd_set active_fds;

int chatbot_v2_enable[MAX_CLIENTS] = {0}; 
uuid_t active_uuids[MAX_CLIENTS];

// char sender_uuid_str1[37];

char *find_faq_answer(const char *question) {
    static char answer[BUFFER_SIZE];
    FILE *faq_file = fopen(FAQS_FILE, "r");
    if (faq_file == NULL) {
        perror("Error opening FAQs file");
        return "Error: FAQs file not found.";
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), faq_file)) {
        char *delimiter = "|||";
        char *query = strtok(line, delimiter);
        char *response = strtok(NULL, delimiter);
        if (query && response && strstr(question, query)) {
            strcpy(answer, response);
            fclose(faq_file);
            return answer;
        }
    }

    fclose(faq_file);
    return "Sorry, I couldn't find an answer to your question.";
}

void save_chat_history(const char *sender_uuid, const char *recipient_uuid, const char *message) {
    char filename[100];
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    FILE *history_file = fopen(filename, "a");
    if (history_file == NULL) {
        perror("Error opening history file");
        return;
    }
    fprintf(history_file, "%s\n", message);
    fclose(history_file);
}

void retrieve_chat_history(const char *sender_uuid, const char *recipient_uuid, int socket) {
    char filename[100];
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    FILE *history_file = fopen(filename, "r");
    if (history_file == NULL) {
        char message[] = "No chat history found.";
        send(socket, message, strlen(message), 0);
        return;
    }

    char message[] = "Chat history:\n";
    send(socket, message, strlen(message), 0);

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), history_file)) {
        send(socket, line, strlen(line), 0);
    }

    fclose(history_file);
}

void delete_chat_history(const char *sender_uuid, const char *recipient_uuid) {
    char filename[100];
    snprintf(filename, sizeof(filename), "chat_history_%s_%s.txt", sender_uuid, recipient_uuid);
    if (remove(filename) != 0) {
        perror("Error deleting file");
    } else {
        printf("Chat history deleted successfully.\n");
    }
}

void delete_all_chat_history(const char *uuid) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0) {
            char recipient_uuid[37];
            uuid_unparse(clients[i].uuid, recipient_uuid);
            delete_chat_history(uuid, recipient_uuid);
        }
    }
}

void handle_client_command(int client_index, char *command) {
    if (strncmp(command, "/active", 7) == 0) {
        char active_clients[BUFFER_SIZE] = "Active clients: ";
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0) {
                char uuid_str[37];
                uuid_unparse(clients[i].uuid, uuid_str);
                strcat(active_clients, uuid_str);
                strcat(active_clients, ", ");
            }
        }
        send(clients[client_index].socket, active_clients, strlen(active_clients), 0);
    } else if (strncmp(command, "/send", 5) == 0) {
        char recipient_uuid[37];
        char message[BUFFER_SIZE];
        sscanf(command, "/send %s %[^\n]", recipient_uuid, message);

        int recipient_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0) {
                char uuid_str[37];
                uuid_unparse(clients[i].uuid, uuid_str);
                if (strcmp(recipient_uuid, uuid_str) == 0) {
                    recipient_index = i;
                    break;
                }
            }
        }

        if (recipient_index != -1) {
            char sender_uuid[37];
            uuid_unparse(clients[client_index].uuid, sender_uuid);
            char full_message[BUFFER_SIZE + 37 + 2];
            snprintf(full_message, sizeof(full_message), "%s:%s", sender_uuid, message);
            save_chat_history(sender_uuid, recipient_uuid, full_message);
            save_chat_history(recipient_uuid, sender_uuid, full_message);
            send(clients[recipient_index].socket, full_message, strlen(full_message), 0);
        } else {
            char error_message[] = "Destination is either offline or not found.";
            send(clients[client_index].socket, error_message, strlen(error_message), 0);
        }
    } else if (strncmp(command, "/logout", 7) == 0) {
        char goodbye_message[] = "Bye!! Have a nice day";
        send(clients[client_index].socket, goodbye_message, strlen(goodbye_message), 0);
        close(clients[client_index].socket);
        FD_CLR(clients[client_index].socket, &active_fds);
        clients[client_index].socket = 0;
        num_clients--;
    } else if (strncmp(command, "/chatbot login", 14) == 0) {
        clients[client_index].chatbot_status = 1;
        char welcome_message[] = "stupidbot> Hi, I am stupid bot, I am able to answer a limited set of your questions\n";
        send(clients[client_index].socket, welcome_message, strlen(welcome_message), 0);
    } else if (strncmp(command, "/chatbot logout", 15) == 0) {
        clients[client_index].chatbot_status = 0;
        char goodbye_message[] = "stupidbot> Bye! Have a nice day and do not complain about me\n";
        send(clients[client_index].socket, goodbye_message, strlen(goodbye_message), 0);
    } else if (clients[client_index].chatbot_status == 1) {
        char question[BUFFER_SIZE];
        strcpy(question, command);
        strcat(question, " ");
        char *answer = find_faq_answer(question);
        char response[BUFFER_SIZE + 11];
        snprintf(response, sizeof(response), "stupidbot> %s", answer);
        send(clients[client_index].socket, response, strlen(response), 0);
    } else if (strncmp(command, "/delete_all", 11) == 0) {
        char uuid_str[37];
        uuid_unparse(clients[client_index].uuid, uuid_str);
        delete_all_chat_history(uuid_str);
        char success_message[] = "All chat history deleted.\n";
        send(clients[client_index].socket, success_message, strlen(success_message), 0);
    } else if (strncmp(command, "/history_delete", 15) == 0) {
        char recipient_uuid[37];
        sscanf(command, "/history_delete %s", recipient_uuid);
        char sender_uuid[37];
        uuid_unparse(clients[client_index].uuid, sender_uuid);
        delete_chat_history(sender_uuid, recipient_uuid);
        char success_message[] = "Chat history deleted.\n";
        send(clients[client_index].socket, success_message, strlen(success_message), 0);
    } else if (strncmp(command, "/history", 8) == 0) {
        char recipient_uuid[37];
        sscanf(command, "/history %s", recipient_uuid);
        char sender_uuid[37];
        uuid_unparse(clients[client_index].uuid, sender_uuid);
        retrieve_chat_history(sender_uuid, recipient_uuid, clients[client_index].socket);
    } else if (strncmp(command, "/chatbot_v2 login", 17) == 0) {
        chatbot_v2_enable[client_index] = 1;
        send(clients[client_index].socket, "gpt2bot> I am a Chatbot capable of answering questions for airline services but take it with a grain of salt it can be wrong.", strlen("gpt2bot> I am a Chatbot capable of answering questions for airline services but take it with a grain of salt it can be wrong."), 0);
        char sender_uuid_str[37];
        uuid_unparse(active_uuids[client_index], sender_uuid_str);
        // printf("%s",sender_uuid_str);
        char filename[100];
        strcpy(filename, "chatbot_details_");
        strcat(filename, sender_uuid_str);
        strcat(filename, ".txt");
        FILE *que = fopen(filename, "w");
        if (que == NULL) {
            printf("Error opening file!\n");
            return;
        }
        fprintf(que, "User >");
        fclose(que);
    } else if (strncmp(command, "/chatbot_v2 logout", 18) == 0) {
        chatbot_v2_enable[client_index] = 0;
        send(clients[client_index].socket, "gpt2bot> Thanks for considering me. Have a nice day.", strlen("gpt2bot> Thanks for considering me. Have a nice day."), 0);
        char sender_uuid_str[37];
        uuid_unparse(active_uuids[client_index], sender_uuid_str);
        char filename[100];
        strcpy(filename, "chatbot_details_");
        strcat(filename, sender_uuid_str);
        strcat(filename, ".txt");
        FILE *que = fopen(filename, "w");
        if (que == NULL) {
            printf("Error opening file!\n");
            return;
        }
        fprintf(que, " ");
        fclose(que);
    } else if (chatbot_v2_enable[client_index] == 1) {
        char sender_uuid_str[37];
        uuid_unparse(active_uuids[client_index], sender_uuid_str);
        char filename[100];
        strcpy(filename, "chatbot_details_");
        strcat(filename, sender_uuid_str);
        strcat(filename, ".txt");
        FILE *que = fopen(filename, "w");
        if (que == NULL) {
            printf("Error opening file!\n");
            return;
        }
        size_t bytes_written = fwrite(command, sizeof(char), strlen(command), que);
        if (bytes_written != strlen(command)) {
            printf("Error writing to file!\n");
            fclose(que);
            return;
        }
        fclose(que);
        char pythonexecc[60];
        memset(pythonexecc, 0, 60);
        strcat(pythonexecc, "python3 gpt2_gen.py ");
        strcat(pythonexecc, sender_uuid_str);
        // printf("\n%s\n", pythonexecc);
        system(pythonexecc);
        strcpy(pythonexecc, "");
        FILE *response_file = fopen(filename, "r");
        if (response_file == NULL) {
            printf("Error opening response file!\n");
            return;
        }
        char response_buffer[BUFFER_SIZE];
        size_t total_bytes_read = 0;
        size_t bytes_read;
        while ((bytes_read = fread(response_buffer + total_bytes_read, 1, BUFFER_SIZE - total_bytes_read, response_file)) > 0) {
            total_bytes_read += bytes_read;
            if (total_bytes_read >= BUFFER_SIZE - 1) {
                printf("Response file too large for buffer!\n");
                fclose(response_file);
                return;
            }
        }
        response_buffer[total_bytes_read] = '\0'; // Null-terminate the string
        fclose(response_file);
        send(clients[client_index].socket, response_buffer, total_bytes_read, 0);
        remove(filename);
    }
}

int main() {
    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5566);

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening for connections...\n");

    // Initialize file descriptor set
    FD_ZERO(&active_fds);
    FD_SET(server_socket, &active_fds);
    max_fd = server_socket;

    // Main server loop
    while (1) {
        fd_set read_fds = active_fds;

        // Wait for activity on any socket
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Check for new connections
        if (FD_ISSET(server_socket, &read_fds)) {
            // Accept new connection
            int client_socket = accept(server_socket, NULL, NULL);
            if (client_socket < 0) {
                perror("Accept failed");
                continue;
            }

            if (num_clients >= MAX_CLIENTS) {
                char error_message[] = "Current limit of server exceeded. Try again later.\n";
                send(client_socket, error_message, strlen(error_message), 0);
                close(client_socket);
                continue;
            }

            printf("New connection accepted. Sending UUID...\n");

            // Generate UUID for the client
            uuid_t client_uuid;
            uuid_generate(client_uuid);
            char uuid_str[37];
            uuid_unparse(client_uuid, uuid_str);

            // Add new client to the list
            int client_index = -1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = client_socket;
                    uuid_copy(clients[i].uuid, client_uuid);
                    clients[i].chatbot_status = 0;
                    snprintf(clients[i].history_file, sizeof(clients[i].history_file), "chat_history_%s.txt", uuid_str);
                    client_index = i;
                    break;
                }
            }

            // Send UUID to the client
            char welcome_message[64];
            snprintf(welcome_message, sizeof(welcome_message), "welcome, your UUID is %s", uuid_str);
            send(client_socket, welcome_message, strlen(welcome_message), 0);

            // Add new client socket to file descriptor set
            FD_SET(client_socket, &active_fds);
            if (client_socket > max_fd) {
                max_fd = client_socket;
            }

            num_clients++;
        }

        // Handle client messages
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_socket = clients[i].socket;
            if (FD_ISSET(client_socket, &read_fds)) {
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0) {
                    // Client disconnected
                    printf("Client %d disconnected.\n", client_socket);
                    close(client_socket);
                    FD_CLR(client_socket, &active_fds);
                    clients[i].socket = 0;
                    num_clients--;
                } else {
                    // Process client message
                    printf("Received message from client %d: %s\n", client_socket, buffer);
                    handle_client_command(i, buffer);
                }
            }
        }
    }

    // Close server socket
    close(server_socket);
    return 0;
}