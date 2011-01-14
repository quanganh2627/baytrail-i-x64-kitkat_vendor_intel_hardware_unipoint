package intel.aidltest.service;

import intel.aidltest.UnipointMode;
import intel.aidltest.Unipoint_ServiceActivity;
import intel.aidltest.aidlsrc.IUnipointService;
import intel.aidltest.jni.JNIClient;
import android.app.Activity;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.Gravity;
import android.widget.Toast;

public class UnipointService extends Service{

	private static final String TAG = "UnipointService";
	private static int mcurrentmode;
	private static Context self;
	private NotificationDialog mNotificationDialog;
	private boolean threadDisable;
	private JNIClient client;

	static {
		System.loadLibrary("Unipoint");
	}

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return new UnipointSerivceimpl();
	}

	
	
	@Override
	public void onDestroy() {
		
		
		Log.v(TAG,"Unipoint Service Stoped");
		super.onDestroy();
		this.threadDisable = true;
	}



	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		
		self = this;
		
		mNotificationDialog = new NotificationDialog(self);
		mcurrentmode = UnipointMode.MODE_NORMAL;
		
		Log.v(TAG,"Unipoint Service Started");
		 client = new JNIClient(this);
	
		
		new Thread(new Runnable() {
			
				public void run() {
				while (!threadDisable) {
				try {
					Thread.sleep(60000);
					
					checkConnection();
					
				} catch (InterruptedException e) {
				}
			}
	
			}
	
		}).start();
		
		
	}



	protected void checkConnection() {
		
		try
		{
			int mode = JNIClient.checkConnection();
			if(mode<0)
			{
				Log.v("DEBUG","Daemon not connected, Needs tetry connection ");
				
			}
			
		}catch(Exception e)
		{
			e.printStackTrace();
		}
		
		
	}



	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// TODO Auto-generated method stub
		
		 Log.v(TAG,"onStartCommand");
		 if(null!=intent){
			 
			 Bundle b = intent.getExtras();
			 
			 if(null!=b)
			 {
				Log.v("DEBUG","FOUND BUNDLE inside intent");
				Integer cmd = b.getInt(Unipoint_ServiceActivity.COMMAND);
				if(null !=cmd ){
				switch(cmd)
				{
				case Unipoint_ServiceActivity.CMD_NOTIFY:
				{
					int newmode = b.getInt(Unipoint_ServiceActivity.PAR_NEWMODE);
					String message = "";
					if(newmode == 0)
					{
						message = "Unipoint Mode switch detected, new mode : MODE_NORMAL";
					}else if (newmode ==1)
					{
						message = "Unipoint Mode switch detected, new mode : MODE_VOLUME";
						
					}
					Log.v(TAG,message);
					
					mcurrentmode = newmode;
					mNotificationDialog.show(message, 2000, Color.WHITE, 18);
				
					
					break;
				}	
				default:
					break;
				
				}
				
				}else
				{
					Log.v(TAG,"No Bundle within intent");
				}
			}else
			{
				Log.v(TAG,"No Unipoint_ServiceActivity.COMMAND");
				
			}
		 }else
		 {
			 Log.v(TAG,"No intent");
		 }
		 
		return super.onStartCommand(intent, flags, startId);
	}



	//This function is called by 3rd party Applications 
	public class UnipointSerivceimpl extends IUnipointService.Stub{

		
		public int getCurrentMode() throws RemoteException {
			// TODO Auto-generated method stub
		
			Log.v(TAG,"Got getCurrentMode request");
			int data = JNIClient.getCurrentMode();
			Log.v(TAG,"Got unipoit Mode :"+data);
			
			String message = "";
			if(data == 0)
			{
				message = "Unipoint Mode Detected : MODE_NORMAL";
			}else if (data ==1)
			{
				message = "Unipoint Mode Detected: MODE_VOLUME";
				
			}
			Log.v(TAG,message);
			
			
			//Toast.makeText(self, message, Toast.LENGTH_SHORT).show();
			mcurrentmode = data;
			
			return mcurrentmode;
		}

		
		
		//This function is called by 3rd party Applications 
		
		public void setMode(int mode) throws RemoteException {
		
			Log.v(TAG,"Got setMode request");
			
			Log.v(TAG,"TRYING to switch to MODE 0");
			
			JNIClient.SetMode(0);
			int data = JNIClient.getCurrentMode();
			Log.v(TAG,"Got unipoit Mode :"+data);
			

			String message = "";
			if(data == 0)
			{
				message = "switch to MODE NORMAL SUCCESS";
				
			}else if (data ==1)
			{
				message = "switch to MODE NORMAL FAILED";
				
			}
			Log.v(TAG,message);
			
		
			
			Toast.makeText(self, message, Toast.LENGTH_SHORT).show();
			
			mcurrentmode = data;
		
			
		}
		
	}



}
