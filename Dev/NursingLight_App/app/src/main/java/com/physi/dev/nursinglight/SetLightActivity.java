package com.physi.dev.nursinglight;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import com.google.android.material.slider.Slider;
import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;

public class SetLightActivity extends AppCompatActivity {

    private static final String TAG = SetLightActivity.class.getSimpleName();

    private Slider sbBrightness;
    private Slider sbRedColor, sbGreenColor, sbBlueColor;
    private BluetoothLEManager bleManager;
    byte isFixed = 0x00;
    int[][] themeColors = new int[3][3];
    int themeNum = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_light);

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
                    byte[] data = (byte[]) msg.obj;
                    if(data.length != 17)
                        return;
                    for(byte b : data){
                        Log.e(TAG, "Val - " + (b & 0xFF));
                    }
                    setStateValues(data);
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    @SuppressLint("ClickableViewAccessibility")
    private final View.OnTouchListener touchListener = (v, event) -> {
        if(event.getAction() == 2) {
            isFixed = (byte) (event.getAction() == 2 ? 0x01 : 0x00);
            if (v.getId() == R.id.sb_brightness) {
                setBrightness((byte) 0);
            } else {
                setThemeColor((byte) 0);
            }
        }
        return false;
    };

    private final CompoundButton.OnCheckedChangeListener checkedListener = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if(isChecked){
                themeNum = Integer.parseInt(buttonView.getTag().toString());
                sbRedColor.setValue(themeColors[themeNum - 1][0]);
                sbGreenColor.setValue(themeColors[themeNum - 1][1]);
                sbBlueColor.setValue(themeColors[themeNum - 1][2]);
            }
        }
    };

    private void setBrightness(byte fixed){
        byte[] cmd = {0x24, 0x41, fixed, (byte) sbBrightness.getValue(), 0x23};
        boolean res = bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, cmd);
        Log.e(TAG, "brightness -> " + res);
    }

    private void setThemeColor(byte fixed){
        if(themeNum == 0){
            return;
        }
        byte[] cmd = {0x24, 0x40, (byte) themeNum, fixed,
                (byte) sbRedColor.getValue(), (byte) sbGreenColor.getValue(), (byte) sbBlueColor.getValue(), 0x23};
        boolean res = bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, cmd);
        Log.e(TAG, "themeColors -> " + res);
        if(fixed == 1 && res){
            themeColors[themeNum - 1][0] = (int) sbRedColor.getValue();
            themeColors[themeNum - 1][1] = (int) sbGreenColor.getValue();
            themeColors[themeNum - 1][2] = (int) sbBlueColor.getValue();
        }
    }

    private void setStateValues(byte[] data){
        if(data[0] != 0x24 || data[data.length - 1] != 0x23)
            return;

        themeNum = data[5];
        sbBrightness.setValue(data[6] & 0xFF);
        themeColors[0][0] = data[7] & 0xFF;
        themeColors[0][1] = data[8] & 0xFF;
        themeColors[0][2] = data[9] & 0xFF;

        themeColors[1][0] = data[10] & 0xFF;
        themeColors[1][1] = data[11] & 0xFF;
        themeColors[1][2] = data[12] & 0xFF;

        themeColors[2][0] = data[13] & 0xFF;
        themeColors[2][1] = data[14] & 0xFF;
        themeColors[2][2] = data[15] & 0xFF;
    }

    @SuppressLint("ClickableViewAccessibility")
    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        Button btnGetState = findViewById(R.id.btn_get_state);
        btnGetState.setOnClickListener(v -> {
            byte[] cmd = { 0x24, 0x51, 0x23 };
            bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, cmd);
        });

        sbBrightness = findViewById(R.id.sb_brightness);
        sbRedColor = findViewById(R.id.sb_red_color);
        sbGreenColor = findViewById(R.id.sb_green_color);
        sbBlueColor = findViewById(R.id.sb_blue_color);;

        sbBrightness.setOnTouchListener(touchListener);
        sbRedColor.setOnTouchListener(touchListener);
        sbGreenColor.setOnTouchListener(touchListener);
        sbBlueColor.setOnTouchListener(touchListener);

        RadioButton btnTheme1 = findViewById(R.id.rbtn_theme1);
        RadioButton btnTheme2 = findViewById(R.id.rbtn_theme2);
        RadioButton btnTheme3 = findViewById(R.id.rbtn_theme3);
        btnTheme1.setOnCheckedChangeListener(checkedListener);
        btnTheme2.setOnCheckedChangeListener(checkedListener);
        btnTheme3.setOnCheckedChangeListener(checkedListener);

        Button btSetBrightness = findViewById(R.id.btn_set_brightness);
        btSetBrightness.setOnClickListener(v -> setBrightness((byte) 1));

        Button btSetTheme = findViewById(R.id.btn_set_theme);
        btSetTheme.setOnClickListener(v -> setThemeColor((byte) 1));
    }

}