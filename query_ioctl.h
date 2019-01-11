#ifndef QUERY_IOCTL_H
#define QUERY_IOCTL_H
#include <linux/ioctl.h>
 
typedef struct
{
    int PWM, IO_R, IO_G, IO_B, R_in, G_in, B_in;
} query_arg_t;

#define CONFIG _IO('q',1)
#endif
