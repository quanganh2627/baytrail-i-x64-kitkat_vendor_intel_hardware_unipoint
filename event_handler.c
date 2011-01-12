/**
 * \file event_handler.c
 * \brief Unipoint daemon core
 *
 * Modified from AMI unipoint daemon implementation.
 * By Wenwei Cai
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/input.h>
#include <event_handler.h>
#include <event_dev.h>
#include <macros.h>
#include <uinput.h>
#include <gesture.h>



#ifdef CONFIG_USE_DAVI_ALGORITHM
//include the algorithm
#include "unipoint.h"
#include <time.h>
#include "statemachine.h"




#endif 
#define OUT_OF_MEM "Out of memory.\n"

#define CAPVALS_DEBUG 0
#define POINTER_DEBUG 0
#define BUF_SIZE 256
#define NAME "RMI4 Unipoint"
#define UNIPOINT_PRE_PATH "/sys/class/input/"

static char unipoint_device_path[BUF_SIZE];
static const unsigned int event_handles_max = 10;
volatile sig_atomic_t do_event_loop = 1;

typedef union {
    void * v;
    FILE * stream;
} handle_private;

typedef struct {
    struct pollfd *restrict pfds;
    unsigned int   nfds;
    handle_private *restrict private;
} event_handles;

static void handler(int signum)
{
    UNUSED(signum);
    do_event_loop = 0;	
}

static int check_name(void) {
    int fd;
    char buf[BUF_SIZE];

    fd = open(unipoint_device_path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    read(fd, buf, BUF_SIZE);
    close(fd);
    if (strncmp(buf, NAME, strlen(NAME)) == 0)
        return 1;
    else
        return 0;
}

int search_unipoint_device_path(void)
{
	struct dirent *entry;
	DIR *dirp;

	dirp = opendir(UNIPOINT_PRE_PATH);
	if (!dirp) {
		perror(UNIPOINT_PRE_PATH);
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dirp)) != NULL) {
		snprintf(unipoint_device_path, BUF_SIZE, "%s%s/device/name",
				UNIPOINT_PRE_PATH, entry->d_name);
		if (check_name() && !strncmp(entry->d_name, "event", 5)) {
			sprintf(unipoint_device_path, "%s%s", "/dev/input/", entry->d_name);
				return 1;
		}
	}
	return 0;
}

static char *unipoint_dev_path()
{
#if 0
    /* swcai - chmod /dev/input/event7 - hardcoded */
    return "/dev/input/event7";
#else
    if (search_unipoint_device_path()) 
        return unipoint_device_path;
    return "/dev/input/event7";
#endif
}

static void update_permission(const char *dev)
{
    struct stat sb;
    while (stat(dev, &sb) == -1) {
        sleep(1);
    }

    chmod(dev, S_IROTH);
}

static inline int event_handles_init(event_handles ** restrict handles)
{
    int error = 0;
    struct pollfd *pfds = NULL;
    handle_private *private = NULL;

    *handles = NULL;
    *handles = malloc(sizeof(event_handles));
    if (*handles == NULL) {
        unipoint_error("%s", OUT_OF_MEM);
        error = -ENOMEM;	
        return error;
    }

    pfds = calloc(event_handles_max, sizeof(struct pollfd));
    if (pfds == NULL) {
        unipoint_error("%s", OUT_OF_MEM);
        error = -ENOMEM;	
        goto fail;
    }

    private = calloc(event_handles_max, sizeof(handle_private));
    if (private == NULL) {
        unipoint_error("%s", OUT_OF_MEM);
        error = -ENOMEM;
        goto fail2;
    }

    (*handles)->pfds = pfds;
    (*handles)->private = private;
    (*handles)->nfds = 0;

    return error;

fail2:
    free(pfds);
fail:
    free(*handles);
    return error;
}

static inline void
event_handles_free(event_handles * const * restrict handles_p)
{
    if (*handles_p != NULL) {
        if ((*handles_p)->private != NULL) {
            free((*handles_p)->private);
        }
        if ((*handles_p)->pfds != NULL) {
            free((*handles_p)->pfds);
        }

        free(*handles_p);
    }
}

static inline int
event_setup_input_dev(event_handles * restrict handles, const char *path)
{
    int error = 0;
    handle_private *private = &handles->private[handles->nfds];

    error = event_dev_open(&private->stream, path);
    if (error) {
        unipoint_error("Could not open input device at %s.\n",
                path);
        return -EIO;
    }
    handles->pfds[handles->nfds].fd = fileno(private->stream);
    handles->pfds[handles->nfds].events = POLLIN|POLLPRI;
    handles->pfds[handles->nfds].revents = 0;
    handles->nfds++;

    return error;
}

static inline int convert(int distance)
{
    return (int)(distance * 0.1f);
}

static inline int convert_x(int distance)
{
    return convert(distance);
}

static inline int convert_y(int distance)
{
    return -convert(distance);
}

#ifdef CONFIG_USE_DAVI_ALGORITHM






/*
This function is used to send gesture to event hub 
*/
void event_send_gesture(
		const int gesture_mask,
		centroid_output cent_out,
		const int uinput_fd)
{

	int rel_x =  0;
	int rel_y = 0;

	switch(gesture_mask)
	{
	case MASK_VOLUME_UP:
		{

			 fprintf(stdout, "uinput event - volume up \n");
			 uinput_write(uinput_fd, EV_KEY, KEY_VOLUMEUP, 1);
			 uinput_syn(uinput_fd);
			  uinput_write(uinput_fd, EV_KEY, KEY_VOLUMEUP, 0);

		break;
		}
	case MASK_VOLUME_DOWN:
		{
			fprintf(stdout, "uinput event - volume down \n");
			uinput_write(uinput_fd, EV_KEY, KEY_VOLUMEDOWN, 1);
			uinput_syn(uinput_fd);
			uinput_write(uinput_fd, EV_KEY, KEY_VOLUMEDOWN, 0);


		break;
	}
	case MASK_RIGHT_GESTURE:
		fprintf(stdout, "uinput event - right\n"); 
		 uinput_write(uinput_fd, EV_KEY, KEY_RIGHT, 1); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		 uinput_syn(uinput_fd);

		 uinput_write(uinput_fd, EV_KEY, KEY_RIGHT, 0); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		//do right 
		break;
	case MASK_LEFT_GESTURE:
		fprintf(stdout, "uinput event - left\n"); 
		 uinput_write(uinput_fd, EV_KEY, KEY_LEFT, 1); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		 uinput_syn(uinput_fd);

		 uinput_write(uinput_fd, EV_KEY, KEY_LEFT, 0); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		//do right 

		break;
	case MASK_DOWN_GESTURE:
		fprintf(stdout, "uinput event - down\n");
		 uinput_write(uinput_fd, EV_KEY, KEY_DOWN, 1); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		 uinput_syn(uinput_fd);

		uinput_write(uinput_fd, EV_KEY, KEY_DOWN, 0); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 

		//do right 
		break;
	case MASK_UP_GESTURE:
		fprintf(stdout, "uinput event - up\n"); 
		 uinput_write(uinput_fd, EV_KEY, KEY_UP, 1); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
		 uinput_syn(uinput_fd);

		uinput_write(uinput_fd, EV_KEY, KEY_UP, 0); //what's the ev_value for KEY_LEFT, need verify ,what 's the meaning of it 
			//do right 
			break;
	case MASK_TAP_GESTURE:

		if(IsDoubleTap()==1)
		{
			uinput_write(uinput_fd, EV_KEY, BTN_MOUSE, 1);
	        uinput_syn(uinput_fd);
	        uinput_write(uinput_fd, EV_KEY, BTN_MOUSE, 0);
	        fprintf(stdout, "uinput event - tap\n");

		}
		// LIRONG : Should we change to ABS event ? or just leave it as left button click ? 
		break;
	case MASK_TAPHOLD_GESTURE:

		//which operation to define , tempary define as left mouse click without release 
		uinput_write(uinput_fd, EV_KEY, BTN_LEFT, 1);
		uinput_syn(uinput_fd);
		

			//do right 
			break;
	case MASK_POINTER_GESTURE:

		//QUESTION: what is vX, vY used for in this case , ignore currently 
		rel_x = cent_out.X;
		rel_y = cent_out.Y;

		fprintf(stdout, "Daemon Got From algorithm relative x:%d y:%d\n", rel_x, rel_y);
	/*   
		if ((abs(rel_x)*16/5) > abs(rel_y)) {
            uinput_write(uinput_fd, EV_REL, REL_X, convert_x(rel_x));
            uinput_write(uinput_fd, EV_REL, REL_Y, 0);
            fprintf(stdout, "Daemon Send uinput event relative x:%d y:%d\n", convert_x(rel_x), 0);
        } else {
            uinput_write(uinput_fd, EV_REL, REL_X, 0);
            uinput_write(uinput_fd, EV_REL, REL_Y, convert_y(rel_y));
            fprintf(stdout, "Daemon Send uinput event relative x:%d y:%d\n", 0, convert_y(rel_y));
        }
*/
 		uinput_write(uinput_fd, EV_REL, REL_X, convert_x(rel_x));
		uinput_write(uinput_fd, EV_REL, REL_Y, convert_y(rel_y));
		

		break;
	default:
		return;

	}


    uinput_syn(uinput_fd);
}




#else
static inline void event_send_gesture
(const int gesture_mask, const int rel_x, const int rel_y, const int flick_time, const int uinput_fd)
{
    if (!(gesture_mask & 0x1)) {
        if ((abs(rel_x)*16/5) > abs(rel_y)) {
            uinput_write(uinput_fd, EV_REL, REL_X, convert_x(rel_x));
            uinput_write(uinput_fd, EV_REL, REL_Y, 0);
            fprintf(stdout, "uinput event x:%d y:%d\n", convert_x(rel_x), 0);
        } else {
            uinput_write(uinput_fd, EV_REL, REL_X, 0);
            uinput_write(uinput_fd, EV_REL, REL_Y, convert_y(rel_y));
            fprintf(stdout, "uinput event x:%d y:%d\n", 0, convert_y(rel_y));
        }
    } else {
        uinput_write(uinput_fd, EV_KEY, BTN_MOUSE, 1);
        uinput_syn(uinput_fd);
        uinput_write(uinput_fd, EV_KEY, BTN_MOUSE, 0);
        fprintf(stdout, "uinput event - tap\n");
    }
    uinput_syn(uinput_fd);
}

#endif
/**
 * Main event dispatch point
 * 
 * Continuously called from event loop; waits on all input handles, 
 * \param handles File descriptors to watch for input (device, gesture thread)
 * \param uinput_fd uinput file descriptor for motion events
 * \return Whether to continue running the event loop
 */
#ifdef CONFIG_USE_DAVI_ALGORITHM

static long long  count;


#endif
static inline int
event_dispatcher(event_handles * restrict const handles, int uinput_fd, FILE *sample_fd, char *op)
{
    int error = 0, nfds, num_events;
    int rc = -1;
    int has_data = 0;
    int delay;
    int gs[11];


    memset(gs, 0, sizeof(unsigned int)*11);

	//If there is data from stored file
    if (sample_fd) {
	    if (*op == 'r') {
		   rc = fscanf(sample_fd, "%d %d %d %d %d %d %d %d %d %d %d %d\n", &delay, &gs[0], &gs[1], &gs[2], &gs[3], &gs[4],
				   &gs[5], &gs[6], &gs[7], &gs[8], &gs[9], &gs[10]);
		   if (rc == 12) {
			   static clock_t last_clock;
			   static int flag = 1;

			   if (flag) {
				   last_clock = clock();
				   flag = 0;
			   }
			   while (clock() - last_clock < delay) {
				   //fprintf(stdout, ".");
			   }
			   last_clock = clock();
			   has_data = 1;
		   }
	    }
    }

	//if there is no data from stored file, try to get from event device 
    if (!has_data) {
	    nfds = poll(handles->pfds, handles->nfds, -1);
	    if(nfds < 0) {
		if (errno == EINTR)
		    return error;
			perror("Error was: ");
			return nfds;
	    } 

		//there is data from event device ,store data into gs array, and if need record, record to file .
  		if (nfds > 0) {
			 /* Core for data from device */
		        if (handles->pfds[0].revents & (POLLIN | POLLPRI)) {
		            //unsigned int gs[11];
		         //   memset(gs, 0, sizeof(unsigned int)*11);
		            // read input event from device
		            if (!event_dev_get(handles->private[0].stream, gs)) {
					has_data = 1;
					if (sample_fd)
						if (*op == 'w') {
							static clock_t last_clock;
							static int flag = 1;
							static int _delay;

							if (flag) {
								_delay = 0;
								flag = 0;
							} else
								_delay = clock() - last_clock;
							last_clock = clock();

							fprintf(sample_fd, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
								_delay, gs[0], gs[1], gs[2], gs[3], gs[4], gs[5], gs[6], gs[7], gs[8], gs[9], gs[10]);
						}
				    }
		                handles->pfds[0].revents = 0;
			    }
		 }
    }//if there is no data from stored file 

//no matter where the data is from, process it here 		
    if (has_data) {

#ifdef CONFIG_USE_DAVI_ALGORITHM

	{//Process using Davi's algorithm 
		F11_centroid_registers cent;
		F11_gesture_registers gest;
		centroid_output cent_out;
		gesture_output gest_out;
		int ret = 0 ;


/*
	gs[0] Finger 
	gs[1]  X
	gs[2]  Y
	gs[3] W
	gs[4] Z
	gs[5] G1
	gs[6] G2
	gs[7] INFO0
	gs[8] INFO1
	gs[9] INFO2

*/

			
			count++;

			cent.finger = gs[0];
			cent.frameCount = count; // internal framecount ; 
			cent.X = gs[1]; // X
			cent.Y = gs[2]; //Y
			cent.Wx = gs[3]&0x0F;
			cent.Wy = (gs[3]&0xF0) >>4;
			cent.Xmax = F11_XMAX;
			cent.Ymax =  F11_YMAX;
			cent.Z = gs[4];

			
			unsigned int gesturedetection = gs[5]; 
			if(gesturedetection & 0x01 == 1)
			{
				gest.tapDone = 1;
				
			}else if(gesturedetection & 0x02 !=0)
			{
				gest.tapHold = 1;
			}else if(gesturedetection & 0x04 !=0)
			{
				//?? no tap double 
			}else if(gesturedetection & 0x08 !=0)
			{
				gest.tapEarly = 1;
			}else if(gesturedetection & 0x10 !=0)
			{
				gest.flick = 1;
			}

			if(gest.flick == 1)
			{
				gest.flickXdist = gs[7];
				gest.flickYdist = gs[8];

			}

			gest.frameCount = count;
			

			//Got the results in gs[input_expected] ,input_expected = 10 currrently
			ret	= process(&cent,&gest, &cent_out, &gest_out);			

			if(error !=0)
			{
			
				fprintf(stdout, "something wrong in process\n");
				return 0;
			}

			

			StateMachine_process(uinput_fd,gest_out,cent_out);
			
	}//Part for Davi's algorithm
#else 

{

//Process using Alek's algorithm 
        if (gs[5] & 0x11) {


					//event_send_gesture(gs[4], gs[6], gs[7], gs[8], uinput_fd); 
		} else { /* do our processing to rel */
			static int x[100], y[100], c[100];
			static int count;
			static clock_t last_clock;

			if (clock() - last_clock > 75000) {
				count = 0;
				fprintf(stdout, "@ reset\n");
			}

			c[count] = last_clock = clock();
			x[count] = gs[1] / 20;
			y[count] = gs[2] / 20;
			
			if (count) {
				int xdiff = x[count] - x[count - 1];
				int ydiff = y[count] - y[count - 1];
				int diff = xdiff * xdiff + ydiff * ydiff;

				if (diff < 4)
					return 0;
				if (diff > 36)
					return 0;
			}

			if (count == 4) {
				int xmax = 0, xmin = 4096;
				int xdiff;
				int xdir = 0;
				int ymax = 0, ymin = 4096;
				int ydiff;
				int ydir = 0;
				int cdiff;
				int loop;
				int sxy = 0, sx = 0, sy = 0, sx2 = 0, n = count + 1, a;

				fprintf(stdout, "@ ");
				for (loop = 0; loop < n; loop++) {
					fprintf(stdout, "(%d, %d) ", x[loop], y[loop]);
					if (xmax < x[loop])
						xmax = x[loop];
					if (xmin > x[loop])
						xmin = x[loop];
					if (ymax < y[loop])
						ymax = y[loop];
					if (ymin > y[loop])
						ymin = y[loop];
					sxy += x[loop] * y[loop];
					sx += x[loop];
					sy += y[loop];
					sx2 += x[loop] * x[loop];
				}
				//fprintf(stdout, "\n");
				fprintf(stdout, "sxy:%d, sx:%d, sy:%d, sx2:%d ", sxy, sx, sy, sx2);
				xdiff = xmax - xmin;
				ydiff = ymax - ymin;
				cdiff = c[count] - c[0];
				if (sx2 * n - sx * sx) {
					a = (sxy * n - sx*sy) * 100 /(sx2 * n - sx * sx);
					fprintf(stdout, "a = %d, xdiff = %d, ydiff = %d, cdiff = %d\n", a, xdiff, ydiff, cdiff);
				} else {
					a = 1000;
					fprintf(stdout, "a = max, xdiff = %d, ydiff = %d, cdiff = %d\n", xdiff, ydiff, cdiff);
				}

				if (ydiff >= xdiff * 2)
					ydir = ydiff * (y[n - 1] >= y[0] ? 1 : -1);
				else if (ydiff >= xdiff && a > 100)
					ydir = ydiff * (y[n - 1] >= y[0] ? 1 : -1);
				else	xdir = xdiff * (x[n - 1] >= x[0] ? 1 : -1);

				if (cdiff > 100000) {
					xdir /= 2;
					ydir /= 2;
				}
				event_send_gesture(0x10, xdir, 0, 10, uinput_fd);
				event_send_gesture(0x10, 0, ydir, 10, uinput_fd);
				count = 0;
			}
			else count++;
		}

}//Part for Alek's algorithm

#endif 

	   
	}

	 return error;
}

int event_loop(char *op, char *fname)
{
    int error;
    const char *dev_path = unipoint_dev_path();
    event_handles *handles = NULL;
    int uinput_fd;
    FILE *sample_fd;

    /* Setup uinput; open uinput file descriptor and set caps */
    error = uinput_gen_setup(&uinput_fd, &sample_fd, op, fname);
    if (error) {
        unipoint_error("Could not initialize uinput \n");
        goto free_handles;
    }

    error = event_handles_init(&handles);
    if (error) {
        unipoint_error("Could not initialize event handlers.\n");
        goto free_handles;
    }

    fprintf(stderr, "dev_path = [%s]\n", dev_path);
    update_permission(dev_path);
    error = event_setup_input_dev(handles, dev_path);
    if (error) {
        unipoint_error("Could not setup input dev.\n");
        goto free_handles;
    }

#ifdef  CONFIG_USE_DAVI_ALGORITHM
	initialize();
#endif 


   // signal(SIGINT, handler);
    while(do_event_loop)
        error = event_dispatcher(handles, uinput_fd, sample_fd, op);

    event_dev_close(handles->private[0].stream);

uinput_teardown:
    uinput_destroy(uinput_fd);

free_handles:
    event_handles_free(&handles);

    return error;
}
