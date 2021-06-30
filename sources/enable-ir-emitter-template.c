#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>
#include <string.h>

int readDataSize(int argc, char **argv){
    for (unsigned i = 0; i < argc; ++i)
        if (! strcmp(argv[i], "-dataSize")) 
            return atoi(argv[++i]);

    fprintf (stderr, "Missing the dataSize argument\n");
    exit(EXIT_FAILURE);
}

void readData(int argc, char **argv, __u8 *data, unsigned dataSize){
    for (unsigned i = 0; i < argc; ++i)
        if (! strcmp(argv[i], "-data")){
            for(unsigned j = 0; j < dataSize; ++j)
                data[j] = strtol(argv[++i], NULL, 16);
            return;
        }
    fprintf (stderr, "Missing the data argument\n");
    exit(EXIT_FAILURE);
}

int readUnit(int argc, char **argv){
    for (unsigned i = 0; i < argc; ++i)
        if (! strcmp(argv[i], "-unit")) 
            return strtol(argv[++i], NULL, 16);

    fprintf (stderr, "Missing the unit argument\n");
    exit(EXIT_FAILURE);
}

int readSelector(int argc, char **argv){
    for (unsigned i = 0; i < argc; ++i)
        if (! strcmp(argv[i], "-selector")) 
            return strtol(argv[++i], NULL, 16);

    fprintf (stderr, "Missing the selector argument\n");
    exit(EXIT_FAILURE);
}

/*
* Four parameters are required: -data, -dataSize, -unit, -selector 
*       -dataSize : wLength, an integer indicating the number of components in the Data Fragment [in decimal]
*       -data : Data Fragment, each component separated by a space [in hexadecimal]
*       -unit : 2 first symbols of wIndex [in hexadecimal]
*       -selector : 2 first symbols of wValue [in hexadecimal]
*/
int main(int argc, char **argv) {
    int result, fd, dataSize, unit, selector;
    errno = 0;

    dataSize = readDataSize(argc, argv);
    __u8 data[dataSize]; 
    readData(argc, argv, data, dataSize);

    unit = readUnit(argc, argv);
    selector = readSelector(argc, argv);

    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = dataSize,
        .data = (__u8*)&data,
    };

    const char* device = "/dev/video2";
    if((fd = open(device, O_WRONLY)) < 0){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return EXIT_FAILURE;
    }

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

