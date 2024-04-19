package com.physi.dev.nursinglight;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;
import com.physi.dev.nursinglight.list.BleAdapter;

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

public class ConnectActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener, View.OnClickListener, BleAdapter.OnItemListener {

    private static final String TAG = ConnectActivity.class.getSimpleName();

    private TextView tvLog;
    private Button btnScan;
    private Button btnSetTimer;
    private SeekBar sbTimer;
    private BluetoothLEManager bleManager;
    private BleAdapter bleAdapter;

    private boolean isConnected = false;
    private boolean isNotify = false;
    private Timer aliveTImer = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect);
        init();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bleManager.setHandler(handler);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bleManager.disconnect();
        bleManager.unregisterReceiver();
        bleManager.unBindService();
    }

    private final Handler handler = new Handler(Looper.getMainLooper()){
        @SuppressLint({"MissingPermission", "SetTextI18n"})
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case BluetoothLEManager.BLE_SCAN_START:
                    btnScan.setEnabled(false);
                    bleAdapter.clearItems();
                    tvLog.setText("Start Scanning..");
                    break;
                case BluetoothLEManager.BLE_SCAN_STOP:
                    btnScan.setEnabled(true);
                    tvLog.setText("Stop Scanning..");
                    break;
                case BluetoothLEManager.BLE_SCAN_DEVICE:
                    BluetoothDevice device = (BluetoothDevice) msg.obj;
                    bleAdapter.addItem(device);
                    break;
                case BluetoothLEManager.BLE_CONNECT_DEVICE:
                    isConnected = true;
                    tvLog.setText("Ble Device Connected..");
                    break;
                case BluetoothLEManager.BLE_SERVICES_DISCOVERED:
                    isNotify = bleManager.notifyCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX);
                    tvLog.setText("Ble service & notification : " + isNotify);
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    isConnected = false;
                    isNotify = false;
                    tvLog.setText("Ble Device Disconnect..");
                    break;
            }

        }
    };

    @Override
    public void onSelectedDevice(BluetoothDevice device) {
        if(isConnected)
            return;
        bleManager.scan(false,false);
        bleManager.connect(device);
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        btnSetTimer.setText("Timer(" + progress + ")");
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.btn_ble_scan){
            bleManager.disconnect();
            isConnected = false;
            isNotify = false;
            bleManager.scan(true,true);
        }else if(v.getId() == R.id.btn_ble_disconnect){
            bleManager.disconnect();
        }else if(isConnected && isNotify){
            if(v.getId() == R.id.btn_set_wifi){
//                startActivity(new Intent(ConnectActivity.this, SetWiFiActivity.class));
                startAliveTimer();
            }else if(v.getId() == R.id.btn_set_brightness){
                startActivity(new Intent(ConnectActivity.this, SetBrightnessActivity.class));
            }else if(v.getId() == R.id.btn_set_theme){
                startActivity(new Intent(ConnectActivity.this, SetThemeActivity.class));
            }else if(v.getId() == R.id.btn_set_bridge) {
                startActivity(new Intent(ConnectActivity.this, SetBridgeActivity.class));
            }else{
                String ctrlStr = "$1";
                if(v.getId() == R.id.btn_ctrl_on){
                    ctrlStr += "01#";
                }else if(v.getId() == R.id.btn_ctrl_off){
                    ctrlStr += "00#";
                }else if(v.getId() == R.id.btn_ctrl_timer){
                    ctrlStr += "1" + sbTimer.getProgress() + "#";
                }
                Log.e(TAG, "Ctrl Str : " + ctrlStr);
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, ctrlStr);
            }
        }
    }

    @SuppressLint({"UseCompatLoadingForDrawables", "SetTextI18n"})
    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());
        bleManager.bindService();
        bleManager.registerReceiver();

        tvLog = findViewById(R.id.tv_ble_log);
        tvLog.setText("Ble state message.");
        btnScan = findViewById(R.id.btn_ble_scan);
        Button btnDisconnect = findViewById(R.id.btn_ble_disconnect);
        Button btnOn = findViewById(R.id.btn_ctrl_on);
        Button btnOff = findViewById(R.id.btn_ctrl_off);
        btnSetTimer = findViewById(R.id.btn_ctrl_timer);
        Button btnSetWiFi = findViewById(R.id.btn_set_wifi);
        Button btnSetBrightness = findViewById(R.id.btn_set_brightness);
        Button btnSetTheme = findViewById(R.id.btn_set_theme);
        Button btnSetBridge = findViewById(R.id.btn_set_bridge);

        btnScan.setOnClickListener(this);
        btnDisconnect.setOnClickListener(this);
        btnOn.setOnClickListener(this);
        btnOff.setOnClickListener(this);
        btnSetTimer.setOnClickListener(this);
        btnSetWiFi.setOnClickListener(this);
        btnSetBrightness.setOnClickListener(this);
        btnSetTheme.setOnClickListener(this);
        btnSetBridge.setOnClickListener(this);

        sbTimer = findViewById(R.id.sb_run_timer);
        sbTimer.setOnSeekBarChangeListener(this);
        RecyclerView rcvBleList = findViewById(R.id.rcv_ble_list);
        DividerItemDecoration decoration
                = new DividerItemDecoration(getApplicationContext(), LinearLayoutManager.VERTICAL);
        decoration.setDrawable(getResources().getDrawable(R.drawable.item_division_line, null));
        rcvBleList.addItemDecoration(decoration);
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(ConnectActivity.this);
        linearLayoutManager.setOrientation(LinearLayoutManager.VERTICAL);
        linearLayoutManager.setItemPrefetchEnabled(true);
        rcvBleList.setLayoutManager(linearLayoutManager);
        rcvBleList.setAdapter(bleAdapter = new BleAdapter());
        bleAdapter.setOnItemListener(this);
    }

    private void startAliveTimer(){
        if(aliveTImer != null)
            return;
        TimerTask aliveTimeTask = new TimerTask() {
            @Override
            public void run() {
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$3A#");
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$32#");
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$33#");
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$34#");
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$37#");
                bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, "$4#");
            }
        };
        aliveTImer = new Timer();
        aliveTImer.schedule(aliveTimeTask, 100, 100);
        Log.e(TAG, "Alive Schedule.");
    }

    private void stopAliveTimer(){
        if(aliveTImer == null)
            return;
        aliveTImer.cancel();
        aliveTImer = null;
        Log.e(TAG, "Alive Cancel.");
    }
}