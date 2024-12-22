#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

// IOCTL commands
#define WR_VALUE _IOW('a', 'a', int32_t*)
#define RD_VALUE _IOR('a', 'b', int32_t*)

int main()
{
    int fd;
    int32_t value, number;

    // Open the device
    fd = open("/dev/ioctl_dev", O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file...\n");
        return -1;
    }

    // Write value
    printf("Enter the value to send: ");
    scanf("%d", &number);
    value = number;
    if (ioctl(fd, WR_VALUE, &value) < 0) {
        printf("IOCTL Write failed\n");
        return -1;
    }

    // Read value
    if (ioctl(fd, RD_VALUE, &value) < 0) {
        printf("IOCTL Read failed\n");
        return -1;
    }
    printf("Value received from driver: %d\n", value);

    close(fd);
    return 0;
} 