package project.three.adk;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.os.Vibrator;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ProjectThreeActivity extends Activity {

	private static final String TAG = ProjectThreeActivity.class.getSimpleName();
	private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";
	private static final byte COMMAND_BUTTON = 0x1;
	private static final byte TARGET_BUTTON = 0x1;
	private static final byte VALUE_ON = 0x1;
	private static final byte VALUE_OFF = 0x0;
	private static final String BUTTON_PRESSED_TEXT = "The Button is pressed!";
	private static final String BUTTON_NOT_PRESSED_TEXT = "The Button is not pressed!";
    private PendingIntent mPermissionIntent;
    private boolean mPermissionRequestPending;
    private UsbManager mUsbManager;
    private UsbAccessory mAccessory;
    private ParcelFileDescriptor mFileDescriptor;
    private FileInputStream mInputStream;
    private FileOutputStream mOutputStream;
    private TextView buttonStateTextView;
    private SeekBar seekBarView;
    private Button buttonSendDataView;

	private Vibrator vibrator;
	private boolean isVibrating;

    private long prevMillis = 0;
    private long intervalMillis = 2000;
    private long curMillis = 0;
    private float maxIrVoltageSurUneSec = 0;
    private int nbTourJoueur1 = 0;
    private int nbTourJoueur2 = 0;
    Runnable commRunnable = new Runnable() {

        @Override
        public void run() {
            int ret = 0;

            final byte[] buffer = new byte[4+4];

            while (ret >= 0) {
                try {
                    ret = mInputStream.read(buffer);
                } catch (IOException e) {
                    break;
                }

                ByteBuffer byteBuffer = ByteBuffer.wrap(buffer);
                byteBuffer.order(ByteOrder.LITTLE_ENDIAN); //arduino c'est du LITTLE_ENDIAN cf doc.
                byteBuffer.position(0);
                final float irVoltage = byteBuffer.getFloat();

                if(maxIrVoltageSurUneSec < irVoltage){
                    maxIrVoltageSurUneSec = irVoltage;
                }

                curMillis = System.currentTimeMillis();
                //si une seconde s'est écoulée
                if ((curMillis - prevMillis) >= intervalMillis) {

                    if(maxIrVoltageSurUneSec < 3 && maxIrVoltageSurUneSec >= 2){
                        nbTourJoueur1++;
                    }else if(maxIrVoltageSurUneSec >= 3){
                        nbTourJoueur2++;
                    }

                    //reset pour la prochaine seconde
                    maxIrVoltageSurUneSec = 0;
                    prevMillis = System.currentTimeMillis();
                }

                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        buttonStateTextView.setText("msg length : "+ buffer.length +" = "+ irVoltage +"V "+"\r\n"+"tour j1 :"+ nbTourJoueur1 +"\r\n"+"tour j2 :"+nbTourJoueur2);
                    }
                });

            }
        }
    };
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbAccessory accessory = intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
                    if (intent.getBooleanExtra(
                            UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        openAccessory(accessory);
                    } else {
                        Log.d(TAG, "permission denied for accessory "
                                + accessory);
                    }
                    mPermissionRequestPending = false;
                }
            } else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
                UsbAccessory accessory = intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
                if (accessory != null && accessory.equals(mAccessory)) {
                    closeAccessory();
                }
            }
        }
    };

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(
				ACTION_USB_PERMISSION), 0);
		IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
		filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		registerReceiver(mUsbReceiver, filter);

        setContentView(R.layout.main);

        seekBarView = (SeekBar) findViewById(R.id.seekBar);

        buttonSendDataView = (Button) findViewById(R.id.sendDataBtn);
        buttonSendDataView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int seekBarValue = seekBarView.getProgress();

                Toast.makeText(getApplicationContext(), "sendData : "+ seekBarValue, Toast.LENGTH_SHORT).show();
                sendData(seekBarValue);
            }
        });

        buttonStateTextView = (TextView) findViewById(R.id.button_state_text_view);

        vibrator = ((Vibrator) getSystemService(VIBRATOR_SERVICE));
	}

    public void sendData(int nbBlink){
        byte[] buffer = new byte[8];

        buffer[0]= (byte) nbBlink; // button says off, light is on

        if (mOutputStream != null) {
            try {
                mOutputStream.write(buffer);
            } catch (IOException e) {
            }
        }
    }
	/**
	 * Called when the activity is resumed from its paused state and immediately
	 * after onCreate().
	 */
	@Override
	public void onResume() {
		super.onResume();

		if (mInputStream != null && mOutputStream != null) {
			return;
		}

		UsbAccessory[] accessories = mUsbManager.getAccessoryList();
		UsbAccessory accessory = (accessories == null ? null : accessories[0]);
		if (accessory != null) {
			if (mUsbManager.hasPermission(accessory)) {
				openAccessory(accessory);
			} else {
				synchronized (mUsbReceiver) {
					if (!mPermissionRequestPending) {
						mUsbManager.requestPermission(accessory,
								mPermissionIntent);
						mPermissionRequestPending = true;
					}
				}
			}
		} else {
			Log.d(TAG, "mAccessory is null");
		}
	}

	/** Called when the activity is paused by the system. */
	@Override
	public void onPause() {
		super.onPause();
		closeAccessory();
		stopVibrate();
	}

	/**
	 * Called when the activity is no longer needed prior to being removed from
	 * the activity stack.
	 */
	@Override
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mUsbReceiver);
	}

	private void openAccessory(UsbAccessory accessory) {
		mFileDescriptor = mUsbManager.openAccessory(accessory);
		if (mFileDescriptor != null) {
			mAccessory = accessory;
			FileDescriptor fd = mFileDescriptor.getFileDescriptor();
			mInputStream = new FileInputStream(fd);
			mOutputStream = new FileOutputStream(fd);
			Thread thread = new Thread(null, commRunnable, TAG);
			thread.start();
			Log.d(TAG, "accessory opened");
		} else {
			Log.d(TAG, "accessory open fail");
		}
	}

	private void closeAccessory() {
		try {
			if (mFileDescriptor != null) {
				mFileDescriptor.close();
			}
		} catch (IOException e) {
		} finally {
			mFileDescriptor = null;
			mAccessory = null;
		}
	}

	public void startVibrate() {
		if(vibrator != null && !isVibrating) {
			isVibrating = true;
			vibrator.vibrate(new long[]{0, 1000, 250}, 0);
		}
	}

    public void stopVibrate() {
		if(vibrator != null && isVibrating) {
			isVibrating = false;
			vibrator.cancel();
		}
	}
}