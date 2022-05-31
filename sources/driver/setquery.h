#ifndef SETQUERY
#define SETQUERY

#include <stdint.h>
#include <linux/usb/video.h>

#include "executequery.h"

/**
 * @brief Change the current uvc control value for the camera device
 *
 * @param fd file descriptor of the camera device 
 * @param unit extension unit ID
 * @param selector control selector
 * @param controlSize size of the uvc control
 * @param control control value
 *
 * @return non zero if error
 **/
inline int set_uvc_query(const int fd, const uint8_t unit, const uint8_t selector, const uint16_t controlSize, uint8_t *control)
{
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = controlSize,
        .data = control,
    };

    return execute_uvc_query(fd, &query);
}

#endif
