package com.android.jay.tbc;

import android.Manifest;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.graphics.ColorUtils;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.jay.tbc.ble.BtleManager;
import com.android.jay.tbc.device.DeviceManager;
import com.android.jay.tbc.dialog.DevicesSelectDialog;
import com.android.jay.tbc.dialog.FileExportDialog;
import com.android.jay.tbc.otto.BusProvider;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import lecho.lib.hellocharts.model.Axis;
import lecho.lib.hellocharts.model.Line;
import lecho.lib.hellocharts.model.LineChartData;
import lecho.lib.hellocharts.model.PointValue;
import lecho.lib.hellocharts.model.Viewport;
import lecho.lib.hellocharts.util.ChartUtils;
import lecho.lib.hellocharts.view.LineChartView;
import pub.devrel.easypermissions.AfterPermissionGranted;
import pub.devrel.easypermissions.EasyPermissions;

public class MainActivity extends AppCompatActivity {
    private static final boolean isFake = false;
    private static final String TAG = "Tbc";
    private static boolean isExit = false;
    private Toast mToast;
    private DeviceManager mDeviceManager;
    private BluetoothAdapter mBluetoothAdapter;
    private ProgressDialog mWaitDialog;
    private DevicesSelectDialog mDeviceSelectDialog;
    private boolean mIsBluetoothConnected = false;
    private Handler mHandler = new Handler();
    private Button mConnectButton;
    private Button mResetButton;
    private Button mSaveButton;
    private TextView mTemp1;
    private TextView mTemp2;
    private TextView mOutput;

    private List<HashMap<String, String>> mData;
    private FileExportDialog mFileExportDialog;

    private LineChartData mChartData;
    private LineChartView mChartView;
    private List<Line> mLines;
    private Line mLine1;
    private Line mLine2;
    private List<PointValue> mPoints1;
    private List<PointValue> mPoints2;

    private int mTime = 0;

    private Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case DeviceManager.UI_MSG_DEVICE_CONNECTED:
                    mIsBluetoothConnected = true;
//                    setBluetoothConnected(true);
                    mDeviceSelectDialog.dismiss();
                    mWaitDialog.dismiss();

                    break;
                case DeviceManager.UI_MSG_DEVICE_DISCONNECTED:
                    mIsBluetoothConnected = false;
//                    setBluetoothConnected(false);
                    break;

                case DeviceManager.UI_MSG_DEVICE_SCAN:
                    Bundle bundle = msg.getData();
                    String name = bundle.getString("name");
                    String addr = bundle.getString("addr");
                    Log.d(TAG, "###name = " + name);
                    mDeviceSelectDialog.addDevice(name, addr);
                    break;
                case DeviceManager.UI_MSG_DEVICE_DATA:
                    Bundle data = msg.getData();
                    int entryFlag = msg.arg1;
                    byte[] recvMsg = data.getByteArray("MSG");
                    if(recvMsg[0] == 0x55 && recvMsg[1] == 0x55) {
                        updateTemp(recvMsg);
                    }
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            toastShow(getString(R.string.ble_not_support));
            finish();
        }
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        mWaitDialog = new ProgressDialog(this, ProgressDialog.STYLE_SPINNER);
        mWaitDialog.setCancelable(false);

        mDeviceSelectDialog = new DevicesSelectDialog();
        mDeviceSelectDialog.setDialog(mWaitDialog);
        mDeviceSelectDialog.setContext(this);
        mDeviceManager = DeviceManager.getInstance();
        mDeviceManager.init(this, mUiHandler);

        mData = new ArrayList<>();
        mFileExportDialog = new FileExportDialog();
        mFileExportDialog.init("save file", "Name", new FileExportDialog.SettingInputListern() {
            @Override
            public void onSettingInputComplete(String name) {
                if(name.length() < 1) {
                    Toast.makeText(MainActivity.this, "invalid name", Toast.LENGTH_SHORT).show();
                    return;
                }
                saveFile(name);
            }

            @Override
            public void onFileTypeSelect(int type) {

            }
        });
        checkPermissions();
        File file = new File(SAVE_FILE_PATH);
        if(!file.exists()) {
            Log.d(TAG, "make dir");
            file.mkdirs();
        }
    }

    private void initView() {
        mToast = Toast.makeText(MainActivity.this, "", Toast.LENGTH_SHORT);
        mConnectButton = (Button)findViewById(R.id.bt_connect);
        mConnectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mIsBluetoothConnected) {
                    mDeviceManager.scan();
                    mDeviceSelectDialog.show(getFragmentManager(), getString(R.string.select_devices));
                }
//                else {
//                    //alert dialog to disconnect current connection
//                    Utils.showAlertDialog(MainActivity.this, getString(R.string.notice), getString(R.string.sure_to_disconnect),
//                            new DialogInterface.OnClickListener() {
//                                @Override
//                                public void onClick(DialogInterface dialog, int which) {
//                                    BtleManager.getInstance().disconnect();
//                                }
//                            });
//                }
            }
        });
        mResetButton = (Button)findViewById(R.id.bt_reset);
        mResetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mData.clear();
                mPoints1.clear();
                mPoints2.clear();
                mTime = 0;
                if(isFake) {
                    for(int i = 0; i < 300; i++) {
                        Item it = new Item();
                        it.time = i;
                        it.temp1 = (float)(Math.random() * 100.0f);
                        it.temp2 = (float)(Math.random() * 100.0f);
                        it.output = (int)(Math.random() * 100.0f);
                        addItem(it);

                        mPoints1.add(new PointValue(i, (float)(it.temp1)));
                        mPoints2.add(new PointValue(i, (float)(it.temp2)));
                        mChartData.setLines(mLines);
                        mChartView.setLineChartData(mChartData);
                    }
                }
            }
        });
        mSaveButton = (Button)findViewById(R.id.bt_save);
        mSaveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //save to file
                mFileExportDialog.show(getFragmentManager(), "save_file");
            }
        });
        mTemp1 = (TextView)findViewById(R.id.tv_temp1);
        mTemp2 = (TextView)findViewById(R.id.tv_temp2);
        mOutput = (TextView)findViewById(R.id.tv_output);
        //init chart
        mChartView = (LineChartView)findViewById(R.id.chart_main);
        mPoints1 = new ArrayList<>();
        mPoints2 = new ArrayList<>();

        mLines = new ArrayList<>();
        mLine1 = new Line(mPoints1).setColor(ChartUtils.COLOR_BLUE).setCubic(false);
        mLine2 = new Line(mPoints2).setColor(ChartUtils.COLOR_GREEN).setCubic(false);
        mLine1.setPointRadius(1);
        mLine1.setStrokeWidth(1);
        mLine2.setPointRadius(1);
        mLine2.setStrokeWidth(1);
        mLine1.setHasPoints(false);
        mLine2.setHasPoints(false);

        mLines.add(mLine1);
        mLines.add(mLine2);
        mChartData = new LineChartData();
        mChartData.setBaseValue(Float.NEGATIVE_INFINITY);
        mChartView.setLineChartData(mChartData);

        final Viewport viewport = new Viewport(mChartView.getCurrentViewport());
        viewport.left = 0;
        viewport.top = 100;
        viewport.right = 1000;
        viewport.bottom = -200;

        final Viewport viewportMax = new Viewport(mChartView.getCurrentViewport());
        viewportMax.left = 0;
        viewportMax.top = 100;
        viewportMax.right = 2000;
        viewportMax.bottom = -200;

        mChartView.setMaximumViewport(viewportMax);
        mChartView.setCurrentViewport(viewport);
        mChartView.setViewportCalculationEnabled(false);

        Axis axisX = new Axis();
        Axis axisY = new Axis();
        axisX.setName("Time");
        axisX.setHasSeparationLine(true);
        axisY.setName("Temperature");
        axisY.setHasSeparationLine(true);
        mChartData.setAxisXBottom(axisX);
        mChartData.setAxisYLeft(axisY);

    }

    private void addItem(Item item) {
        HashMap<String, String> it = new HashMap<>();

        it.put("time", "" + item.time);
        it.put("temp1", "" + item.temp1);
        it.put("temp2", "" + item.temp2);
        it.put("output", "" + item.output);
        mData.add(it);
    }

    private static String SAVE_FILE_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + "/TBC/";
    private void saveFile(String name) {
        File saveFile = new File(SAVE_FILE_PATH + name + ".txt");

        try {
            FileWriter out = new FileWriter(saveFile, false);
            BufferedWriter writer = new BufferedWriter(out);
            for(int i = 0; i < mData.size(); i++) {
                int time;
                float temp1;
//                float temp2;
                int output;

                HashMap<String, String> map = mData.get(i);
                time = Integer.parseInt(map.get("time"));
                temp1 = Float.parseFloat(map.get("temp1"));
                //temp2 = Float.parseFloat(map.get("temp2"));
                output = Integer.parseInt(map.get("output"));
                String line = String.format("%s\t%s\t%s\n",
                        time, temp1, output);
                writer.write(line);
            }
            writer.flush();
            writer.close();
            out.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void updateTemp(byte[] data) {
        int[] idata = Utils.barr2iarr(data);
        int time1;
        int temp1;
        int output;
        int temp2;

        time1 = (idata[4] << 8) | idata[5];
        temp1 = (idata[6] << 8) | idata[7];
        output = (idata[8] << 8) | idata[9];
        temp2 = (idata[10] << 8) | idata[11];

        temp1 = temp1 - 20000;
        temp2 = temp2 - 20000;
        //update to data
        Item it = new Item();
        it.time = mTime;
        it.temp1 = temp1;
        it.temp2 = temp2;
        it.output = output;
        addItem(it);

//        Log.d(TAG, "t1 = " + time1 + " T1 = " + temp1);
//        Log.d(TAG, "t2 = " + time2 + " T2 = " + temp2);
        mTemp1.setText("T1 = " + (float)(temp1) / 100);
        mTemp2.setText("T2 = " + (float)(temp2) / 100);
        mOutput.setText("OUTPUT = " + (float)(output) / 100);
        mPoints1.add(new PointValue(mTime, (float)(temp1) / 100));
        mPoints2.add(new PointValue(mTime++, (float)(temp2) / 100));
        mChartData.setLines(mLines);
        mChartView.setLineChartData(mChartData);
    }

    private static final int RC_ROOT = 102;

    @AfterPermissionGranted(RC_ROOT)
    public void checkPermissions() {
        String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.ACCESS_COARSE_LOCATION};
        boolean[] allows = new boolean[permissions.length];
        for(int i = 0; i < permissions.length; i++) {
            if(EasyPermissions.hasPermissions(this, permissions[i])) {
                allows[i] = true;
            } else {
                allows[i] = false;
            }
        }
        boolean result = true;
        for(int i = 0; i < permissions.length; i++) {
            result &= allows[i];
        }

        if(!result) {
            EasyPermissions.requestPermissions(this, "Root", RC_ROOT, permissions[0], permissions[1], permissions[2]);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        BusProvider.getInstance().register(this);
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivity(enableBtIntent);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        BusProvider.getInstance().unregister(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Main onDestroy!");
        mDeviceManager.release();
        mDeviceManager = null;
    }

    @Override
    public void onBackPressed() {

        if (!isExit) {
            isExit = true;
            toastShow(getString(R.string.retry_to_exit));
            new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    super.handleMessage(msg);
                    isExit = false;
                }
            }.sendEmptyMessageDelayed(0, 2000);
        } else {
            mDeviceManager.release();
            mDeviceManager = null;
            finish();
            System.exit(0);
        }
    }

    private void toastShow(String msg) {
        if (mToast != null) {
            mToast.setText(msg);
            mToast.show();
        }
    }
}
