package com.physi.dev.nursinglight;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;

public class SetThemeActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener, View.OnClickListener {

    private static final String TAG = SetThemeActivity.class.getSimpleName();

    private TextView tvRedColor, tvGreenColor, tvBlueColor;
    private SeekBar sbRedColor, sbGreenColor, sbBlueColor;

    private String themeNum = null;
    private BluetoothLEManager bleManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_theme);

        init();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bleManager.setHandler(handler);
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.btn_sync_themes){

        }else if(v.getId() == R.id.btn_set_theme){
            if(themeNum == null)
                return;
            String ctrlStr = "$2"
                    + themeNum
                    + zeroPad(sbRedColor.getProgress())
                    + zeroPad(sbGreenColor.getProgress())
                    + zeroPad(sbBlueColor.getProgress())
                    + "#";
            bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, ctrlStr);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if(isChecked){
            themeNum = buttonView.getTag().toString();
            Log.e(TAG, "Selected Theme Number : " + themeNum);
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if(seekBar.getId() == R.id.sb_red_color){
            tvRedColor.setText(String.valueOf(progress));
        }else if(seekBar.getId() == R.id.sb_green_color){
            tvGreenColor.setText(String.valueOf(progress));
        }else if(seekBar.getId() == R.id.sb_blue_color){
            tvBlueColor.setText(String.valueOf(progress));
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }


    private final Handler handler = new Handler(Looper.getMainLooper()){
        @SuppressLint({"MissingPermission", "SetTextI18n"})
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case BluetoothLEManager.BLE_DATA_AVAILABLE:
                    String data = (String) msg.obj;
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        tvRedColor = findViewById(R.id.tv_red_color);
        tvGreenColor = findViewById(R.id.tv_green_color);
        tvBlueColor = findViewById(R.id.tv_blue_color);

        sbRedColor = findViewById(R.id.sb_red_color);
        sbGreenColor = findViewById(R.id.sb_green_color);
        sbBlueColor = findViewById(R.id.sb_blue_color);

        sbRedColor.setOnSeekBarChangeListener(this);
        sbGreenColor.setOnSeekBarChangeListener(this);
        sbBlueColor.setOnSeekBarChangeListener(this);

        RadioButton btnTheme1 = findViewById(R.id.rbtn_theme1);
        RadioButton btnTheme2 = findViewById(R.id.rbtn_theme2);
        RadioButton btnTheme3 = findViewById(R.id.rbtn_theme3);
        RadioButton btnTheme4 = findViewById(R.id.rbtn_theme4);
        RadioButton btnTheme5 = findViewById(R.id.rbtn_theme5);
        btnTheme1.setOnCheckedChangeListener(this);
        btnTheme2.setOnCheckedChangeListener(this);
        btnTheme3.setOnCheckedChangeListener(this);
        btnTheme4.setOnCheckedChangeListener(this);
        btnTheme5.setOnCheckedChangeListener(this);

        Button btnSyncTheme = findViewById(R.id.btn_sync_themes);
        Button btnSetTheme = findViewById(R.id.btn_set_theme);

        btnSyncTheme.setOnClickListener(this);
        btnSetTheme.setOnClickListener(this);
    }

    private String zeroPad(int value){
        if(value < 10){
            return "00" + value;
        }else if(value < 100){
            return "0" + value;
        }else{
            return String.valueOf(value);
        }
    }
}