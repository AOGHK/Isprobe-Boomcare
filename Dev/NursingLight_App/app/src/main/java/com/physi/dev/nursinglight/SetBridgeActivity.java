package com.physi.dev.nursinglight;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;

public class SetBridgeActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = SetBridgeActivity.class.getSimpleName();

    private EditText etAddress;
    private TextView tvTmpValue;
    private BluetoothLEManager bleManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_bridge);

        init();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bleManager.setHandler(handler);
    }

    private final Handler handler = new Handler(Looper.getMainLooper()){
        @SuppressLint({"MissingPermission", "SetTextI18n"})
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case BluetoothLEManager.BLE_DATA_AVAILABLE:
                    String data = (String) msg.obj;
                    Log.e(TAG, data);
                    receiveMessage(data);
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    private void receiveMessage(String str){
        if(!str.startsWith("$") || !str.endsWith("#")){
            return;
        }

        if(str.charAt(1) == '5'){
            tvTmpValue.setText(str.substring(2, str.length() - 1));
        }else if(str.startsWith("$3C")){
            Toast.makeText(getApplicationContext(), "Boomcare Sound State : " + str.charAt(3) , Toast.LENGTH_SHORT).show();
        }else if(str.startsWith("$1B")){
            Toast.makeText(getApplicationContext(), "Brigde Mode : " + str.charAt(3) , Toast.LENGTH_SHORT).show();
        }
    }

    @SuppressLint("SetTextI18n")
    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        etAddress = findViewById(R.id.et_device_address);
        Button btnModeOn = findViewById(R.id.btn_mode_on);
        Button btnModeOff = findViewById(R.id.btn_mode_off);
        Button btnMuteOn = findViewById(R.id.btn_mute_on);
        Button btnMuteOff = findViewById(R.id.btn_mute_off);
        Button btnGetMute = findViewById(R.id.btn_get_mute);

        etAddress.setText("d9:75:34:9d:76:05");

        btnModeOn.setOnClickListener(this);
        btnModeOff.setOnClickListener(this);
        btnMuteOn.setOnClickListener(this);
        btnMuteOff.setOnClickListener(this);
        btnGetMute.setOnClickListener(this);

        tvTmpValue = findViewById(R.id.tv_tmp_value);
    }

    @Override
    public void onClick(View v) {
        String ctrlStr = "$";
        if(v.getId() == R.id.btn_mode_on){
            ctrlStr += "1B1" + etAddress.getText().toString();
        }else if(v.getId() == R.id.btn_mode_off){
            ctrlStr += "1B0";
        }else if(v.getId() == R.id.btn_mute_on){
            ctrlStr += "1C1";
        }else if(v.getId() == R.id.btn_mute_off){
            ctrlStr += "1C0";
        }else if(v.getId() == R.id.btn_get_mute){
            ctrlStr += "3C";
        }
        ctrlStr += "#";
        bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, ctrlStr);
    }
}