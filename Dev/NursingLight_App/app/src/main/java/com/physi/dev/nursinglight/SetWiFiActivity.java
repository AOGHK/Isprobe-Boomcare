package com.physi.dev.nursinglight;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.physi.dev.nursinglight.ble.BluetoothLEManager;
import com.physi.dev.nursinglight.ble.GattAttributes;
import com.physi.dev.nursinglight.list.BleAdapter;
import com.physi.dev.nursinglight.list.WiFiItemAdapter;

import java.util.List;

public class SetWiFiActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = SetWiFiActivity.class.getSimpleName();
    private EditText etPwd;
    private Button btnScan;

    private WifiManager wManager;
    private BluetoothLEManager bleManager;
    private WiFiItemAdapter  wifiItemAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_set_wifi);

        init();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bleManager.setHandler(handler);
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.btn_wifi_scan){
            wManager.startScan();
            btnScan.setEnabled(false);
        }else if(v.getId() == R.id.btn_set_wifi){
            String ssid = wifiItemAdapter.getSelectedSSID();
            String pwd;
            if(ssid == null)
                return;

            if(etPwd.length() != 0){
                pwd = etPwd.getText().toString();
            } else {
                pwd = "";
            }
            bleManager.writeCharacteristic(GattAttributes.ESP32_SERVICE, GattAttributes.ESP32_RX_TX,
                    "$28" + ssid + "," + pwd + "#");
        }
    }

    private final Handler handler = new Handler(Looper.getMainLooper()){
        @SuppressLint({"MissingPermission", "SetTextI18n"})
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case BluetoothLEManager.BLE_DATA_AVAILABLE:
                    String res = (String)msg.obj;
                    receiveMessage(res);
                    break;
                case BluetoothLEManager.BLE_DISCONNECT_DEVICE:
                    finish();
                    break;
            }

        }
    };

    private void receiveMessage(String str){
        if(str.startsWith("$28") && str.endsWith("#")){
            Toast.makeText(getApplicationContext(),
                    "Connect Result : " + str.charAt(3), Toast.LENGTH_SHORT).show();
        }
    }

    @SuppressLint("UseCompatLoadingForDrawables")
    private void init(){
        bleManager = BluetoothLEManager.getInstance(getApplicationContext());

        wManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                List<ScanResult> scanResults = wManager.getScanResults();
                wifiItemAdapter.setItem(scanResults);
                btnScan.setEnabled(true);
            }
        }, intentFilter);

        btnScan = findViewById(R.id.btn_wifi_scan);
        Button btnSetup = findViewById(R.id.btn_set_wifi);

        btnScan.setOnClickListener(this);
        btnSetup.setOnClickListener(this);

        etPwd = findViewById(R.id.et_wifi_pwd);

        RecyclerView rcvWiFiList = findViewById(R.id.rcv_wifi_list);
        DividerItemDecoration decoration
                = new DividerItemDecoration(getApplicationContext(), LinearLayoutManager.VERTICAL);
        decoration.setDrawable(getResources().getDrawable(R.drawable.item_division_line, null));
        rcvWiFiList.addItemDecoration(decoration);
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(SetWiFiActivity.this);
        linearLayoutManager.setOrientation(LinearLayoutManager.VERTICAL);
        linearLayoutManager.setItemPrefetchEnabled(true);
        rcvWiFiList.setLayoutManager(linearLayoutManager);
        rcvWiFiList.setAdapter(wifiItemAdapter = new WiFiItemAdapter());
    }
}