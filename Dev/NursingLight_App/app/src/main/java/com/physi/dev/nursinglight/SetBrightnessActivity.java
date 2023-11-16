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
import android.widget.SeekBar;
import android.widget.TextView;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;

public class SetBrightnessActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener, View.OnClickListener {

    private static final String TAG = SetBrightnessActivity.class.getSimpleName();

    private TextView tvBrightness;
    private SeekBar sbBrightness;

    private BluetoothLEManager bleManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_brightness);

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
                    data = data.substring(3, data.length() - 1);
                    sbBrightness.setProgress(Integer.parseInt(data));
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        tvBrightness.setText(String.valueOf(progress));
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onClick(View v) {
        String ctrlStr = "$";
        if(v.getId() == R.id.btn_get_brightness){
            ctrlStr += "37#";
        }else if (v.getId() == R.id.btn_set_brightness){
            ctrlStr += "27" + sbBrightness.getProgress() + "#";
        }
        bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, ctrlStr);
    }

    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        tvBrightness = findViewById(R.id.tv_brightness);
        sbBrightness = findViewById(R.id.sb_brightness);
        sbBrightness.setOnSeekBarChangeListener(this);
        Button btnGetBrightness = findViewById(R.id.btn_get_brightness);
        Button btnSetBrightness = findViewById(R.id.btn_set_brightness);
        btnGetBrightness.setOnClickListener(this);
        btnSetBrightness.setOnClickListener(this);
    }
}