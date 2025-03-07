#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *message = "Merhaba, istemci!\n";
    int opt = 1;

    // Soket oluştur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Soket yeniden kullanılabilir hale getir
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Adres bilgilerini ayarla
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Soketi bağla
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Dinlemeye başla
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Sunucu %d numaralı portta çalışıyor...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        printf("Yeni bağlantı alındı!\n");
        
        // İstemciden veri al
        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("İstemciden gelen: %s\n", buffer);
        }
        
        send(new_socket, message, strlen(message), 0);
        //close(new_socket);
    }

    return 0;
}
