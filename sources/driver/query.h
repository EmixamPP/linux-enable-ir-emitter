#ifndef QUERY
#define QUERY

#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>

/**
 * @brief Execute an uvc query on the device indicated by the file descriptor
 *
 * @param fd file descriptor of the camera device
 * @param query uvc query to execute
 *
 * @return 1 if error, otherwise 0
 **/
inline int execute_uvc_query(const int fd, const struct uvc_xu_control_query *query)
{
     errno = 0;
     int result = ioctl(fd, UVCIOC_CTRL_QUERY, query);
     if (result || errno)
     {
          /* // ioctl debug not really useful for automated driver generation since linux-enable-ir-emitter v3
          fprintf(stderr, "Ioctl error code: %d, errno: %d\n", result, errno);
          switch (errno) {
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
          }*/
          return 1;
     }
     return 0;
}

/**
 * @brief Change the current uvc control value for the camera device
 *
 * @param fd file descriptor of the camera device
 * @param unit extension unit ID
 * @param selector control selector
 * @param controlSize size of the uvc control
 * @param control control value
 *
 * @return 1 if error, otherwise 0
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
