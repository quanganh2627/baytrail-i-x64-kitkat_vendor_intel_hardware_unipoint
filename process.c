// DLG TODO Add UMG's official CopyRight Head

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <macros.h>
#include <unipoint.h>

// ballistic global constants
point gpointRawLast;
point gvelocityRawLast;
ballistics gballistic;

// scratch pad history for gesture feature extaction and ballistic
F11_centroid_registers gcentroidHistory[SWIPE_MAX_DURATION_FRAMES];
centroid_scratchpad gcentroidScratchPad[SWIPE_MAX_DURATION_FRAMES];
gesture_features ggestureFeatures;
unsigned int gcentroidHistoryIndex;

// statemechine variables
int gprocessMode;
int grunningFrameCount;

// Main processing loop called for each arrival of F11 registers
int
process(F11_centroid_registers *cent, F11_gesture_registers * gest, centroid_output *cent_out, gesture_output *gest_out)
{
	int error = 0;
	int gest_class = NONE_GESTURE;

	// ballistics stack support
	// DLG TODO partition into separate file/functions
	point pointRaw;
	point velocityRaw;
		
	pointRaw.X = (float)cent->X;
	pointRaw.Y = (float)cent->Y;

	// main touch onset state machine
	switch(gprocessMode) {
		case PROC_IDLE: // wait for finger to touch sensor
			//Event FingerOff 
			if(cent->finger == 0) {
				gest_class = NONE_GESTURE;
				gprocessMode = PROC_IDLE;
			} // end if finger off

			//Event FingerOn
			if(cent->finger == 1) {
				centroidHistoryClear();
				grunningFrameCount = 0;
				gcentroidHistoryIndex = 0;
				centroidHistoryPush(*cent);
				gest_class = NONE_GESTURE;
				gprocessMode = PROC_DEBOUNCE;
			}
			break;

		case PROC_DEBOUNCE: // allow for short settle time and remove onset missed frame tap firing
			//Event FingerOff 
			if(cent->finger == 0) {
				gest_class = NONE_GESTURE;
				gprocessMode = PROC_IDLE;
			} // end if finger off

			//Event FingerOn
			if(cent->finger == 1) {
				//collect path history for this contact
				if(gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES) {
					centroidHistoryPush(*cent);
				} else {
					//gesture segmentation too long
					gest_class = TIMEDOUT_GESTURE;
					gprocessMode = PROC_BALLISTICS_START;
				}

				//tally this frame
				grunningFrameCount++;
				//if too much time has elapsed then not a tap but maybe a swipe
				if(grunningFrameCount >= TOUCH_MIN_DURATION_FRAMES) {
					gprocessMode = PROC_TAP_N_SWIPE_MONITOR;
				}

				gest_class = NONE_GESTURE;

			} // end if finger on

		break;

	case PROC_TAP_N_SWIPE_MONITOR: // monitor for tap and short gestures
		//Event FingerOff 
		if(cent->finger == 0) {

			centroidFilterHistory();
			//DLG TODO likely change to 'tap specific' classifyInTapDuration();
			gest_class = classifyGesture();
			gprocessMode = PROC_IDLE;
		} // end if finger off

		//Event FingerOn
		if(cent->finger == 1) {

			//collect path history for this contact
			if(gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES) {
				centroidHistoryPush(*cent);
			} else {
				//gesture segmentation too long
				gest_class = TIMEDOUT_GESTURE;
				gprocessMode = PROC_BALLISTICS_START;
			}

			//tally this frame
			grunningFrameCount++;
			//if too much time has elapsed then not a tap but maybe a swipe
			if(grunningFrameCount > TAP_MAX_DURATION_FRAMES) {
				gprocessMode = PROC_SWIPE_MONITOR;
			}
			gest_class = NONE_GESTURE;

		} // end if finger on

		break;

	case PROC_SWIPE_MONITOR: // taps are not an option but swipe gestures UDLR are still possible
		//Event Finger Off
		if(cent->finger == 0) {
			centroidFilterHistory();
			gest_class = classifyGesture();
			gprocessMode = PROC_IDLE;
		}

		//Event Finger On
		if(cent->finger == 1) {

			//collect path history for this contact
			if(gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES) {
				centroidHistoryPush(*cent);
			} else {
				//gesture segmentation too long
				gest_class = TIMEDOUT_GESTURE;
				gprocessMode = PROC_BALLISTICS_START;
			}

			// tall this frame
			grunningFrameCount++;
			
			if(grunningFrameCount > SWIPE_MAX_DURATION_FRAMES) {
				gprocessMode = PROC_BALLISTICS_START;
			}
			gest_class = NONE_GESTURE;
		}
		break;

	case PROC_BALLISTICS_START: // taps and swipes have elapsed must be pointer
		//DLG TODO should we provide subscriber with 'retroactive centroid history' now that we know the early movement was not a swipe

		//Event Finger On first time history set up 
		//DLG TODO move to file/functions/structures
		if(cent->finger == 1) {
			gpointRawLast.X = pointRaw.X;
			gpointRawLast.Y = pointRaw.Y;
			gpointRawLast.X = pointRaw.X;
			gpointRawLast.Y = pointRaw.Y;
			gprocessMode = PROC_BALLISTICS_RUN;
		}

		//break; fall through on start so we don't wait for next frame to report

	case PROC_BALLISTICS_RUN: // pointer state with ballistics but no physics engine
		// Event Finger Off
		if(cent->finger == 0) { 
			gprocessMode = PROC_IDLE;
			gest_class = POINTER_GESTURE;
		}

		//Event finger On
		if(cent->finger == 1) {
	
			// run balistics
			velocityRaw.X = pointRaw.X - gpointRawLast.X;
			velocityRaw.Y = pointRaw.Y - gpointRawLast.Y;
			gpointRawLast.X = pointRaw.X;
			gpointRawLast.Y = pointRaw.Y;
			
			//DLG TODO do we need filter location noise here? SYNA seems to already be doing in FW.
			//DLG use Y + Wy tip so the thumb can move scroll in a 'pressing' fashion
			//DLG design a stable lift off because X Y will move before Finger Off?

			// convert absolute X Y data to relative movement
			point transformed_pt = normalize_relative(
						velocityRaw,
						gballistic.lower,
						gballistic.upper);

			// apply ballistics curve to changes in rate of change
			transformed_pt = ballistic_transform(
						gballistic.vmax,
						gballistic.gain,
						gballistic.precision,
						transformed_pt);

			// report results
			cent_out->X = transformed_pt.X;
			cent_out->Y = transformed_pt.Y;
			cent_out->Vx = velocityRaw.X;
			cent_out->Vy = velocityRaw.Y;
			gest_class = POINTER_GESTURE;

		}
		break;

	default:
		break;

	} // end switch 

	// share results
	//DLG fill in rest of output structure
	gest_out->type = gest_class;
	gest_out->magnitude = 0; //DLG does subscriber want this?
	gest_out->frameCount = grunningFrameCount;

	return error;

} // end process

