#include "stdlib.h"
#include "stdio.h"
#include <jni.h>
#define TAG "INTEL_JNI_TEST"
#include <utils/Log.h>
#include "intel_aidltest_jni_JNIClient.h"
#include "socketclient_interface.h"
#include <pthread.h>
#include <sys/time.h>
#include "jnicallback_interface.h"

static const char* className="intel/aidltest/jni/JNIClient";
static jclass cls ; //global reference for calss JNICLient 
static jmethodID mid;
static JNIEnv* env = NULL;
static JavaVM *g_jvm = NULL;
static pthread_t pollprocessid;
//CHeck whether is is valid mode or not .
static int isValidMode(int mode )
{
	if(mode ==0 || mode ==1)
	{

		return 1;
	}else
	{

		return 0;
	}
	
}

static int getcurrentmode()
{

	int moderet = -1;
	LOGW("JNI, IN native_getCurrentMode");


	char response[1024] = {0};
	//Test GET MODE 
	moderet = SendCommandAndReceive(CMD_GETMODE,response,sizeof(response));
	if(-1!=moderet)
	{
		//moderet = ProcessResponse(CMD_GETMODE,response);

		LOGW("Got mode successfully from Daemon, mode is %d\n",moderet);
	
	}else
	{
		LOGW("SendCommandAndReceive for command %s failed \n",CMD_GETMODE);
	}

	
	return moderet;

}


static jint setmode(jint newmode)
{
		char response[1024] = {0};
		char cmdbuf[1024] = {0};
		int ret = 0;
		LOGW("Inside native_SetMode, the newmode value is %d",newmode);
	
		if(!isValidMode(newmode))
		{
			LOGW("Inside native_SetMode, INVLAID MODE to SET return");
			return -1;
		}
		
		//TEST SET MODE 
		memset(cmdbuf,0,sizeof(cmdbuf));
		sprintf(cmdbuf,"%s%d",CMD_SETMODE,newmode);
		ret = SendCommandAndReceive(cmdbuf,response,sizeof(response));
		if(-1!=ret)
		{
			
			LOGV("SendCommandAndReceive for command %s success \n",CMD_SETMODE);
		}else{
			LOGW("SendCommandAndReceive for command %s failed \n",CMD_SETMODE);
		}
	
		return ret;

}




//check the signature through javap -s -p classname
//!!!!!!Remeber : JNI CALLS HAVE TO BE DONE IN JNI THREAD !!!!! 
int callbackModeSwitchRequest(int newmode)
{
    JNIEnv* env = NULL;
	jmethodID mid_local = NULL;

	//Attach the native thread to virtual machine 
	if((*g_jvm)->AttachCurrentThread(g_jvm,(void**)&env,NULL)!=0)
	{
		 LOGE("AttachCurrentThread failed ");
        return -1;
	}else
	{
		LOGW("AttachCurrentThread SUCCESS ");

	}
	

	LOGW("In LibUnipoint ,start to invoke CallBackModeSwitchRequest");

	mid_local = (*env)->GetStaticMethodID(env,cls,"CallBackModeSwitchRequest","(I)I");
	if(NULL == mid_local)
	{
		LOGW("Can not find method CallBackModeSwitchRequest in class reference");
		return -1;
	}else
	{
		LOGW(" Successfully find method CallBackModeSwitchRequest in class reference");

	}



	(*env)->CallStaticIntMethod(env,cls,mid_local,newmode);

	return 0;


	
}

JNIEXPORT jint JNICALL native_getCurrentMode()
{
	
	return getcurrentmode();
	
}



//SET MODE from application 
JNIEXPORT jint JNICALL native_SetMode(jint newmode)
{
	return setmode(newmode);
}

static JNINativeMethod gMethods[] =
{	
//name, signature,functionpointer 
	{"getCurrentMode",  "()I", (void*)native_getCurrentMode},
	{"SetMode", "(I)I" , (void*)native_SetMode},

};


jint registerNativeMethods(JavaVM* vm, void* reserved)
{

    JNIEnv* env = NULL;
    jclass cls = NULL;

    if ((*vm)->GetEnv(vm,(void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        return -1;
    }

    cls = (*env)->FindClass(env,className);
    if (cls == NULL) {
        LOGE("Native registration unable to find class '%s'\n", className);
        return -1;
    }else
    {
		LOGW("Native registration unable to find class '%s' SUCCESSFULLy \n", className);

	}
    if ((*env)->RegisterNatives(env,cls, gMethods,
            sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        LOGE("RegisterNatives failed for '%s'\n", className);
        return -1;
    }

    return JNI_VERSION_1_4;

}



//Thread that handling events from driver 
void* Poll_processor(void* cnt)
{
	int error = 0;
	LOGV("INSIDE Poll_processor THREAD");


	
	
	
	
	pthread_exit((void*)error);

	return((void*)0);
}



jint JNI_OnLoad(JavaVM* vm,void* researved)
{

	int ret = 0;
	/*
	if(registerNativeMethods(vm,researved) !=JNI_VERSION_1_4)
	{
		LOGW("Register failed, JNI VERSION NOT MATCH");
		return -1;
		
	}
*/
	
	
	jclass cls_local = NULL;



	LOGV("JNI_ONLOAD");
	g_jvm = vm; //store the jvm for further usage .
	
	if ((*vm)->GetEnv(vm,(void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		  LOGE("ERROR: GetEnv failed\n");
		  return -1;
	}


	cls_local = (*env)->FindClass(env,className);
    if (cls_local == NULL) {
        LOGE("JNI_OnLoad unable to find class '%s'\n", className);
        return -1;
    }else
    {
		LOGW("Native registration find class '%s' SUCCESSFULLy \n", className);

	}

	cls = (*env)->NewGlobalRef(env,cls_local);
	if(NULL == cls)
	{
		LOGE("JNI_OnLoad unable to Create Global Reference for %s \n",className);
		return -1;

	}else
	{
		LOGW("Create Global Reference for class successfully");

	}

	






	ret = init_clientsocket();
	if(ret!=0)
	{
		
		LOGW("init_clientsocket failed");

	}




	
/*

	 if(pthread_create(&pollprocessid, NULL, (void *)Poll_processor, NULL) == 0){
			LOGV("pollprocessid thread create OK!\n");
	 }

*/

	
	return JNI_VERSION_1_4;


}

void JNI_OnUnload(JavaVM* vm,void* reserved){
	int ret = 0;
	
	LOGV("Call JNI_OnUnload ~~");
	ret = close_clientsocket();
	if(ret!=0)
	{
		
		LOGW("close_clientsocket failed");

	}

	
}



/*
 * Class:     intel_aidltest_jni_JNIClient
 * Method:    getCurrentMode
 * Signature: ()I
*/

JNIEXPORT jint JNICALL Java_intel_aidltest_jni_JNIClient_getCurrentMode
  (JNIEnv *env, jclass class)
{
	
	LOGW("in function Java_intel_aidltest_jni_JNIClient_getCurrentMode");

	return getcurrentmode();
}
  
/*
 * Class:     intel_aidltest_jni_JNIClient
 * Method:    SetMode
 * Signature: (I)I
*/
JNIEXPORT jint JNICALL Java_intel_aidltest_jni_JNIClient_SetMode
  (JNIEnv *env, jclass class, jint newmode)
{

	LOGW("in function Java_intel_aidltest_jni_JNIClient_SetMode");

	return setmode(newmode);
	
} 

