package com.physi.dev.nursinglight;

import androidx.annotation.NonNull;
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

import com.google.android.material.slider.Slider;
import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;

public class SetBrightnessActivity extends AppCompatActivity implements View.OnClickListener, Slider.OnChangeListener {

    private static final String TAG = SetBrightnessActivity.class.getSimpleName();

    private TextView tvBrightness;
    private Slider sbBrightness;

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
                    sbBrightness.setValue(Integer.parseInt(data));
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    @Override
    public void onClick(View v) {
        String ctrlStr = "$";
        if(v.getId() == R.id.btn_get_brightness){
            ctrlStr += "37#";
        }else if (v.getId() == R.id.btn_set_brightness){
            ctrlStr += "271" + (int)sbBrightness.getValue() + "#";
        }else if (v.getId() == R.id.btn_show_brightness){
            ctrlStr += "270" + (int)sbBrightness.getValue() + "#";
        }
        bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, ctrlStr);
    }

    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        tvBrightness = findViewById(R.id.tv_brightness);
        sbBrightness = findViewById(R.id.sb_brightness);
        sbBrightness.addOnChangeListener(this);
        Button btnGetBrightness = findViewById(R.id.btn_get_brightness);
        Button btnSetBrightness = findViewById(R.id.btn_set_brightness);
        Button btnShowBrightness = findViewById(R.id.btn_show_brightness);
        btnGetBrightness.setOnClickListener(this);
        btnSetBrightness.setOnClickListener(this);
        btnShowBrightness.setOnClickListener(this);
    }

    @SuppressLint("RestrictedApi")
    @Override
    public void onValueChange(@NonNull Slider slider, float value, boolean fromUser) {
        tvBrightness.setText(String.valueOf((int)value));
    }
}