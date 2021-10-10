#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/uvcvideo.h>
#include <linux/uvcvideo.h>

#include "execute_query.h"

/* 
* Execute an uvc query on the device indicated by the file descriptor
* if error return 1 and print in stderr the details, otherwise 0
*/
int executeUVCQuery(int fd, struct uvc_xu_control_query *query) {
     errno = 0;
     int result = ioctl(fd, UVCIOC_CTRL_QUERY, query);
     if (result || errno) {
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
          }
          return 1;
     }
     return 0;
}