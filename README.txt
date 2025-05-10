TCS34725 Raspberry Pi 5 Driver & User Space IOCTL Tester
THANH VIEN:
NGO ANH MINH_22146170
NGUYEN KIEN DUC TAI_22146212
TRUONG KY VY_22146255
=========================================================
✅ Ứng dụng thực tế của cảm biến TCS34725
1. Phân loại màu sắc
    Dự án robot line follower, phân biệt các màu vạch.
    Phân loại sản phẩm theo màu sắc trong các dây chuyền sản xuất mini.
2. Đo ánh sáng môi trường
    Kết hợp với LED trắng để xác định màu của vật thể được chiếu sáng.
    Có thể dùng để đo chất lượng ánh sáng hoặc điều chỉnh độ sáng đèn tự động theo môi trường.
3. Phát hiện vật thể và màu sắc trong tự động hóa
    Phân biệt các đối tượng trên băng tải (đỏ, xanh, vàng,...)
    Tự động kích hoạt các quy trình xử lý khác nhau theo màu sắc.
4. Ứng dụng nghệ thuật
    Dùng làm công cụ quét màu để chuyển vật thể thực thành màu số hóa RGB.
    Có thể dùng làm công cụ lấy màu cho họa sĩ kỹ thuật số.
5. Tự động hóa trong nhà thông minh
    Xác định màu ánh sáng môi trường để điều chỉnh đèn RGB nội thất.
    Kết hợp với camera để phân tích không gian, ánh sáng.
✅ Ưu điểm
    Tích hợp IR filter ⇒ màu nhận được chính xác hơn.
    Có amplifier và gain control, cho phép đo chính xác ngay cả khi ánh sáng yếu.
    Dễ dàng sử dụng qua I2C trên Raspberry Pi, Arduino, STM32,...
=========================================================
Author: You  
License: GPL  
Target: Raspberry Pi 5 (BCM2712)  
Sensor: TCS34725 (I2C Color Sensor)

Directory Structure:
--------------------
driver_tcs34725/
├── tcs34725_driver.c       → Kernel module with IOCTL support
├── tcs_user.c              → User-space test application
├── Makefile                → For building the kernel module
├── tcs34725-overlay.dts    → Device Tree Overlay for TCS34725
└── instruction.txt         → This file

-----------------------------------------------------------
STEP 1: Enable I2C on Raspberry Pi
-----------------------------------------------------------
Run:
  sudo raspi-config
Navigate to:
  Interface Options → I2C → Enable

Reboot your Pi after enabling I2C.

-----------------------------------------------------------
STEP 2: Build the Kernel Driver
-----------------------------------------------------------
From inside the folder:
  make

Load the module:
  sudo insmod tcs34725_driver.ko

Check logs:
  dmesg | grep TCS

-----------------------------------------------------------
STEP 3: Compile User-Space Application
-----------------------------------------------------------
  gcc -o tcs_user tcs_user.c

-----------------------------------------------------------
STEP 4: Use Device Tree Overlay (Optional but Recommended)
-----------------------------------------------------------
Compile the overlay:
  dtc -I dts -O dtb -o tcs34725.dtbo tcs34725-overlay.dts

Move it to the overlays directory:
  sudo cp tcs34725.dtbo /boot/overlays/

Edit /boot/firmware/config.txt (or /boot/config.txt):
  sudo nano /boot/config.txt

Add at the bottom:
  dtoverlay=tcs34725

Then reboot:
  sudo reboot

-----------------------------------------------------------
STEP 5: Run the User-Space Test
-----------------------------------------------------------
  sudo ./tcs_user

Expected Output:
  Raw Sensor Data + Normalized RGB + Color classification

-----------------------------------------------------------
To Clean:
-----------------------------------------------------------
  make clean

To Remove Module:
  sudo rmmod tcs34725_driver

-----------------------------------------------------------
Notes:
-----------------------------------------------------------
- Make sure the TCS34725 sensor is connected to I2C1 (default: SDA1/SCL1).
- Default I2C address for TCS34725 is 0x29.
- If multiple I2C devices are connected, ensure address is not in conflict.

Happy Hacking!
