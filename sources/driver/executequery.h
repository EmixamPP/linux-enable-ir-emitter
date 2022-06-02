#ifndef EXECUTEQUERY
#define EXECUTEQUERY

#include <errno.h>
#include <sys/ioctl.h>
#include <linux/uvcvideo.h>

/**
 * @brief Execute an uvc query on the device indicated by the file descriptor
 *
 * @param fd file descriptor of the camera device
 * @param query uvc query to execute
 *
 * @return non zero if error
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

#endif
