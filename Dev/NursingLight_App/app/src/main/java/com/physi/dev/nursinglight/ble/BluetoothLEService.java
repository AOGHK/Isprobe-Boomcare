package com.physi.dev.nursinglight.ble;

import android.annotation.SuppressLint;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.util.Arrays;
import java.util.List;
import java.util.UUID;

@SuppressLint("MissingPermission")
public class BluetoothLEService extends Service {

    private static final String TAG = BluetoothLEService.class.getSimpleName();

    /*
            Service Bind
     */
    private final IBinder binder = new ServiceBinder();

    @Override
    public IBinder onBind(Intent intent) {
        Log.e(TAG, "# Bluetooth LE Service Bind..");
        return binder;
    }

    class ServiceBinder extends Binder {
        BluetoothLEService getBLEService(){
            return BluetoothLEService.this;
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.e(TAG, "# Bluetooth LE Service UnBind..");
        return super.onUnbind(intent);
    }


    /*
               Ble Service
     */
    public final static String ACTION_GATT_CONNECTED = "com.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED = "com.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED = "com.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE = "com.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String ACTION_CHARACTERISTIC_READ = "com.bluetooth.le.ACTION_CHARACTERISTIC_READ";

    public final static String EXTRA_KEY_CHANGE_VALUE = "changeValue";

    public final static UUID UUID_HEART_RATE_MEASUREMENT =
            UUID.fromString(GattAttributes.HEART_RATE_MEASUREMENT);

    private BluetoothManager bluetoothManager = null;
    private BluetoothAdapter bluetoothAdapter = null;
    private BluetoothGatt bluetoothGatt = null;
    private BluetoothGattCharacteristic gattCharacteristic = null;
    private BluetoothGattCharacteristic notifyCharacteristic;
    private BluetoothGattCharacteristic writeCharacteristic;


    public boolean initialize() {
        // For API level 18 and above, get a reference to BluetoothAdapter through
        if (bluetoothManager == null) {
            bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (bluetoothManager == null) {
                Log.e(TAG, "# Unable to initialize BluetoothManager..");
                return false;
            }
        }
        bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null) {
            Log.e(TAG, "# Unable to obtain a BluetoothAdapter..");
            return false;
        }
        return true;
    }

    public static IntentFilter gattIntentFilter() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_GATT_CONNECTED);
        intentFilter.addAction(ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(ACTION_DATA_AVAILABLE);
        intentFilter.addAction(ACTION_CHARACTERISTIC_READ);
        return intentFilter;
    }

    public void connectGatt(BluetoothDevice device) {
        if(bluetoothManager == null && bluetoothAdapter == null){
            Log.e(TAG, "# Unable to initialize BluetoothLEService..");
            return;
        }
        bluetoothGatt = device.connectGatt(getApplicationContext(), true, gattCallback);
    }

    public void disconnectGatt() {
        if (bluetoothGatt == null) {
            return;
        }
        Log.e(TAG, "# Disconnected Bluetooth Gatt..");
        bluetoothGatt.disconnect();
        bluetoothGatt.close();
        bluetoothGatt = null;
        sendBroadcast(new Intent(ACTION_GATT_DISCONNECTED));
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            //super.onConnectionStateChange(gatt, status, newState);
            if(newState == BluetoothProfile.STATE_CONNECTED){
                Log.e(TAG, "# Connected to Bluetooth Gatt Server..");
                Log.e(TAG, "# Attempting to start service discovery : " + bluetoothGatt.discoverServices());
                sendBroadcast(new Intent(ACTION_GATT_CONNECTED));
            }else if (newState == BluetoothProfile.STATE_DISCONNECTED){
                Log.e(TAG, "# Disconnected from Gatt Server..");
                bluetoothGatt.close();
                bluetoothGatt = null;
                sendBroadcast(new Intent(ACTION_GATT_DISCONNECTED));
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            //super.onServicesDiscovered(gatt, status);
            sendBroadcast(new Intent(ACTION_GATT_SERVICES_DISCOVERED));
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            //super.onCharacteristicRead(gatt, characteristic, status);
            sendBroadcast(new Intent(ACTION_CHARACTERISTIC_READ));
            Log.e(TAG,"# Read Characteristic.");
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            //super.onCharacteristicChanged(gatt, characteristic);
            byte[] data = characteristic.getValue();
            if (data != null && data.length > 0) {
//                String value =  new String(data);
                sendBroadcast(new Intent(ACTION_DATA_AVAILABLE)
                        .putExtra(EXTRA_KEY_CHANGE_VALUE, data));
            }
        }
    };

    private void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (bluetoothAdapter == null || bluetoothGatt == null) {
            Log.e(TAG, "# BluetoothAdapter not initialized..");
            return;
        }
        boolean result = bluetoothGatt.readCharacteristic(characteristic);
        Log.e(TAG, "# readCharacteristic : " + result);
    }


    private void setCharacteristicNotification(BluetoothGattCharacteristic characteristic,
                                               boolean enabled) {
        Log.e(TAG, "setCharacteristicNotification :" + enabled);
        if (bluetoothAdapter == null || bluetoothGatt == null) {
            Log.e(TAG, "# BluetoothAdapter not initialized..");
            return;
        }
        boolean result = bluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        Log.e(TAG, "# setCharacteristicNotification : " + result);

        // This is specific to Heart Rate Measurement.
        if (UUID_HEART_RATE_MEASUREMENT.equals(characteristic.getUuid())) {
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                    UUID.fromString(GattAttributes.CLIENT_CHARACTERISTIC_CONFIG));
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            bluetoothGatt.writeDescriptor(descriptor);
        }

        if(enabled){
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                    UUID.fromString(GattAttributes.CLIENT_CHARACTERISTIC_CONFIG));
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            bluetoothGatt.writeDescriptor(descriptor);
        }
    }

    public boolean setNotifyCharacteristic(String serviceUUID, String charUUID){
        try{
            gattCharacteristic = bluetoothGatt.getService(UUID.fromString(serviceUUID))
                    .getCharacteristic(UUID.fromString(charUUID));
            final int charaProp = gattCharacteristic.getProperties();
            if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
                if (notifyCharacteristic != null) {
                    setCharacteristicNotification(notifyCharacteristic, false);
                    notifyCharacteristic = null;
                }
//                readCharacteristic(gattCharacteristic);
            }
            if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
                notifyCharacteristic = gattCharacteristic;
                setCharacteristicNotification(gattCharacteristic, true);
            }
            return true;
        }catch (Exception e){
            e.getStackTrace();
            Log.e(TAG, "# Notify Characteristic Err - " + e.getMessage());
            return false;
        }
    }


    public boolean writeCharacteristic(String serviceUUID, String charUUID, String msg) {
        BluetoothGattService RxService = bluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (RxService == null) {
            return false;
        }

        BluetoothGattCharacteristic RxChar = RxService.getCharacteristic(UUID.fromString(charUUID));
        if (RxChar == null) {
            return false;
        }

        RxChar.setValue(msg);
        boolean status = bluetoothGatt.writeCharacteristic(RxChar);
        Log.d(TAG, "# Write result : " + status);

        return status;
    }

    public boolean writeCharacteristic(String serviceUUID, String charUUID, byte[] cmd) {
        BluetoothGattService RxService = bluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (RxService == null) {
            return false;
        }

        BluetoothGattCharacteristic RxChar = RxService.getCharacteristic(UUID.fromString(charUUID));
        if (RxChar == null) {
            return false;
        }

        RxChar.setValue(cmd);
        boolean status = bluetoothGatt.writeCharacteristic(RxChar);
        Log.d(TAG, "# Write result : " + status);

        return status;
    }
}
