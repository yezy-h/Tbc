package com.android.jay.tbc.ble;

/**
 * Created by Administrator on 2016/5/26.
 */
public interface BtleListener {
    void onDeviceScan(String name, String addr);
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onDataAvailable(byte[] data);
}
