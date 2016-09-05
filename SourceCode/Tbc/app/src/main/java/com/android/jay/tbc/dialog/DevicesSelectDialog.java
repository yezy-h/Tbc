package com.android.jay.tbc.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

import com.android.jay.tbc.R;
import com.android.jay.tbc.ble.BtleManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by H151136 on 5/24/2016.
 */
public class DevicesSelectDialog extends DialogFragment {

    private ListView mDeviceListView;
    private SimpleAdapter mAdapter;
    private List<HashMap<String, String>> mData;
    private ProgressDialog mDialog = null;
    private Context mContext;
    private final int CONNECT_TIMEOUT = 5000; //40s
    private Handler mHandler;


    private Runnable mCallback = new Runnable() {
        @Override
        public void run() {
            if (mDialog.isShowing()) {
                mDialog.dismiss();
                //timeout
                BtleManager.getInstance().disconnect();
                Toast.makeText(getActivity(), getString(R.string.connect_timeout), Toast.LENGTH_SHORT).show();
            }
        }
    };

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        LayoutInflater inflater = getActivity().getLayoutInflater();
        View view = inflater.inflate(R.layout.dialog_device_list, null);

        mHandler = new Handler();
        mData = new ArrayList<HashMap<String, String>>();

        mDeviceListView = (ListView) view.findViewById(R.id.dialog_lv_devices);
        mAdapter = new SimpleAdapter(getActivity(), mData, R.layout.item_select_device,
                new String[]{"name"}, new int[]{R.id.item_device});
        mDeviceListView.setAdapter(mAdapter);
        mDeviceListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String address = mData.get(position).get("addr");
                BtleManager.getInstance().connect(address);
//                Toast.makeText(getActivity(), getActivity().getString(R.string.attempt_connecting_device)
//                        , Toast.LENGTH_SHORT).show();
                if (mDialog != null && (!mDialog.isShowing())) {
                    mDialog.setMessage(getString(R.string.attempt_connecting_device));
                    mDialog.show();
                    mHandler.postDelayed(mCallback, CONNECT_TIMEOUT);
                    mDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            mHandler.removeCallbacks(mCallback);
                        }
                    });
                }
//                dismiss();
            }
        });

        clear();

        builder.setView(view).setNegativeButton(getString(R.string.cancel_string), null)
                .setTitle(getString(R.string.select_devices)).setIcon(R.mipmap.ic_launcher);

        return builder.create();
    }

    public void setContext(Context context) {
        mContext = context;
    }

    public void setDialog(ProgressDialog dialog) {
        mDialog = dialog;
    }

    private void clear() {
        mData.clear();
        mAdapter.notifyDataSetChanged();
    }

    public void addDevice(String name, String addr) {
        if (mData == null)
            return;
        for (int i = 0; i < mData.size(); i++) {
            if (mData.get(i).get("addr").equals(addr))
                return;
        }

        HashMap<String, String> item = new HashMap<String, String>();
        item.put("name", "" + name + "(" + addr + ")");
        item.put("addr", addr);

        mData.add(item);
        mAdapter.notifyDataSetChanged();
    }
}
