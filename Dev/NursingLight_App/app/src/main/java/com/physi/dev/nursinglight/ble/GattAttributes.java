/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.physi.dev.nursinglight.ble;

import java.util.HashMap;

/**
 * This class includes a small subset of standard GATT attributes for demonstration purposes.
 */
public class GattAttributes {
    private static HashMap<String, String> attributes = new HashMap();
    public static final String HEART_RATE_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
    public static final String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";

    public static final String NRF5_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
    public static final String NRF5_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
    public static final String NRF5_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
    public static final String NRF5_DFU = "00001530-1212-EFDE-1523-785FEABCD123";

    public static final String HM_10_CONF = "0000ffe0-0000-1000-8000-00805f9b34fb";
    public static final String HM_RX_TX = "0000ffe1-0000-1000-8000-00805f9b34fb";

    public static final String ESP32_SERVICE = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    public static final String ESP32_RX_TX = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

    public static final String CUSTOM_SERVICE = "000000ff-0000-1000-8000-00805f9b34fb";
    public static final String CUSTOM_RX_TX = "0000FF01-0000-1000-8000-00805f9b34fb";

    static {
        // Sample Services.
        attributes.put("0000180d-0000-1000-8000-00805f9b34fb", "Heart Rate Service");
        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");
        // Sample Characteristics.
        attributes.put(HEART_RATE_MEASUREMENT, "Heart Rate Measurement");
        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");
    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }
}
