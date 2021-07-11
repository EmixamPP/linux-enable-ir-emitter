#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>
#include <string.h>

/*
* usage : enable-ir-emitter [device] [unit] [selector] [dataSize] [data ...] 
*       device : path to the infrared camera, e.g. /dev/video2
*       dataSize : wLength, an integer indicating the number of components in the Data Fragment [in decimal]
*       data : Data Fragment, each component separated by a space [in hexadecimal]
*       unit : 2 first symbols of wIndex [in hexadecimal]
*       selector : 2 first symbols of wValue [in hexadecimal]
*/
int main(int argc, char **argv) {
    int unit = strtol(argv[2], NULL, 16);
    int selector = strtol(argv[3], NULL, 16);

    int dataSize = strtol(argv[4], NULL, 10);
    __u8 data[dataSize]; 
    for (unsigned i = 0; i < dataSize; ++i)
        data[i] = strtol(argv[i+5], NULL, 16);

    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = dataSize,
        .data = (__u8*)&data,
    };

    const char* device = argv[1];
    int fd = open(device, O_WRONLY);
    if(fd < 0){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return EXIT_FAILURE;
    }

    errno = 0;
    int result = ioctl(fd, UVCIOC_CTRL_QUERY, &query);
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

