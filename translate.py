import os

replacements = {
    '"1. Choi Game (Offline)"': '"1. Chơi Game (Offline)"',
    '"2. Tao phong (May Chu)"': '"2. Tạo phòng (Máy Chủ)"',
    '"3. Vao phong (Nguoi choi)"': '"3. Vào phòng (Người chơi)"',
    '"4. Quan tri (Admin)"': '"4. Quản trị (Admin)"',
    '"Q. Thoat Game"': '"Q. Thoát Game"',
    '"SPACE SHOOTER"': '"SPACE SHOOTER"',
    '"- Quiz Adventure -"': '"- Quiz Adventure -"',
    '"1. Them cau hoi moi"': '"1. Thêm câu hỏi mới"',
    '"2. Xem danh sach cau hoi"': '"2. Xem danh sách câu hỏi"',
    '"3. Doi mat khau Admin"': '"3. Đổi mật khẩu Admin"',
    '"4. Dat lai bo cau hoi"': '"4. Đặt lại bộ câu hỏi"',
    '"5. Import de thi (CSV)"': '"5. Import đề thi (CSV)"',
    '"Q. Quay lai Menu chinh"': '"Q. Quay lại Menu chính"',
    '"Nhap du lieu:"': '"Nhập dữ liệu:"'
}

for filename in ["ui.c", "story.c", "admin.c", "player.c", "network.c", "graphics.c"]:
    if os.path.exists(filename):
        with open(filename, "r", encoding="utf-8") as f:
            content = f.read()
            
        for k, v in replacements.items():
            content = content.replace(k, v)
            
        with open(filename, "w", encoding="utf-8") as f:
            f.write(content)

print("Done translations!")
