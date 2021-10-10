#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>

#include "execute_query.h"

/*
* Print the size of the uvc control for the indicated device.
* usage : len_query [device] [unit] [selector]
*       device : path to the infrared camera, e.g. /dev/video2
*       unit : 8 bits Extension unit ID [in decimal]
*       selector : 8 bits Control selector [in decimal]
*
* exit code: 0 sucess
*            1 failure, see stderr output for more details
*            126 failure, unable to open a file descriptor for the camera
*/
int main(int argc, char **argv) {
    const char *device = argv[1];
    __u8 unit = atoi(argv[2]);
    __u8 selector = atoi(argv[3]);
    __u8 len[2] = {0x00, 0x00};
    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_GET_LEN,
        .size = 2,
        .data = len,
    };

    errno = 0;
    int fd = open(device, O_WRONLY);
    if(fd < 0 || errno){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return 126;
    }

    int result = executeUVCQuery(fd, &query);
    if (!result)
        printf("%d\n", len[0] + len[1] * 16);

    close(fd);
    return result;
}
