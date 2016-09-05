package com.android.jay.tbc.device;

import android.util.Log;

import com.android.jay.tbc.device.work.BaseWork;

import java.util.HashMap;
import java.util.List;

/**
 * Created by H151136 on 5/27/2016.
 */
public class DeviceWork extends BaseWork {

    private static final String TAG = "Tbc.Device";
    private List<HashMap<String, Cmd>> mCmdList;

    public void setCmdList(List<HashMap<String, Cmd>> cmdList) {
        mCmdList = cmdList;
    }
    private boolean running = true;

    @Override
    public void setup() throws InterruptedException {
        super.setup();
        Log.v(TAG, "setup");
//        DeviceManager.getInstance().setLoopThreadPause();
        running = true;
    }

    public void setStop() {
        running = false;
    }

    @Override
    public void process() throws InterruptedException {
        super.process();
        Log.v(TAG, "process");
        int count = mCmdList.size();
        for(int i = 0; i < count; i++) {
            if(running) {
//                DeviceManager.getInstance().sendCmd(mCmdList.get(i).get("cmd"));
            }
        }
    }

    @Override
    public void cleanup() throws InterruptedException {
        super.cleanup();
        Log.v(TAG, "cleanup");
//        DeviceManager.getInstance().setLoopThreadRestart();
    }
}
