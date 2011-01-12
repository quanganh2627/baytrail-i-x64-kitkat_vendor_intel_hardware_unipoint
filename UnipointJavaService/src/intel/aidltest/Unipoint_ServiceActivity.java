package intel.aidltest;


import intel.aidltest.jni.JNIClient;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class Unipoint_ServiceActivity extends Activity {
	
	public static final String SERVICEACTION = "intel.aidltest.service.UnipointService";
	
	private static final String TAG="Unipoint_ServiceActivity";

	public static final String COMMAND = "COMMAND";

	public static final int CMD_NOTIFY = 1;

	public static final String PAR_NEWMODE = "PAR_NEWMODE";

	public static final boolean bDEBUG = true;
	
	private TextView mInformationText;
	private Button mShowmodeButton;
	private Button mchangemodebutton0;
	private JNIClient client;
	private Button changemodebutton1;

	private Unipoint_ServiceActivity self;

	private Button mreconnecttodaemon;
	
	
	static {
		System.loadLibrary("Unipoint");
	}
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        self = this;
        client = new JNIClient(this);
        //Start Service
        
        Intent m = new Intent(SERVICEACTION);
        this.startService(m);
        
        init_screen();
        
    }
    
    
    private void init_screen() {
		mInformationText = (TextView)this.findViewById(R.id.informationtext);
		mShowmodeButton = (Button)this.findViewById(R.id.getmodebutton);
		changemodebutton1 = (Button)this.findViewById(R.id.changemodebutton1);
		
		mchangemodebutton0 = (Button)this.findViewById(R.id.changemodebutton);
		mreconnecttodaemon = (Button)this.findViewById(R.id.reconnecttodaemon);
		mreconnecttodaemon.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View arg0) {
			     
					Intent b = new Intent(SERVICEACTION);
					self.stopService(b);
					
					try {
						Thread.sleep(1000);
						self.startService(b);
						
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
			        

				
			}});
		
		
		
		mShowmodeButton.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				
			
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
				
				
				Toast.makeText(self, message, Toast.LENGTH_SHORT).show();
				
				
				
			}});
		//changemodebutton1
		
		mchangemodebutton0.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				
			
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
				
				
				
			}});
		
		
		changemodebutton1.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				
			
				Log.v(TAG,"TRYING to switch to MODE 1");
				
				JNIClient.SetMode(1);
				int data = JNIClient.getCurrentMode();
				Log.v(TAG,"Got unipoit Mode :"+data);
				

				String message = "";
				if(data == 1)
				{
					message = "switch to MODE VOLUME SUCCESS";
				}else if (data ==0)
				{
					message = "switch to MODE VOLUME FAILED";
					
				}
				Log.v(TAG,message);
				
				
				Toast.makeText(self, message, Toast.LENGTH_SHORT).show();

				
			}});
	}
	
}