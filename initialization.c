// DLG TODO Add UMG's official CopyRight Head

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <macros.h>
#include <unipoint.h>

// ballistics global support
// DLG TODO make structure in unipoint.h
extern point gpointRawLast;
extern point gvelocityRawLast;
extern ballistics gballistic;

extern F11_centroid_registers gcentroidHistory[SWIPE_MAX_DURATION_FRAMES];
extern centroid_scratchpad gcentroidStratchPad[SWIPE_MAX_DURATION_FRAMES];
extern unsigned int gcentroidHistoryIndex;

extern int gprocessMode; 
extern int grunningFrameCount;

int
initialize(void)
{
	int error = 0;

	//zero out location parameters (But need to redo ever time new finger contacts sensor)
	gprocessMode = PROC_IDLE;
	grunningFrameCount = 0;

	//set ballistics characteristics 
	// DLG TODO move to unipoint.h all subject to change
	gballistic.gain.X = 5e2;//5e2;
	gballistic.gain.Y = 5e2;//-1e2;
	gballistic.precision.X = 0.20;//0.05;
	gballistic.precision.Y = 0.20;//0.05;
	gballistic.vmax.X = 1.0;//0.39;
	gballistic.vmax.Y = 1.0;//0.53;
	gballistic.lower.X = F11_XMIN;
	gballistic.lower.Y = F11_YMIN;
	gballistic.upper.X = F11_XMAX;
	gballistic.upper.Y = F11_YMAX;

	gcentroidHistoryIndex = 0;

	return error;
} // end init

int release(void) {
	int error = 0;

	//DLG TODO place free here if we ever malloc

	return error;
} // end release 