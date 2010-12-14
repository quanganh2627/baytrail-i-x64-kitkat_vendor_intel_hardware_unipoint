// DLG TODO Add UMG's official CopyRight Head

#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <unipoint.h>

// sample history raw and scratch pad
extern F11_centroid_registers gcentroidHistory[SWIPE_MAX_DURATION_FRAMES];
extern centroid_scratchpad gcentroidScratchPad[SWIPE_MAX_DURATION_FRAMES];
extern unsigned int gcentroidHistoryIndex;

// signal features
extern gesture_features ggestureFeatures;

// --- external functions for development/debug/QA support ---

// provide raw input history for current touch engagement
int 
centroidHistoryPull(F11_centroid_registers *cent, const unsigned int index)
{
	assert(index < gcentroidHistoryIndex);
	assert( gcentroidHistoryIndex > 0);

	*cent = gcentroidHistory[(gcentroidHistoryIndex - 1) - index];
	return 0;
}

// remove all history of current touch engagement
void
centroidHistoryClear()
{
	gcentroidHistoryIndex = 0;
}

// provide side of current engagement history
int
centroidHistoryCount(int * count)
{
	*count = gcentroidHistoryIndex;
	return 0;
}

// provide centroid features of currrent touch engagement
int 
centroidScratchPull(centroid_scratchpad * scratch, const unsigned int index)
{
	assert(index < gcentroidHistoryIndex);
	assert( gcentroidHistoryIndex > 0);

	*scratch = gcentroidScratchPad[(gcentroidHistoryIndex - 1) - index];
	return 0;
}

// provide gestures feature created from centroid history
int
gestureFeaPull(gesture_features * fea)
{
	*fea = ggestureFeatures;
	return 0;
}

// --- internal functions ---

// insert raw input to history for current touch engagement
void
centroidHistoryPush(const F11_centroid_registers cent)
{
	assert( gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES);

	if (gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES) {
		gcentroidHistory[gcentroidHistoryIndex] = cent; // does this copy 
		gcentroidHistoryIndex++;
	}
}


// classify the current touch history 
int classifyGesture() {

	// assume globals are set
	assert( gcentroidHistoryIndex > 0);

	float TapClosureYThresh = (float)gcentroidHistory[0].Ymax * 0.1;
	float TapClosureXThresh = (float)gcentroidHistory[0].Xmax * 0.2;

	float NoTrendThreshold = 0.5;
	float UDTrendThreshold = 0.40;
	float UpTrendThreshold = 2.0;
	float DownTrendThreshold = 0.5;

	// DLG TODO add J48 classifier if needed

	//LR simple criterion
	if(ggestureFeatures.LRVelTrig > 0.0) 
		return RIGHT_GESTURE;
	if(ggestureFeatures.LRVelTrig < 0.0)
		return LEFT_GESTURE;


	if(    (fabs(ggestureFeatures.LRSegSX - ggestureFeatures.LRSegEX) < TapClosureXThresh) 
		&& (fabs(ggestureFeatures.UDSegSY - ggestureFeatures.UDSegEY) < TapClosureYThresh) ){
				return TAP_GESTURE;
	}

	//UD simple critera
	float ratioUpDown = 0.0;

	if (ggestureFeatures.CoTrendUp == 0.0)
		ratioUpDown = 0.0;
	else if (ggestureFeatures.CoTrendDown == 0.0)
		ratioUpDown = UpTrendThreshold + 1.0;
	else 
		ratioUpDown = ggestureFeatures.CoTrendUp/ggestureFeatures.CoTrendDown;


	if(    //(ggestureFeatures.CoTrendNone + Oppos/(float)ggestureFeatures.count < NoTrendThreshold)
		 ((ggestureFeatures.CoTrendUp + ggestureFeatures.CoTrendDown)/(float)ggestureFeatures.frameCount > UDTrendThreshold) )
	{
		if(ratioUpDown > UpTrendThreshold) 
			return UP_GESTURE;
		if(ratioUpDown < DownTrendThreshold)
			return DOWN_GESTURE;
	}


	return NONE_GESTURE;

} // end classify gesture

// process raw data and story in centroid filtered history
void centroidFilterHistory() { 

	int smoothingKernel = 3;

	assert( gcentroidHistoryIndex < SWIPE_MAX_DURATION_FRAMES);
	assert( gcentroidHistoryIndex > 0);
	//must have enough data to use
	//if(gcentroidHistoryIndex <= smoothingKernel) return;

	int Ymax = gcentroidHistory[0].Ymax;
	int Xmax = gcentroidHistory[0].Xmax;
	
	float Xalpha = 0.0; //DLG TODO make single poles global constants in unipoint header
	float Yalpha = 0.5;
	float Wxalpha = 0.5;
	float Wyalpha = 0.5;

	float smoothXlast = gcentroidHistory[0].X;
	float smoothYlast = gcentroidHistory[0].Y;
	float smoothWxlast = gcentroidHistory[0].Wx;
	float smoothWylast = gcentroidHistory[0].Wy;

	//run single pole IIR for fast high frequency noise removal 
	for(int i = 0;i<gcentroidHistoryIndex;i++){
		smoothXlast = smoothXlast * (Xalpha) + gcentroidHistory[i].X * (1.0 - Xalpha);
		smoothYlast = smoothYlast * (Yalpha) + gcentroidHistory[i].Y * (1.0 - Yalpha);
		smoothWxlast = smoothWxlast * (Wxalpha) + gcentroidHistory[i].Wx * (1.0 - Wxalpha);
		smoothWylast = smoothWylast * (Wyalpha) + gcentroidHistory[i].Wy * (1.0 - Wyalpha);

		gcentroidScratchPad[i].smoothX = smoothXlast;
		gcentroidScratchPad[i].smoothY = smoothYlast;
		gcentroidScratchPad[i].smoothWx = smoothWxlast;
		gcentroidScratchPad[i].smoothWy = smoothWylast;

	} // end for each value entry

	//draw other values from smoothed values
	//DLG TODO ask SYNA what the proper unit (and mm) relationship is between Y and Wy
	for(int i = 0;i<gcentroidHistoryIndex;i++) {
			gcentroidScratchPad[i].X = gcentroidScratchPad[i].smoothX;
			gcentroidScratchPad[i].Y = gcentroidScratchPad[i].smoothY + (gcentroidScratchPad[i].smoothWy/(double)0xF)*(gcentroidHistory[i].Ymax/2.0);
	} // end for each value entry

	gcentroidScratchPad[0].dX = 0.0;
	gcentroidScratchPad[0].dY = 0.0;

	// derive raw velocities
	for(int i = 1;i<gcentroidHistoryIndex;i++) {
			gcentroidScratchPad[i].dX = gcentroidScratchPad[i].X - gcentroidScratchPad[i-1].X;
			gcentroidScratchPad[i].dY = gcentroidScratchPad[i].Y - gcentroidScratchPad[i-1].Y;
	} // end for each value entry
	
	// smooth LR velocities for edge analysis
	float LREdgeAlpha = 0.7;
	float velocityLRLast = gcentroidScratchPad[0].dX;
	gcentroidScratchPad[0].velocityLREdge = velocityLRLast;
	for(int i = 1; i<gcentroidHistoryIndex;i++) {
		velocityLRLast = velocityLRLast * (LREdgeAlpha) + gcentroidScratchPad[i].dX * (1.0 - LREdgeAlpha);
		gcentroidScratchPad[i].velocityLREdge = velocityLRLast;
	} // end instantanious edge velocity

	// run through slope signs for co-traveling slopes. 
	// see which domonates gesture
	int upCoTrend = 0;
	int downCoTrend = 0;
	int slopeCoTrend = 10;
	int noneCoTrend = 0;
	int oposCoTrend = 0;

	gcentroidScratchPad[0].CoTrendUp = 0;
	gcentroidScratchPad[0].CoTrendDown = 0;
	gcentroidScratchPad[0].CoTrendNone = 0;
	gcentroidScratchPad[0].CoTrendOppos = 0;


	for(int i = 1;i<gcentroidHistoryIndex;i++){

		gcentroidScratchPad[i].CoTrendUp = gcentroidScratchPad[i-1].CoTrendUp;
		gcentroidScratchPad[i].CoTrendDown = gcentroidScratchPad[i-1].CoTrendDown;
		gcentroidScratchPad[i].CoTrendNone = gcentroidScratchPad[i-1].CoTrendNone;
		gcentroidScratchPad[i].CoTrendOppos = gcentroidScratchPad[i-1].CoTrendOppos;

		//if both are moving in same direction
		if((gcentroidScratchPad[i].dY * (gcentroidScratchPad[i].smoothY - gcentroidScratchPad[i-1].smoothY ) > 0))
		{
			if(gcentroidScratchPad[i].dY > slopeCoTrend) { 
				upCoTrend++;
				gcentroidScratchPad[i].CoTrendUp = upCoTrend;
			} else if(gcentroidScratchPad[i].dY < -slopeCoTrend) {
				downCoTrend++;
				gcentroidScratchPad[i].CoTrendDown = downCoTrend;
			} else {
				noneCoTrend++;
				gcentroidScratchPad[i].CoTrendNone = noneCoTrend;
			}
		} else {
			oposCoTrend++;
			gcentroidScratchPad[i].CoTrendOppos = oposCoTrend;
		}
	} // end for each scratch pad
	
	// promote then ending trends to feature status
	ggestureFeatures.CoTrendUp = gcentroidScratchPad[gcentroidHistoryIndex-1].CoTrendUp;
	ggestureFeatures.CoTrendDown = gcentroidScratchPad[gcentroidHistoryIndex-1].CoTrendDown;
	ggestureFeatures.CoTrendNone = gcentroidScratchPad[gcentroidHistoryIndex-1].CoTrendNone;
	ggestureFeatures.CoTrendOppos =  gcentroidScratchPad[gcentroidHistoryIndex-1].CoTrendOppos;

	//now create dominate trends in Y
	float uptrend = 0.0;
	float downtrend = 0.0;
	float notrend = 0.0;
	float counttrend = 0.0;
	float startY = gcentroidScratchPad[0].Y;
	int   startIndexY = 0;
	float endY = gcentroidScratchPad[gcentroidHistoryIndex-1].Y;
	int   endIndexY = gcentroidHistoryIndex-1;
	float maxY = -1.0;
	int   maxIndexY = 0;
	float minY = Ymax +1;
	int   minIndexY = 0;

	//find max/min and end/start points
	for(int i=0;i<gcentroidHistoryIndex;i++) {
		
		if(maxY < gcentroidScratchPad[i].Y) {
			maxY = gcentroidScratchPad[i].Y;
			maxIndexY = i;
		} 
		if(minY >= gcentroidScratchPad[i].Y) {
			minY = gcentroidScratchPad[i].Y;
			minIndexY = i;
		} 
	}

	//find trends between peaks
	//gesture_direction_trend trend1, trend2, trend3, trend4;
	int	segment0 = startIndexY;
	int segment1 = (minIndexY < maxIndexY) ? minIndexY : maxIndexY;
	int	segment2 = (minIndexY >= maxIndexY)? minIndexY : maxIndexY;
	int	segment3 = endIndexY;
	int segCurrent  = 0;
	float segTrend01 = 0.0;
	float segTrend12 = 0.0;
	float segTrend23 = 0.0;

	for(int i=0;i<gcentroidHistoryIndex;i++) {
		
			//set segment to trend based on where i is w.r.t. segment indices
			if (i <= segment1 && segment1 != segment0)
				segCurrent = 1;
			else if((i <= segment2) && (segment2 != segment1))
				segCurrent = 2;
			else if((i <= segment3) && (segment3 != segment2))
				segCurrent = 3;

		switch(segCurrent) {
			case 1:
				segTrend01 += gcentroidScratchPad[i].dY;
				break;
			case 2:
				segTrend12 += gcentroidScratchPad[i].dY;
				break;
			case 3: 
				segTrend23 += gcentroidScratchPad[i].dY;
				break;
			default:
				break;
		}
	}

	// populate features struct
	ggestureFeatures.UDSegSY        = gcentroidScratchPad[segment0].Y;
	ggestureFeatures.UDSeg1Y        = gcentroidScratchPad[segment1].Y;
	ggestureFeatures.UDSeg2Y        = gcentroidScratchPad[segment2].Y;
	ggestureFeatures.UDSegEY        = gcentroidScratchPad[segment3].Y;
	ggestureFeatures.UDtrendS1      = segTrend01;
	ggestureFeatures.UDtrend12      = segTrend12;
	ggestureFeatures.UDtrend2E      = segTrend23;
	ggestureFeatures.UDindexS       = segment0;
	ggestureFeatures.UDindex1       = segment1;
	ggestureFeatures.UDindex2       = segment2;
	ggestureFeatures.UDindexE       = segment3;
	ggestureFeatures.frameCount     = gcentroidHistoryIndex;

	// find endpoints of exit segment
	// and measure movement against edge of sensor
	// find endpoints of start of segment
	// and measure movement againts 
	//now create dominate trends in X

	float lefttrend = 0.0;
	float righttrend = 0.0;
	float notlrtrend = 0.0;
	float threshlrtrend = Xmax/4;
	float startX = gcentroidScratchPad[0].X;
	int   startIndexX = 0;
	float endX = gcentroidScratchPad[gcentroidHistoryIndex-1].X;
	int   endIndexX = gcentroidHistoryIndex-1;
	float maxX = -1.0;
	int   maxIndexX = 0;
	float minX = Xmax + 1;
	int   minIndexX = 0;

	//find max/min and end/start points
	for(int i=0;i<gcentroidHistoryIndex;i++) {
		
		if(maxX < gcentroidScratchPad[i].X) {
			maxX = gcentroidScratchPad[i].X;
			maxIndexX = i;
		} 
		if(minX >= gcentroidScratchPad[i].X) {
			minX = gcentroidScratchPad[i].X;
			minIndexX = i;
		} 
	}

	//find trends between peaks
	//gesture_direction_trend trend1, trend2, trend3, trend4;
	segment0 = startIndexX;
	segment1 = (minIndexX <=  maxIndexX) ? minIndexX : maxIndexX;
	segment2 = (minIndexX <= maxIndexX)? maxIndexX : minIndexX;
	segment3 = endIndexX;
	segCurrent  = 0;
	segTrend01 = 0.0;
	segTrend12 = 0.0;
	segTrend23 = 0.0;

	for(int i=0;i<gcentroidHistoryIndex;i++) {
		
			//set you segment to trend
			if (i <= segment1 && segment1 != segment0)
				segCurrent = 1;
			else if((i <= segment2) && (segment2 != segment1))
				segCurrent = 2;
			else if((i <= segment3) && (segment3 != segment2))
				segCurrent = 3;

		switch(segCurrent) {
			case 1:
				segTrend01 += gcentroidScratchPad[i].dX;
				break;
			case 2:
				segTrend12 += gcentroidScratchPad[i].dX;
				break;
			case 3: 
				segTrend23 += gcentroidScratchPad[i].dX;
				break;
			default:
				break;
		}
	}

	// promote to features.
	ggestureFeatures.LRSegSX        = gcentroidScratchPad[segment0].X;
	ggestureFeatures.LRSeg1X        = gcentroidScratchPad[segment1].X;
	ggestureFeatures.LRSeg2X        = gcentroidScratchPad[segment2].X;
	ggestureFeatures.LRSegEX        = gcentroidScratchPad[segment3].X;
	ggestureFeatures.LRtrendS1      = segTrend01;
	ggestureFeatures.LRtrend12      = segTrend12;
	ggestureFeatures.LRtrend2E      = segTrend23;
	ggestureFeatures.LRindexS       = segment0;
	ggestureFeatures.LRindex1       = segment1;
	ggestureFeatures.LRindex2       = segment2;
	ggestureFeatures.LRindexE       = segment3;

	// find longest trend.
	float segXTrendMax = segTrend01;
	int segXStart = segment0;
	int segXEnd = segment1;

	if(fabs(segXTrendMax) < fabs(segTrend12)) {
		segXTrendMax = segTrend12;
		segXStart = segment1;
		segXEnd   = segment2;
	}
	if(fabs(segXTrendMax) < fabs(segTrend23)) {
		segXTrendMax = segTrend23;
		segXStart = segment2;
		segXEnd   = segment3;
	}

	//trigger on LR edge velocity
	float velocityTriggered = 0.0;
	bool startTriggered = false;
	float EdgeTrendThreshold = Xmax/5; //most move over 1/3 the sensor LR from peak (not start).	
	float EdgeVelocityThreshold = 20; // ticks per frame with 331 full scale 9mm

	// is this left to right movement 
	if (    (segXTrendMax > EdgeTrendThreshold) // enough peak to peak movement left to right
	    &&  (gcentroidScratchPad[segXEnd].X   > (Xmax - Xmax/10)) // and ending on edge
		&&  (gcentroidScratchPad[segXStart].X < (Xmax - Xmax/3)) )	// and starting off edge
	{ 

		//if velocity triggered as we neared the edge
		for(int i = segXStart;i<=segXEnd;i++) {
			if(gcentroidScratchPad[i].X > (Xmax - Xmax/5)) {
				startTriggered = true;
			}
			if(startTriggered) { // if we started off edge
				if(gcentroidScratchPad[i].velocityLREdge > EdgeVelocityThreshold) {
					velocityTriggered = gcentroidScratchPad[i].velocityLREdge;
					break;
				} 
			}  
		} // end trigger velocity
	}


	// is this left to right movement 
	if (    (fabs(segXTrendMax) > EdgeTrendThreshold) // enough peak to peak movement left to right
	    &&  (gcentroidScratchPad[segXEnd].X   < (Xmax/10)) // and ending on edge
		&&  (gcentroidScratchPad[segXStart].X > (Xmax/3)) )	// and starting off edge
	{ 

		//if velocity triggered as we neared the edge
		for(int i = segXStart;i<=segXEnd;i++) {
			if(gcentroidScratchPad[i].X < (Xmax/5)) {
				startTriggered = true;
			}
			if(startTriggered) { // if we started off edge
				if(fabs(gcentroidScratchPad[i].velocityLREdge) > EdgeVelocityThreshold) {
					velocityTriggered = gcentroidScratchPad[i].velocityLREdge;
					break;
				} 
			}  
		} // end trigger velocity
	}


	// populate features struct 
	ggestureFeatures.LRMaxSegSX	= gcentroidScratchPad[segXStart].X;
	ggestureFeatures.LRMaxEX    = gcentroidScratchPad[segXEnd].X;
	ggestureFeatures.LRMaxTrend = segXTrendMax;
	ggestureFeatures.LRVelTrig  = velocityTriggered;

	return;

} // end centroid filter history
