// DLG TODO Add UMG's official CopyRight Head

#ifndef UNIPOINT_H_
#define UNIPOINT_H_

#ifdef __cplusplus
extern "C" {
#endif

//gesture names
#define NONE_GESTURE	    0
#define RIGHT_GESTURE	    1
#define LEFT_GESTURE	    2
#define DOWN_GESTURE	    3
#define UP_GESTURE		    4
#define TAP_GESTURE		    5
#define TAPHOLD_GESTURE	    6
#define POINTER_GESTURE     7
#define TIMEDOUT_GESTURE    8

// processing gesture modes
#define PROC_OFF					0
#define PROC_IDLE					1
#define PROC_DEBOUNCE				2
#define PROC_TAP_N_SWIPE_MONITOR    3
#define PROC_SWIPE_MONITOR			4
#define PROC_BALLISTICS_START		5
#define PROC_BALLISTICS_RUN			6

#define F11_PER_SEC 100.0
#define TOUCH_MIN_DURATION 0.03
#define TAP_MAX_DURATION 0.25
#define SWIPE_MAX_DURATION 1.0

#define TOUCH_MIN_DURATION_FRAMES ((int)(F11_PER_SEC * TOUCH_MIN_DURATION + 0.5))
#define TAP_MAX_DURATION_FRAMES ((int)(F11_PER_SEC * TAP_MAX_DURATION + 0.5))
#define SWIPE_MAX_DURATION_FRAMES ((int)(F11_PER_SEC * SWIPE_MAX_DURATION + 0.5)) 

#ifndef bool
    #define bool char
    #define false ((bool)0)
    #define true  ((bool)1)
#endif

typedef struct {
	float X;
	float Y;
} point;

typedef struct {
	unsigned int type;
	unsigned int magnitude;
	unsigned int frameCount;
} gesture_output;

typedef struct {
	float X;
	float Y;
	float Vx;
	float Vy;
} centroid_output;

typedef struct {
	float strenth;
	float slope;
	int   startIndex;
	int   endIndex;
} gesture_direction_trend;

typedef struct {
	float LRSegSX;	
	float LRSeg1X;  
	float LRSeg2X;   
	float LRSegEX;   
	float LRtrendS1; 
	float LRtrend12; 
	float LRtrend2E; 
	int   LRindexS;  
	int   LRindex1;  
	int   LRindex2;  
	int   LRindexE;  
	float LRMaxSegSX;
	float LRMaxEX;
	float LRMaxTrend; 

	float UDSegSY;       
	float UDSeg1Y;       
	float UDSeg2Y;       
	float UDSegEY;       
	float UDtrendS1;     
	float UDtrend12;     
	float UDtrend2E;     
	int   UDindexS;      
	int   UDindex1;      
	int   UDindex2;      
	int   UDindexE; 

	float UDMainVelocity; 
	float UDVelTrig;
	float LREdgeVelocity;
	float LRVelTrig;

	int   CoTrendUp;
	int   CoTrendDown;
	int   CoTrendNone;
	int   CoTrendOppos;

	int   frameCount;
} gesture_features;

/* RMI4 interface data representing a centroid */
typedef struct {
	unsigned int finger;//finger present
	unsigned int X;		//X location width
	unsigned int Y;		//Y location height
	unsigned int Xmax;  //Xmax location width
	unsigned int Ymax;	//Ymax location height
	unsigned int Wx;	//spread of current X profile
	unsigned int Wy;	//spread of current Y profile
	unsigned int Z;		//intensity of current contact
	unsigned int frameCount;//frame count
} F11_centroid_registers;

typedef struct {
	unsigned int flick;		//flick detected		
	unsigned int tapEarly;	//tap timeout start
	unsigned int tapHold;	//tap held before timeout ended
	unsigned int tapDone;	//tap timeout ended 
	signed int flickDistance; // overall flick distance from interface
	signed int flickXdist;	//X distance traveled flick
	signed int flickYdist;	//Y distance traveled during flick
	unsigned int flickDuration; // duration of flick
	unsigned int frameCount;//frame count - matched X,Y
} F11_gesture_registers;

typedef struct {
	float X;
	float Y;
	float smoothX;
	float smoothY;
	float dX;
	float dY;
	float velocityLREdge;
	float velocityUDMain;
	float smoothWx;
	float smoothWy;
	int CoTrendUp;
	int CoTrendDown;
	int CoTrendNone;
	int CoTrendOppos;
} centroid_scratchpad;

typedef struct {
	float startX;
	float startY;
	float dominateDerivativeX;
	float dominateDerivativeY;
	float peakX;
	float peakY;
	float endX;
	float endY;
	float minX;
	float minY;
} centroid_history_features;

typedef struct {
	point precision;
	point gain;
	point vmax;
	point upper;
	point lower;
} ballistics;

#define F11_XMAX ((double)354) // maximum value of X register (same as centroid_registers->Xmax)
#define F11_YMAX ((double)1353) // maximumn value of Y register (same as centroid_registers->Ymax)
#define F11_XMIN ((double)0) 
#define F11_YMIN ((double)0) 

//prototypes I/O
int initialize(void);
int process(F11_centroid_registers *cent, F11_gesture_registers * gest, centroid_output *cent_out, gesture_output *gest_out);
int release(void);
int centroidHistoryPull(F11_centroid_registers * cent, const unsigned int offset);
int centroidScratchPull(centroid_scratchpad * scratch, const unsigned int offset);
int gestureFeaPull(gesture_features * fea);
int centroidHistoryCount( int * count);


//prototypes internal
point ballistic_transform(const point v_max, const point gain, const point precision, const point v_in);
point normalize_relative(const point input, const point lower, const point upper);
void centroidHistoryPush(F11_centroid_registers cent);
void centroidFilterHistory(void);
int  classifyGesture(void);
void centroidHistoryClear(void);

#ifdef __cplusplus
}
#endif

#endif/*UNIPOINT_H_*/
