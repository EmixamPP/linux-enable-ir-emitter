#ifndef LENQUERY
#define LENQUERY

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/usb/video.h>

#include "executequery.h"

/**
 * @brief Get the size of the uvc control for the indicated device.
 *
 * @param device path to the infrared camera /dev/videoX
 * @param unit extension unit ID
 * @param selector control selector
 *
 * @return size of the control, 0 if error
 * Exit 126 if unable to open the camera device
 **/
inline __u16 len_uvc_query(const char *device, __u8 unit, __u8 selector)
{
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

    if (fd < 0 || errno)
    {
        fprintf(stderr, "ERROR: Cannot access to %s\n", device);
        exit(126);
    }

    int result = execute_uvc_query(fd, &query);
    if (result)
    {
        close(fd);
        return 0;
    }

    close(fd);
    return (__u16) (len[0] + len[1] * 16); // UVC_GET_LEN is in little-endian
}

#endif
