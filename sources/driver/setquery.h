#ifndef SETQUERY
#define SETQUERY

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/usb/video.h>

#include "executequery.h"

/**
 * @brief Change the current uvc control value for the camera device
 *
 * @param device path to the infrared camera /dev/videoX
 * @param unit extension unit ID
 * @param selector control selector
 * @param controlSize size of the uvc control
 * @param control control value
 *
 * @return non zero if error
 * Exit 126 if unable to open the camera device
 **/
inline int set_uvc_query(const char *device, const uint8_t unit, const uint8_t selector, const uint16_t controlSize, uint8_t *control)
{
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = controlSize,
        .data = control,
    };

    errno = 0;
    const int fd = open(device, O_WRONLY);
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
