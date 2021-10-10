#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h> 
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>

#include "execute_query.h"

/*
* Print the current or the max value of the uvc control for the indicated device. Each value separated by a space.
* usage : get_query [query_type] [device] [unit] [selector] [controlSize]
*       query_type : 0 for current, 1 for maximum, 2 for resolution
*       device : path to the infrared camera, e.g. /dev/video2
*       unit : 8 bits Extension unit ID [in decimal]
*       selector : 8 bits Control selector [in decimal]
*       controlSize : 16 bits Size of the uvc control [in decimal]
*
* exit code: 0 sucess
*            1 failure, see stderr output for more details
*            126 failure, unable to open a file descriptor for the camera
*/
int main(int argc, char **argv) {
    __u8 query_type;
    if (atoi(argv[1]) == 1) 
        query_type= UVC_GET_MAX;
    else if (atoi(argv[1]) == 2) 
        query_type= UVC_GET_RES;
    else 
        query_type = UVC_GET_CUR;
    
    const char *device = argv[2];
    __u8 unit = atoi(argv[3]);
    __u8 selector = atoi(argv[4]);
    __u16 controlSize = atoi(argv[5]);
    __u8 *control = (__u8*) malloc(controlSize * sizeof(__u8));
    memset(control, 0, controlSize);
    
    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = query_type,
        .size = controlSize,
        .data = control,
    };
    
    errno = 0;
    int fd = open(device, O_WRONLY);
    if(fd < 0 || errno){
        fprintf (stderr, "Unable to open a file descriptor for %s\n", device);
        return 126;
    }

    int result = executeUVCQuery(fd, &query);
    if (!result){
        for(unsigned i=0; i < controlSize; ++i)
            printf("%d ", control[i]);
        printf("\n");
    }

    close(fd);
    free(control);
    return result;
}
