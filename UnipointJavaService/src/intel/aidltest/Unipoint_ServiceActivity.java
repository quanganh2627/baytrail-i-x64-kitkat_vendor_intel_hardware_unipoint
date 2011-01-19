package intel.aidltest;

import intel.aidltest.jni.JNIClient;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class Unipoint_ServiceActivity extends Activity {

    public static final String SERVICEACTION = "intel.aidltest.service.UnipointService";

    private static final String TAG = "Unipoint_ServiceActivity";

    public static final String COMMAND = "COMMAND";

    public static final int CMD_NOTIFY = 1;
    public static final int CMD_RECONNECT = 2;


    public static final String PAR_NEWMODE = "PAR_NEWMODE";

    public static final boolean bDEBUG = true;



    private TextView mInformationText;
    private Button mShowmodeButton;
    private Button mchangemodebutton0;
    private JNIClient client;
    private Button changemodebutton1;

    private Unipoint_ServiceActivity self;

    private int mCurrentmode = -1;
    private Button mreconnecttodaemon;

    private Button munipointonoffswitch;

    private boolean bUnipointEnabled = false;

    private Handler myHandler;


    static {
        System.loadLibrary("Unipoint");
    }


    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        self = this;

        // Start Service

        Intent m = new Intent(SERVICEACTION);
        this.startService(m);

        init_screen();

        getCurrentmode();
    }

    private void init_screen() {


        mInformationText = (TextView) this.findViewById(R.id.informationtext);
        mShowmodeButton = (Button) this.findViewById(R.id.getmodebutton);
        changemodebutton1 = (Button) this.findViewById(R.id.changemodebutton1);
        munipointonoffswitch = (Button)this.findViewById(R.id.unipointonoffswitch);

        mchangemodebutton0 = (Button) this.findViewById(R.id.changemodebutton);
        mreconnecttodaemon = (Button) this.findViewById(R.id.reconnecttodaemon);

        munipointonoffswitch.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                int ret = -1;


                if(!bUnipointEnabled) {

                    if (Unipoint_ServiceActivity.bDEBUG)
                        Log.v(TAG, "TRYING to switch to MODE 1, enable unipoint ");

                    ret = JNIClient.SetMode(1);

                } else
                {

                    if (Unipoint_ServiceActivity.bDEBUG)
                        Log.v(TAG, "TRYING to switch to MODE 0, disable unipoint ");
                    ret = JNIClient.SetMode(0);

                }

                if(ret!=-1)
                {
                    String message = "Success";
                    Toast.makeText(self, message, Toast.LENGTH_SHORT).show();
                } else
                {
                    Toast.makeText(self, "Failed to switch on/off", Toast.LENGTH_SHORT).show();
                }


                getCurrentmode();


            }
        });





        mreconnecttodaemon.setOnClickListener(new OnClickListener() {

            public void onClick(View arg0) {

                Intent b = new Intent(SERVICEACTION);
                self.stopService(b);

                try {
                    Thread.sleep(1000);


                    Bundle bundle = new Bundle();

                    bundle.putInt(Unipoint_ServiceActivity.COMMAND,
                                  Unipoint_ServiceActivity.CMD_RECONNECT);

                    b.putExtras(bundle);

                    self.startService(b);

                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }

            }
        });

        mShowmodeButton.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                int data = getCurrentmode();


                String message = "";
                if (data == 0) {
                    message = "Unipoint Mode Detected :DISABLED";
                } else if (data == 1) {
                    message = "Unipoint Mode Detected: MODE_NORMAL";

                } else if(data == 2 )
                {
                    message = "Unipoint Mode Detected: MODE_VOLUME";

                } else
                {
                    message = "Get Mode failed, return value "+data;
                }


                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, message);

                Toast.makeText(self, message, Toast.LENGTH_SHORT).show();

            }
        });
        // changemodebutton1

        mchangemodebutton0.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, "TRYING to switch to MODE 0");

                JNIClient.SetMode(0);
                int data = JNIClient.getCurrentMode();

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, "Got unipoit Mode :" + data);

                String message = "";
                if (data == 0) {
                    message = "Unipoint Mode Detected :DISABLED";
                } else if (data == 1) {
                    message = "Unipoint Mode Detected: MODE_NORMAL";

                } else if(data == 2 )
                {
                    message = "Unipoint Mode Detected: MODE_VOLUME";

                } else
                {
                    message = "Get Mode failed, return value "+data;
                }

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, message);

                Toast.makeText(self, message, Toast.LENGTH_SHORT).show();

            }
        });

        changemodebutton1.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, "TRYING to switch to MODE 1");

                JNIClient.SetMode(1);
                int data = JNIClient.getCurrentMode();

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, "Got unipoit Mode :" + data);

                String message = "";
                if (data == 0) {
                    message = "Unipoint Mode Detected :DISABLED";
                } else if (data == 1) {
                    message = "Unipoint Mode Detected: MODE_NORMAL";

                } else if(data == 2 )
                {
                    message = "Unipoint Mode Detected: MODE_VOLUME";

                } else
                {
                    message = "Get Mode failed, return value "+data;
                }

                if (Unipoint_ServiceActivity.bDEBUG)
                    Log.v(TAG, message);

                Toast.makeText(self, message, Toast.LENGTH_SHORT).show();

            }
        });
    }



    //This function is used to get current mode
    private int getCurrentmode() {

        try {


            int data = JNIClient.getCurrentMode();

            if (Unipoint_ServiceActivity.bDEBUG)
                Log.v(TAG, "Got unipoit Mode :" + data);


            mCurrentmode = data;


            if(mCurrentmode == -1)
            {
                mInformationText.setText("Status: Service Not Connected");
                munipointonoffswitch.setEnabled(false);

                //Service not connected

            } else if(mCurrentmode == 0) // it is None Mode
            {
                munipointonoffswitch.setEnabled(true);
                munipointonoffswitch.setText("Enable Unipoint");
                mInformationText.setText("Status: Service Connected, Unipoint Disabled ");
                bUnipointEnabled = false;

            } else if(mCurrentmode >=0)
            {
                munipointonoffswitch.setEnabled(true);
                munipointonoffswitch.setText("Disable Unipoint");
                mInformationText.setText("Status: Service Connected, Unipoint Enabled");
                bUnipointEnabled = true;
            }


            return data;

        } catch(Exception e)
        {
            e.printStackTrace();
        }
        return -1;
    }

}