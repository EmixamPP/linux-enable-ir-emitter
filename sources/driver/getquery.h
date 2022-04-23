#ifndef GETQUERY
#define GETQUERY

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/usb/video.h>

#include "executequery.h"

/** 
 * @brief Get the current, maximale, resolution or minimale value of the uvc control for the camera device
 * 
 * @param query_type UVC_GET_MAX, UVC_GET_RES, UVC_GET_CUR or UVC_GET_MIN
 * @param device path to the infrared camera /dev/videoX
 * @param unit extension unit ID 
 * @param selector control selector
 * @param controlSize size of the uvc control
 * @param control control value
 * 
 * @return non zero if error
 * Exit 126 if unable to open the camera device
 **/
inline int get_uvc_query(__u8 query_type, const char *device, __u8 unit, __u8 selector, __u16 controlSize, __u8 *control)
{   
    struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = query_type,
        .size = controlSize,
        .data = control,
    };

    errno = 0;
    int fd = open(device, O_WRONLY);
    if (fd < 0 || errno)
    {
        fprintf(stderr, "ERROR: Cannot access to %s\n", device);
        exit(126);
    }

    int result = execute_uvc_query(fd, &query);

    close(fd);
    return result;
}

#endif
