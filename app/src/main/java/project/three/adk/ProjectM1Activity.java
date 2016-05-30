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
import android.widget.TextView;
import android.widget.Toast;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import it.moondroid.seekbarhint.library.SeekBarHint;

public class ProjectM1Activity extends Activity {

    private static final String TAG = ProjectM1Activity.class.getSimpleName();
    private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";
    private PendingIntent mPermissionIntent;
    private boolean mPermissionRequestPending;
    private UsbManager mUsbManager;
    private UsbAccessory mAccessory;
    private ParcelFileDescriptor mFileDescriptor;
    private FileInputStream mInputStream;
    private FileOutputStream mOutputStream;
    private TextView buttonStateTextView;
    private SeekBarHint seekBar;
    private Button buttonSendDataView;

    private Vibrator vibrator;
    private boolean isVibrating;

    private long prevMillis = 0;
    private final long intervalMillis = 2000;
    private long curMillis = 0;
    private float maxIr1VoltageSurUneSec = 0;

    private float ancienVoltage = 0;
    private float nouveauVoltage = 0;
    private boolean isGrowing = true; // lorsque l'on stagne, on considère que c'est vrai aussi
    private boolean isGrowingAncien = true;
    private boolean isTurnUp = false;

    private float ancienVoltageJ1 = 0;
    private float nouveauVoltageJ1 = 0;
    private final float SEUIL_VOLTAGE_J1 = 2.60f;

    private float ancienVoltageJ2 = 0;
    private float nouveauVoltageJ2 = 0;
    private final float SEUIL_VOLTAGE_J2 = 1.70f;

    private int nbTourJoueur1 = 0;
    private int nbTourJoueur2 = 0;
    Runnable commRunnable = new Runnable() {

        @Override
        public void run() {
            int ret = 0;

            //buffer pour les donnees a recevoir de la carte arduino
            final byte[] buffer = new byte[4 + 4];

            while (true) {
                try {
                    ret = mInputStream.read(buffer);
                } catch (IOException e) {
                    break;
                }

                ByteBuffer byteBuffer = ByteBuffer.wrap(buffer);
                byteBuffer.order(ByteOrder.LITTLE_ENDIAN); //arduino c'est du LITTLE_ENDIAN cf doc.
                byteBuffer.position(0);
                final float irVoltage = byteBuffer.getFloat();

                ancienVoltage = nouveauVoltage;
                nouveauVoltage = irVoltage;

                if(irVoltage < SEUIL_VOLTAGE_J2 && isTurnUp){
                    isTurnUp=false;
                }

                isGrowingAncien = isGrowing;
                isGrowing = nouveauVoltage > ancienVoltage;
                if (!isGrowing & isGrowingAncien & !isTurnUp) {
                    if (ancienVoltage > SEUIL_VOLTAGE_J1) {
                        nbTourJoueur1++;
                        isTurnUp = true;
                    } else if (ancienVoltage > SEUIL_VOLTAGE_J2) {
                        nbTourJoueur2++;
                        isTurnUp = true;
                    }
                }

                /*
                nouveauVoltageJ1 = irVoltage;
                if(ancienVoltageJ1 < SEUIL_VOLTAGE_J1 && nouveauVoltageJ1 > SEUIL_VOLTAGE_J1){
                    nbTourJoueur1++;
                }

                ancienVoltageJ2 = nouveauVoltageJ2;
                nouveauVoltageJ2 = irVoltage;
                if(ancienVoltageJ2 < SEUIL_VOLTAGE_J2 && nouveauVoltageJ2 > SEUIL_VOLTAGE_J2){
                    nbTourJoueur2++;
                }
*/
                //mise à jour de la vue utilisateur
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        String t = getApplicationContext().getResources().getString(R.string.nbTourJ1);
                        buttonStateTextView.setText("msg length : " + buffer.length + " = " + irVoltage + "V " + "\r\n" +
                                        "tour j1 :" + nbTourJoueur1 + "\r\n" +
                                        "tour j2 :" + nbTourJoueur2 + "\r\n" +
                                        "isGrowingAncien : " + isGrowingAncien + "\r\n" +
                                        "isGrowing : " + isGrowing + "\r\n" +
                                        "ancienVoltage : " + ancienVoltage + "\r\n" +
                                        "nouveauVoltage : " + nouveauVoltage + "\r\n" +
                                        "SEUIL_VOLTAGE_J1 : " + SEUIL_VOLTAGE_J1 + "\r\n" +
                                        "SEUIL_VOLTAGE_J2 : " + SEUIL_VOLTAGE_J2 + "\r\n" +
                                        "isTurnUp : " + isTurnUp + "\r\n"
                        );
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
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        openAccessory(accessory);
                    } else {
                        Log.d(TAG, "permission denied for accessory " + accessory);
                    }
                    mPermissionRequestPending = false;
                }
            } else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
                UsbAccessory accessory = intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
                if (accessory != null && accessory.equals(mAccessory)) {
                    closeAccessory();
                    manageArduinoDisconnected();
                }
            }
        }
    };

    /**
     * Called when the activity is first created.
     */
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

        seekBar = (SeekBarHint) findViewById(R.id.seekBar);
        seekBar.setMax(255);
        seekBar.setProgress(100);
        seekBar.incrementProgressBy(5);
        seekBar.setOnProgressChangeListener(new SeekBarHint.OnSeekBarHintProgressChangeListener() {
            @Override
            public String onHintTextChanged(SeekBarHint seekBarHint, int progress) {
                return String.format("Vitesse de %d", progress);
            }
        });

        buttonSendDataView = (Button) findViewById(R.id.sendDataBtn);
        buttonSendDataView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int seekBarValue = seekBar.getProgress();

                Toast.makeText(getApplicationContext(), "sendData : " + seekBarValue, Toast.LENGTH_SHORT).show();
                sendData(seekBarValue);
            }
        });

        buttonStateTextView = (TextView) findViewById(R.id.button_state_text_view);

        vibrator = ((Vibrator) getSystemService(VIBRATOR_SERVICE));
    }

    public void sendData(int nbBlink) {
        byte[] buffer = new byte[8];

        buffer[0] = (byte) nbBlink; // button says off, light is on

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

    /**
     * Called when the activity is paused by the system.
     */
    @Override
    public void onPause() {
        super.onPause();
        closeAccessory();
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
        vibrator.vibrate(1000); //vibre pendant 1000 ms
    }

    private void manageArduinoDisconnected() {
        Toast.makeText(getApplicationContext(), getApplicationContext().getString(R.string.arduinoDisconnected), Toast.LENGTH_LONG).show();
        startVibrate();
        //fermeture de cette activity
        this.finish();
    }
}