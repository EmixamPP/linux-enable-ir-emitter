#ifndef GETQUERY
#define GETQUERY

#include <stdint.h>
#include <linux/usb/video.h>

#include "executequery.h"

/**
 * @brief Get the current, maximale, resolution or minimale value of the uvc control for the camera device
 *
 * @param query_type UVC_GET_MAX, UVC_GET_RES, UVC_GET_CUR or UVC_GET_MIN
 * @param fd file descriptor of the camera device
 * @param unit extension unit ID
 * @param selector control selector
 * @param controlSize size of the uvc control
 * @param control control value
 *
 * @return 1 if error, otherwise 0
 **/
inline int get_uvc_query(uint8_t query_type, const int fd, const uint8_t unit, const uint8_t selector, const uint16_t controlSize, uint8_t *control)
{
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = query_type,
        .size = controlSize,
        .data = control,
    };

    return execute_uvc_query(fd, &query);
}

#endif
