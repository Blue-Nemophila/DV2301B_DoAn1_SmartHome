# DV2301B_DoAn1_SmartHome

# ĐỒ ÁN SMART HOME IoT (ESP32)

## 1. Mục tiêu dự án
Xây dựng hệ thống Nhà Thông Minh sử dụng ESP32 điều khiển thiết bị qua Web Server.

## 2. Chức năng chính
- Kết nối Wi-Fi
- Điều khiển relay / đèn / servo
- Hiển thị dữ liệu cảm biến theo thời gian thực
- Giao diện Web tương tác

## 3. Cách chạy
1. Cài thư viện cần thiết (ESP32, WiFi, WebServer…)
2. Cấu hình SSID + Password trong `main.ino`
3. Nạp code vào ESP32
4. Mở Serial để lấy IP → truy cập trình duyệt

## 4. Cấu trúc project
- `/src/` – code chính
- `/web/` – HTML, JS, CSS giao diện
- `/assets/` – ảnh, icon nếu có

## 5. Tác giả
- Tên: Nguyễn Lê Thái Tài
- Lớp: DV2301B_UTH
