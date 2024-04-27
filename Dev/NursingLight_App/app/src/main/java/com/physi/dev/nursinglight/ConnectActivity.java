package com.physi.dev.nursinglight;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;
import com.physi.dev.nursinglight.list.BleAdapter;

import java.util.LinkedList;
import java.util.List;

public class ConnectActivity extends AppCompatActivity implements View.OnClickListener, BleAdapter.OnItemListener {

    private static final String TAG = "BC-LIGHT";

    private TextView tvLog, tvPower, tvThermometer;
    private SeekBar sbTimer;
    private Button btnSetTimer;
    private BluetoothLEManager bleManager;
    private BleAdapter bleAdapter;

    private boolean isConnected = false;

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
                    bleAdapter.clearItems();
                    tvLog.setText("Start Scanning..");
                    break;
                case BluetoothLEManager.BLE_SCAN_STOP:
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
                    isConnected = bleManager.notifyCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX);
                    tvLog.setText("Light Connected : " + isConnected);
                    break;
                case BluetoothLEManager.BLE_DATA_AVAILABLE:
                    byte[] data = (byte[]) msg.obj;
                    syncStatus(data);
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    isConnected = false;
                    tvPower.setTextColor(Color.DKGRAY);
                    tvThermometer.setTextColor(Color.DKGRAY);
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

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.btn_ble_scan){
            bleManager.disconnect();
            bleManager.scan(true,true);
        }else if(v.getId() == R.id.btn_ble_disconnect){
            bleManager.disconnect();
        }else if(isConnected){
            if(v.getId() == R.id.btn_set_wifi){
                startActivity(new Intent(ConnectActivity.this, SetWiFiActivity.class));
            }else if(v.getId() == R.id.btn_set_light){
                startActivity(new Intent(ConnectActivity.this, SetLightActivity.class));
            }else{
                sendCommand(v.getId());
            }
        }
    }

    private void syncStatus(byte[] sta){
        if(sta[0] != 0x24 || sta[sta.length - 1] != 0x23)
            return;
        if(sta[1] == 0x55){
            tvPower.setTextColor(sta[2] == 0x01 ? Color.GREEN : Color.DKGRAY);
        }else if(sta[1] == 0x52 && sta[2] == 0x01){
            tvThermometer.setTextColor(Color.GREEN);
            String str = String.format("Thermometer - %02x:%02x:%02x:%02x:%02x:%02x",
                    sta[4] & 0xFF, sta[5] & 0xFF, sta[6] & 0xFF, sta[7] & 0xFF, sta[8] & 0xFF, sta[9] & 0xFF);
            tvThermometer.setText(str);
        }
    }

    private void sendCommand(int _id){
        List<Byte> cmd = new LinkedList<>();
        cmd.add((byte) 0x24);
        if(_id == R.id.btn_ctrl_on){
            cmd.add((byte) 0x30);
            cmd.add((byte) 0x01);
        }else if(_id == R.id.btn_ctrl_off){
            cmd.add((byte) 0x30);
            cmd.add((byte) 0x00);
        }else if(_id == R.id.btn_ctrl_timer){
            int sec = sbTimer.getProgress();
            cmd.add((byte) 0x31);
            cmd.add((byte)((sec >>> 8) & 0xFF));
            cmd.add((byte)(sec & 0xFF));
        }else if(_id == R.id.btn_sound_on){
            cmd.add((byte) 0x33);
            cmd.add((byte) 0x01);
        }else if(_id == R.id.btn_sound_off){
            cmd.add((byte) 0x33);
            cmd.add((byte) 0x00);
        }
        cmd.add((byte) 0x23);
        byte[] _cmd = new byte[cmd.size()];
        for(int i = 0; i < _cmd.length; i++){
            _cmd[i] = cmd.get(i);
        }
        bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX, _cmd);
    }

    @SuppressLint({"UseCompatLoadingForDrawables", "SetTextI18n"})
    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());
        bleManager.bindService();
        bleManager.registerReceiver();

        tvLog = findViewById(R.id.tv_ble_log);
        tvPower = findViewById(R.id.tv_power);
        tvThermometer = findViewById(R.id.tv_thermometer);

        Button btnScan = findViewById(R.id.btn_ble_scan);
        Button btnDisconnect = findViewById(R.id.btn_ble_disconnect);
        Button btnOn = findViewById(R.id.btn_ctrl_on);
        Button btnOff = findViewById(R.id.btn_ctrl_off);
        btnSetTimer = findViewById(R.id.btn_ctrl_timer);
        Button btnSetWiFi = findViewById(R.id.btn_set_wifi);
        Button btnLight = findViewById(R.id.btn_set_light);
        Button btnSoundOn = findViewById(R.id.btn_sound_on);
        Button btnSoundOff = findViewById(R.id.btn_sound_off);

        btnScan.setOnClickListener(this);
        btnDisconnect.setOnClickListener(this);
        btnOn.setOnClickListener(this);
        btnOff.setOnClickListener(this);
        btnSetTimer.setOnClickListener(this);
        btnSetWiFi.setOnClickListener(this);
        btnLight.setOnClickListener(this);
        btnSoundOn.setOnClickListener(this);
        btnSoundOff.setOnClickListener(this);

        sbTimer = findViewById(R.id.sb_run_timer);
        sbTimer.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
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
        });

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

}