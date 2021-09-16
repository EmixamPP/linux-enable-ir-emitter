#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>
#include <string.h>


#include "execute_query.h"

/*
* Change the current uvc control value for the indicated device.
* usage : set_query [device] [unit] [selector] [controlSize] [control ...] 
*       device : path to the infrared camera, e.g. /dev/video2
*       unit : 8 bits Extension unit ID [in decimal]
*       selector : 8 bits Control selector [in decimal]
*       controlSize : 16 bits Size of the uvc control [in decimal]
*       control : control value, <controlSize> values in total separated by a space [in decimal]
*
* exit code: 0 sucess
*            1 failure, see stderr output for more details
*            126 failure, unable to open a file descriptor for the camera
*/
int main(int argc, char **argv) {
    const char* device = argv[1];
    __u8 unit = atoi(argv[2]);
    __u8 selector = atoi(argv[3]);
    __u16 controlSize = atoi(argv[4]);
    __u8 control[controlSize]; 
    for (unsigned i = 0; i < controlSize; ++i)
        control[i] = atoi(argv[i+5]);

    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = controlSize,
        .data = (__u8*)&control,
    };

    int fd = open(device, O_WRONLY);
    if(fd < 0){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return 126;
    }

    int result = executeUVCQuery(fd, &query);
    close(fd);
    return result;
}
