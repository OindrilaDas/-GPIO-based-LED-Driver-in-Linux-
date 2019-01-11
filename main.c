#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <errno.h>
#include "query_ioctl.h"

int mouseflag=0;

// Mouse click detect function


void *mouse_click()

{

       int fd;


                  struct input_event mouse_event;

                  // To open mouse event file descriptor

                  if ((fd = open("/dev/input/event2", O_RDONLY)) == -1) {

                             perror("opening device");

                             exit(EXIT_FAILURE);

                  }

                  // Creating a structure for polling mouse events

                  struct pollfd poll_mouse;

                  poll_mouse.fd = fd;

                  poll_mouse.events = POLLIN;

                  poll_mouse.revents = 0;


                  while (1) {


                             int poll_ret = poll(&poll_mouse, 1, -1);

                             if (poll_ret == -1) {

                                        printf("Poll Error!!");

                             }


                             if (poll_mouse.revents & POLLIN) {

                                        poll_mouse.revents = 0;


                                        // Reading the mouse event file


                                        int read_ret = read(fd, &mouse_event, sizeof(struct input_event));

                                        if (read_ret == -1) {

                                                   printf("Error reading mouse event\n");

                                        }


                                        // Event for detecting LEFT Mouse button release


                                        if ((mouse_event.type == EV_KEY) && (mouse_event.code == BTN_LEFT)

                                                              && (mouse_event.value == 1)) {

                                                   printf("left mouse click detected\n");
                            mouseflag=1;



                                        }

                                        // Event for detecting RIGHT Mouse button release


                                        if ((mouse_event.type == EV_KEY) && (mouse_event.code == BTN_RIGHT)

                                                              && (mouse_event.value == 1)) {

                                                   printf("right mouse click detected\n");
                            mouseflag=1;

                                        }


                             }


                  }

                  pthread_exit(0);

   }



int main(int argc, char *argv[])
{ 
   pthread_t tid;
   pthread_create(&tid,NULL,mouse_click,NULL);
   query_arg_t q;
   int j=0;
    
    if(argc!=5)      // if number of inputs is not 4, it prints error//
   {
      printf("\nERROR: Give only 4 inputs: PWM, R, G, B");
      return 0;
   }
  
  // taking inputs
 q.PWM = atoi(argv[1]);        // input value of PWM
 q.IO_R = atoi(argv[2]);       // IO pins for Red
 q.IO_G = atoi(argv[3]);       // IO pins for Green
 q.IO_B = atoi(argv[4]);       // IO pins for Blue
 
    
      
 // Displaying LED sequence
  
  int fd1 = open("/dev/RGBLed", O_WRONLY); 
  if(fd1==-1)
  {
  	printf("Error Opening RGBLed Driver\n");
  }
  
  int i = ioctl(fd1, CONFIG, &q);
  if(i==-1)
  {
  	exit(0);
  } 
  

while(j<4)
{

	BEGIN: write(fd1, "0", 1);                          // RGB Off 
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
              
                                                 // R On GB Off
	       write(fd1, "4", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                 // G On RB Off
	       write(fd1, "2", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                 // B On RG Off
	       write(fd1, "1", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                 // RG On B Off
	       write(fd1, "6", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                 // RB On G Off
	       write(fd1, "5", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                  // GB On R Off
	       write(fd1, "3", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
                
                                                  // RGB On
	       write(fd1, "7", 1);
	       usleep(500000);
	       if(mouseflag==1)
                {
                    mouseflag=0;
                    goto BEGIN;
                   
                }
j++;
}

   close(fd1);
   return 0;
 
}

