package com.physi.dev.nursinglight.ble;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import java.util.List;

public class BluetoothLEManager {
    private static final String TAG = "BluetoothLEManager";

    public static final int BLE_SCAN_START = 101;
    public static final int BLE_SCAN_STOP = 102;
    public static final int BLE_SCAN_DEVICE = 103;
    public static final int BLE_CONNECT_DEVICE = 105;
    public static final int BLE_DISCONNECT_DEVICE = 106;
    public static final int BLE_SERVICES_DISCOVERED = 107;
    public static final int BLE_DATA_AVAILABLE = 108;
    public static final int BLE_READ_CHARACTERISTIC = 109;

    private static BluetoothLEManager bluetoothLEManager = null;
    private Context context;
    private Handler bleHandler = null;

    private BluetoothAdapter bluetoothAdapter = null;
    private BluetoothLeScanner bluetoothLeScanner = null;
    private BluetoothLEService bluetoothLEService = null;

    private long scanTime = 3000;
    private boolean isScanning = false;
    private boolean isBind = false;
    private boolean isRegister = false;
    private boolean isBleConnected = false;

    private BluetoothLEManager(Context context){
        this.context = context;
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
    }

    public synchronized static BluetoothLEManager getInstance(Context context){
        if(bluetoothLEManager == null)
            bluetoothLEManager = new BluetoothLEManager(context);
        return bluetoothLEManager;
    }

    //  Check Bluetooth LE Support
    public boolean checkBleStatus(Context context) {
        return bluetoothAdapter != null && context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
    }

    public boolean getEnable(){
        return bluetoothAdapter.isEnabled();
    }

    //  Setup Scan Time
    public void setScanTime(long time){
        scanTime = time;
    }

    //  Setup Bluetooth LE Handler
    public void setHandler(Handler handler){
        bleHandler = handler;
    }

    public boolean isConnected(){
        return isBleConnected;
    }

    //region    Bluetooth LE Scanner
    private void startBLEScan(){
        bluetoothLeScanner.startScan(scanCallback);
        isScanning = true;
        bleHandler.obtainMessage(BLE_SCAN_START).sendToTarget();
        Log.e(TAG, "# Start Bluetooth LE Scan..");
    }

    private void stopBLEScan(){
        bluetoothLeScanner.stopScan(scanCallback);
        isScanning = false;
        bleHandler.removeCallbacks(scanStopRunnable);
        bleHandler.obtainMessage(BLE_SCAN_STOP).sendToTarget();
        Log.e(TAG, "# Stop Bluetooth LE Scan..");
    }

    @SuppressLint("NewApi")
    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            bleScanResult(result.getDevice(), result.getRssi(), result.getScanRecord().getBytes());
        }
        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult result : results){
                Log.e(TAG, "onBatchScanResults : " + result.getDevice().getAddress());
            }
        }
        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "# BLE Scan Error..[ErrorCode] : " + errorCode);
        }
    };

    private void bleScanResult(BluetoothDevice bluetoothDevice, int rssi, byte[] scanRecord){
        if(bluetoothDevice != null && scanRecord != null){
            Log.e(TAG, "# Bluetooth LE Device : " + bluetoothDevice.getName() + "-" + bluetoothDevice.getAddress());
            bleHandler.obtainMessage(BLE_SCAN_DEVICE, bluetoothDevice).sendToTarget();
        }
    }

    public void scan(boolean isScan, boolean isTimer){
        if(isScan){
            if(isScanning)
                return;
            startBLEScan();
            if(isTimer)
                bleHandler.postDelayed(scanStopRunnable, scanTime);
        }else{
            if(isScanning) stopBLEScan();
        }
    }

    private final Runnable scanStopRunnable = new Runnable() {
        @Override
        public void run() {
            stopBLEScan();
        }
    };
    //endregion


    //region    Bluetooth LE Service
    public void bindService(){
        if(isBind)
            return;
        Intent gattServiceIntent = new Intent(context.getApplicationContext(), BluetoothLEService.class);
        context.getApplicationContext().bindService(gattServiceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    public void unBindService(){
        if(!isBind)
            return;
        try{
            isBind = false;
            context.getApplicationContext().unbindService(serviceConnection);
        }catch (Exception e){
            e.getStackTrace();
        }
    }

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            Log.e(TAG, "# Bluetooth Service Connected..");
            bluetoothLEService = ((BluetoothLEService.ServiceBinder) iBinder).getBLEService();
            bluetoothLEService.initialize();
            isBind = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "# Bluetooth Service DisConnected..");
            bluetoothLEService = null;
            isBind = false;
        }
    };

    public void connect(BluetoothDevice device){
        if(!isBind || bluetoothLEService == null)
            return;
        bluetoothLEService.connectGatt(device);
    }

    public void disconnect(){
        if(!isBind || bluetoothLEService == null)
            return;
        bluetoothLEService.disconnectGatt();
    }

    public boolean notifyCharacteristic(String serviceUUID, String charUUID){
        return bluetoothLEService.setNotifyCharacteristic(serviceUUID, charUUID);
    }

    public boolean writeCharacteristic(String serviceUUID, String charUUID, String writeData){
        if(!isBleConnected)
            return false;
        boolean result = bluetoothLEService.writeCharacteristic(serviceUUID, charUUID, writeData);
        Log.e(TAG, "> Write Message : " + writeData + " (" + result + ")");
        return result;
    }

    public boolean writeCharacteristic(String serviceUUID, String charUUID, short[] data){
        if(!isBleConnected)
            return false;
        StringBuilder cmd = new StringBuilder();
        for(int i = 0; i < data.length; i++){
            cmd.append((char) data[i]);
        }
        return bluetoothLEService.writeCharacteristic(serviceUUID, charUUID, cmd.toString());
    }


    //endregion

    //region Service Broadcast Receiver
    public void registerReceiver(){
        if(isRegister)
            return;
        context.registerReceiver(gattReceiver, BluetoothLEService.gattIntentFilter());
        isRegister = true;
        Log.e(TAG, "# Start Register Receiver..");
    }

    public void unregisterReceiver(){
        if(!isRegister)
            return;
        context.unregisterReceiver(gattReceiver);
        isRegister = false;
        Log.e(TAG, "# Stop Register Receiver..");
    }

    private final BroadcastReceiver gattReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.e(TAG, action);
            assert action != null;
            switch (action) {
                case BluetoothLEService.ACTION_GATT_CONNECTED:
                    isBleConnected = true;
                    bleHandler.obtainMessage(BLE_CONNECT_DEVICE).sendToTarget();
                    break;
                case BluetoothLEService.ACTION_GATT_SERVICES_DISCOVERED:
                    bleHandler.obtainMessage(BLE_SERVICES_DISCOVERED).sendToTarget();
                    break;
                case BluetoothLEService.ACTION_CHARACTERISTIC_READ:
                    bleHandler.obtainMessage(BLE_READ_CHARACTERISTIC).sendToTarget();
                    break;
                case BluetoothLEService.ACTION_DATA_AVAILABLE:
                    bleHandler.obtainMessage(BLE_DATA_AVAILABLE,
                            intent.getByteArrayExtra(BluetoothLEService.EXTRA_KEY_CHANGE_VALUE)).sendToTarget();
                    break;
                case BluetoothLEService.ACTION_GATT_DISCONNECTED:
                    isBleConnected = false;
                    bleHandler.obtainMessage(BLE_DISCONNECT_DEVICE).sendToTarget();
                    break;
            }
        }
    };
    //endregion



}
