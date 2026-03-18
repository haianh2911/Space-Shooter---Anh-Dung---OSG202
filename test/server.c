#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888 // Cổng mạng mà máy chủ sẽ mở để chờ người chơi

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Tin nhắn máy chủ sẽ ném cho người chơi ngay khi kết nối thành công
    char *welcome_msg = "CHAO MUNG DEN VOI MAY CHU SPACE SHOOTER!";

    // 1. Tạo Socket (Giống như mua một chiếc điện thoại)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Loi tao socket");
        exit(EXIT_FAILURE);
    }

    // Cài đặt để cổng 8888 có thể dùng lại ngay nếu lỡ tắt đột ngột
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Loi setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Nhận kết nối từ mọi IP
    address.sin_port = htons(PORT);

    // 2. Bind (Giống như cắm sim số 8888 vào điện thoại)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Loi bind (Cong 8888 co the dang bi phan mem khac chiem)");
        exit(EXIT_FAILURE);
    }

    // 3. Listen (Bật chuông, chờ cuộc gọi đến)
    if (listen(server_fd, 3) < 0) {
        perror("Loi listen");
        exit(EXIT_FAILURE);
    }
    
    // Dọn dẹp Terminal cho đẹp
    system("clear");
    printf("=========================================\n");
    printf("         MAY CHU ADMIN DA KHOI DONG      \n");
    printf("=========================================\n");
    printf("[*] Dang cho nguoi choi ket noi vao cong %d...\n", PORT);

    // 4. Accept (Có người gọi đến thì "Nhấc máy")
    // Lệnh này sẽ treo Terminal ở đây cho đến khi có người kết nối
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Loi accept");
        exit(EXIT_FAILURE);
    }

    printf("\n[!] BINGO! Da co mot Nguoi choi ket noi thanh cong!\n");

    // 5. Giao tiếp thử nghiệm (Gửi và Nhận)
    send(new_socket, welcome_msg, strlen(welcome_msg), 0);
    printf("[->] Da gui tin nhan chao mung den Nguoi choi.\n");
    
    read(new_socket, buffer, 1024);
    printf("[<-] Nguoi choi phan hoi: %s\n", buffer);

    // 6. Đóng kết nối
    close(new_socket);
    close(server_fd);
    printf("\n[*] May chu da dong ket noi.\n");

    return 0;
}