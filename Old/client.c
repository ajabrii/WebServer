#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // localhost

    // 3. Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // 4. Send HTTP GET request
    char *http_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    send(sock, http_request, strlen(http_request), 0);
    printf("Sent HTTP request:\n%s", http_request);

    // 5. Read response
    int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("\nResponse from server:\n%s\n", buffer);
    } else {
        printf("No response from server.\n");
    }

    close(sock);
    return 0;
}
