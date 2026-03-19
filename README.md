# Space Shooter - 

Một tựa game bắn súng không gian 2D với yếu tố giải đố, được lập trình bằng ngôn ngữ C và thư viện SDL2. Người chơi sẽ điều khiển phi thuyền thám hiểm, vừa né tránh hoặc tiêu diệt kẻ thù, vừa giải quyết các câu hỏi trắc nghiệm để thu thập tài nguyên cần thiết cho Trái Đất.

## 🚀 Cốt truyện

Năm 2250, Trái Đất cạn kiệt tài nguyên hoàn toàn. Hy vọng cuối cùng của nhân loại đặt lên vai bạn - điều khiển phi thuyền thám hiểm tiến vào vùng không gian sâu thẳm. Nguồn tài nguyên quý giá đang bị phong ấn bên trong lõi của các tiểu hành tinh cổ đại. 

Để khai thác, bạn phải giải mã các "khóa tri thức". Mỗi khi bạn trả lời đúng một câu hỏi, lớp vỏ tiểu hành tinh sẽ vỡ vụn, giải phóng năng lượng và khoáng sản. Tuy nhiên, hành trình này đầy rẫy hiểm nguy từ các tàu thế lực thù địch đang chực chờ cướp nguồn năng lượng của bạn. Trí tuệ và sự phản xạ của bạn chính là hy vọng duy nhất của nhân loại!

## 🎮 Cách chơi

- **Di chuyển:** Sử dụng các **phím Mũi tên** (Lên, Xuống, Trái, Phải) để điều khiển phi thuyền.
- **Tương tác:** Đâm vào tiểu hành tinh để kích hoạt câu hỏi (Khóa Tri Thức).
- **Trả lời câu hỏi:** Gõ phím `A`, `B`, `C`, `D` tương ứng với đáp án bạn chọn, hoặc dùng Mũi tên Lên/Xuống + `ENTER`.
- **Tạm dừng / Thoát:** Bấm `ESC` để tạm dừng, `Q` để thoát trò chơi.

### 💯 Ký hiệu và Điểm số

- **Tàu của bạn:** Điều khiển bằng phím mũi tên.
- **Tiểu hành tinh:** Chứa năng lượng và câu hỏi tri thức.
  - Trả lời **ĐÚNG**: +10 Điểm.
  - Trả lời **SAI** / **HẾT GIỜ**: -1 Máu.
- **Tàu địch:** Né tránh chúng! Nếu chạm phải, bạn sẽ mất 1 Mạng. Tốc độ và số lượng sẽ tăng dần theo thời gian.
- **Khiên bảo vệ:** Nhặt để có lớp bảo vệ trong 10 giây (vô hiệu hóa 1 lần va chạm với Tàu địch).

## 🛠 Yêu cầu hệ thống và Cài đặt

Dự án sử dụng ngôn ngữ **C** và thư viện **SDL2**, **SDL2_ttf**. Bạn cần cài đặt các thư viện này trước khi tiến hành biên dịch trò chơi.

### Trên macOS

1. Cài đặt Homebrew (nếu chưa có).
2. Cài đặt các thư viện SDL2:
   ```bash
   brew install sdl2 sdl2_ttf
   ```
3. Chạy script để biên dịch:
   ```bash
   ./build_mac.sh
   # hoặc dùng Makefile
   make
   ```
4. Chạy game:
   ```bash
   ./space_shooter
   ```

### Trên Windows

1. Cài đặt MSYS2 (MinGW-w64).
2. Cài đặt thư viện SDL2 qua pacman:
   ```bash
   pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
   ```
3. Sử dụng Makefile dành cho Windows:
   ```bash
   make -f Makefile.win
   ```

## 📂 Cấu trúc dự án

- `src/` - Chứa toàn bộ mã nguồn trò chơi.
  - `main.c`: Vòng lặp chính của chương trình.
  - `player.c`: Điều khiển phi thuyền, xử lý va chạm và hệ thống câu hỏi.
  - `story.c`, `ui.c`, `graphics.c`: Xử lý giao diện, hiệu ứng hiển thị, văn bản và cốt truyện.
  - `admin.c`: (Tính năng nhà phát triển) Quản lý bộ câu hỏi.
  - `network.c`: Xử lý bảng xếp hạng hoặc điểm chuẩn hóa (nếu có).

## 📝 Chú ý

- Tốc độ trò chơi sẽ tăng nhẹ mỗi 3 giây. Tốc độ sinh ra Tàu địch sẽ tăng lên khi chơi lâu.
- Game sẽ hiển thị màn hình *Xem Lại Câu Hỏi* (Review Screen) sau khi kết thúc, giúp người chơi ôn lại những câu nào đã làm sai/đúng.

---
*Dự án thực hiện bởi [haianh2911](https://github.com/haianh2911)*
