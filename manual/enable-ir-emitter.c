#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>

int main() {
    int result, fd;
    errno = 0;

    const char* device = "/dev/video2"; //your camera path
    if((fd = open(device, O_WRONLY)) < 0){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return EXIT_FAILURE;
    }

    //Data Fragment
    __u8 data[9] = {0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    //You can use my python data-seq-to-hex script for format the data fragment directly in the buffer format.
    //You just have to pass the data fragment in parameter
    //Don't forget to change the buffer size with the value of wLength

    struct uvc_xu_control_query query = {
        .unit = 0x0e, //2 first symbols of wIndex
        .selector = 0x06, //2 first symbols of wValue
        .query = UVC_SET_CUR,
        .size = 9, //wLength
        .data = (__u8*)&data,
    };

    result = ioctl(fd, UVCIOC_CTRL_QUERY, &query);
    if (result || errno) {
        fprintf(stderr, "Ioctl error code: %d, errno: %d\n", result, errno);
        switch(errno) {
           case ENOENT:
                fprintf(stderr, "The device does not support the given control or the specified extension unit could not be found.\n");
                break;
           case ENOBUFS:
                fprintf(stderr, "The specified buffer size is incorrect (too big or too small).\n");
                break;
           case EINVAL:
                fprintf(stderr, "An invalid request code was passed.\n");
                break;
           case EBADRQC:
                fprintf(stderr, "The given request is not supported by the given control.\n");
                break;
           case EFAULT:
                fprintf(stderr, "The data pointer references an inaccessible memory area.\n");
                break;
           case EILSEQ:
                fprintf(stderr, "Illegal byte sequence.\n");
                break;
        }
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}

