#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Toi la Nguoi choi! Toi da vao phong thanh cong!";
    char buffer[1024] = {0};

    // 1. Tạo socket (Tạo thiết bị kết nối)
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nLoi tao socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Chuyển đổi IP 127.0.0.1 (Localhost) để hệ thống hiểu
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nDia chi IP khong hop le\n");
        return -1;
    }

    // 3. Tiến hành kết nối đến Máy chủ (Gõ cửa phòng)
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nKet noi den May chu that bai! Hay kiem tra xem server da chay chua.\n");
        return -1;
    }

    // 4. Nhận tin nhắn từ Admin
    read(sock, buffer, 1024);
    printf("MAY CHU ADMIN: %s\n", buffer);

    // 5. Trả lời lại Admin
    send(sock, hello, strlen(hello), 0);
    printf("[->] Da bao cao danh tinh cho May chu.\n");

    // 6. Xong việc thì đóng kết nối
    close(sock);
    return 0;
} 