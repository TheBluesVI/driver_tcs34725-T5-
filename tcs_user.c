#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define TCS_IOCTL_MAGIC 't'
#define TCS_IOCTL_READ_CLEAR  _IOR(TCS_IOCTL_MAGIC, 1, unsigned short)
#define TCS_IOCTL_READ_RED    _IOR(TCS_IOCTL_MAGIC, 2, unsigned short)
#define TCS_IOCTL_READ_GREEN  _IOR(TCS_IOCTL_MAGIC, 3, unsigned short)
#define TCS_IOCTL_READ_BLUE   _IOR(TCS_IOCTL_MAGIC, 4, unsigned short)

const char* classify_color(int r, int g, int b) {
    // Khoảng cách màu
    int rg = r - g;
    int rb = r - b;
    int gb = g - b;
    int gr = g - r;
    int br = b - r;
    int bg = b - g;

    if (r > g && r > b && rg > 40 && rb > 40)
        return "Red";
    else if (g > r && g > b && gr > 40 && gb > 40)
        return "Green";
    else if (b > r && b > g && br > 40 && bg > 40)
        return "Blue";
    else
        return "Unknown";
}

int main() {
    int fd;
    unsigned short clear = 0, red = 0, green = 0, blue = 0;

    fd = open("/dev/tcs34725", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/tcs34725");
        return 1;
    }

    ioctl(fd, TCS_IOCTL_READ_CLEAR, &clear);
    ioctl(fd, TCS_IOCTL_READ_RED, &red);
    ioctl(fd, TCS_IOCTL_READ_GREEN, &green);
    ioctl(fd, TCS_IOCTL_READ_BLUE, &blue);

    printf("\nRaw Sensor Data:\n");
    printf("  Clear: %u\n", clear);
    printf("  Red:   %u\n", red);
    printf("  Green: %u\n", green);
    printf("  Blue:  %u\n", blue);

    // Normalize
    float sum = red + green + blue;
    int r = 0, g = 0, b = 0;

    if (sum > 0) {
        r = (int)((float)red / sum * 255.0);
        g = (int)((float)green / sum * 255.0);
        b = (int)((float)blue / sum * 255.0);
    }

    printf("\nNormalized RGB: (%d, %d, %d)\n", r, g, b);

    const char* result = classify_color(r, g, b);
    printf("Detected Color: %s\n", result);

    close(fd);
    return 0;
}
