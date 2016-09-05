package com.android.jay.tbc.device;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.android.jay.tbc.ble.BtleListener;
import com.android.jay.tbc.ble.BtleManager;
import com.android.jay.tbc.device.work.WorkTask;


/**
 * Created by Administrator on 2016/5/26.
 */
public class DeviceManager implements BtleListener {

    private boolean isFake = false;
    private static final String TAG = "Tbc.DeviceManager";
    private static DeviceManager instance = null;

    static {
        instance = new DeviceManager();
    }

    private DeviceManager() {
    }

    public static DeviceManager getInstance() {
        return instance;
    }

    public static final int UI_MSG_DEVICE_CONNECTED = 0;
    public static final int UI_MSG_DEVICE_DISCONNECTED = 1;
    public static final int UI_MSG_DEVICE_SCAN = 2;
    public static final int UI_MSG_DEVICE_DATA = 3;

    private Context mContext;
    private Handler mUiHandler = null;
    private WorkTask mWorkThread;
    private DeviceWork mWork;
    private final int BUF_SIZE = 1024;
    private byte[] buffer;
    private int position;
    private int last_flag_pos;
    private int flag_pos;
    private boolean mIsConnected = false;

    private void initCmdList() {
    }

    @Override
    public void onDeviceConnected() {
        mUiHandler.obtainMessage(UI_MSG_DEVICE_CONNECTED).sendToTarget();
//        Toast.makeText(mContext, "Connected", Toast.LENGTH_SHORT).show();
        mIsConnected = true;
    }

    @Override
    public void onDeviceScan(String name, String addr) {
        Message msg = mUiHandler.obtainMessage();
        Bundle bundle = new Bundle();
        bundle.putString("name", name);
        bundle.putString("addr", addr);
        msg.what = UI_MSG_DEVICE_SCAN;
        msg.setData(bundle);
        mUiHandler.sendMessage(msg);
    }

    @Override
    public void onDeviceDisconnected() {
        mUiHandler.obtainMessage(UI_MSG_DEVICE_DISCONNECTED).sendToTarget();
        mIsConnected = false;
    }

    @Override
    public void onDataAvailable(byte[] data) {
        byte[] recvMsg;
        if (handlerBuffer(data)) {

            recvMsg = process();
            for (int i = 0; i < recvMsg.length; i++) {
                Log.v(TAG, String.format("[%d] = %02x\n", i, recvMsg[i]));
            }
        } else {
            return;
        }

        Message msg = mUiHandler.obtainMessage();
        Bundle bundle = new Bundle();
        bundle.putByteArray("MSG", recvMsg);
        msg.setData(bundle);
        msg.what = UI_MSG_DEVICE_DATA;
        //msg.arg1 = mEntryFlag;
        mUiHandler.sendMessage(msg);
        Log.v(TAG, "TASK DONE!");
    }

    private synchronized boolean handlerBuffer(byte[] data) {
        boolean retVal = false;
        boolean getFlag = false;
        for (int i = 0; i < data.length; i++) {
            buffer[position++] = data[i];
            if (position == BUF_SIZE) {
                position = 0;
            }
            //get '>>'
            if(i > 0) {
                if(data[i] == 0x3e && (data[i - 1] == 0x3e)) {
                    getFlag = true;
                }
            } else if (i == 0){
                if(data[0] == 0x3e && data[BUF_SIZE - 1] == 0x3e) {
                    getFlag = true;
                }
            }
            if (getFlag) {
                if (position != 0) {
                    flag_pos = position - 1;
                } else {
                    flag_pos = BUF_SIZE - 1;
                }
                //process data
                retVal = true;
                this.notify();
            }
        }

        return retVal;
    }

    private synchronized byte[] process() {
        int validBufLength;

        Log.v(TAG, "process: last_flag_pos = " + last_flag_pos + ", flag_pos = " + flag_pos);

        if (flag_pos > last_flag_pos) {
            validBufLength = flag_pos - last_flag_pos;
        } else {
            validBufLength = BUF_SIZE - last_flag_pos + flag_pos;
        }
        byte[] validBuf = new byte[validBufLength];
        if (flag_pos > last_flag_pos) {
            for (int i = last_flag_pos; i < flag_pos; i++) {
                validBuf[i - last_flag_pos] = buffer[i];
            }
        } else {
            for (int i = last_flag_pos; i < BUF_SIZE; i++) {
                validBuf[i - last_flag_pos] = buffer[i];
            }
            for (int i = 0; i < flag_pos; i++) {
                validBuf[BUF_SIZE - last_flag_pos + i] = buffer[i];
            }
        }
        flag_pos += 1;
        if (flag_pos == BUF_SIZE) {
            flag_pos = 0;
        }
        last_flag_pos = flag_pos;
        return validBuf;
//        String validString = new String(validBuf);
//        Log.v(TAG, "VALID BUF = " + validString);
//
//        return validString.split("\n");
    }

    public void init(Context context, Handler handler) {
        BtleManager.getInstance().init(context);
        BtleManager.getInstance().register(this);
        mContext = context;
        mUiHandler = handler;
        initCmdList();
        buffer = new byte[BUF_SIZE];
        position = 0;
        last_flag_pos = 0;
        flag_pos = 0;
        mIsConnected = false;
    }

    public synchronized void release() {
        BtleManager.getInstance().unregister();
        BtleManager.getInstance().release();
        mIsConnected = false;
        this.notifyAll();
    }

    public void scan() {
        BtleManager.getInstance().scan(true);
    }

}
