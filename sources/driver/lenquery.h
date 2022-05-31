#ifndef LENQUERY
#define LENQUERY

#include <stdint.h>
#include <linux/usb/video.h>

#include "executequery.h"

/**
 * @brief Get the size of the uvc control for the indicated device.
 *
 * @param fd file descriptor of the camera device
 * @param unit extension unit ID
 * @param selector control selector
 *
 * @return size of the control, 0 if error
 **/
inline uint16_t len_uvc_query(const int fd, const uint8_t unit, const uint8_t selector)
{
    uint8_t len[2] = {0x00, 0x00};
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_GET_LEN,
        .size = 2,
        .data = len,
    };

    if (execute_uvc_query(fd, &query))
        return 0;

    return (uint16_t)(len[0] + len[1] * 16); // UVC_GET_LEN is in little-endian
}

#endif
