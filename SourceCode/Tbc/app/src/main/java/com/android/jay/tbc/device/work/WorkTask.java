package com.android.jay.tbc.device.work;

import android.os.AsyncTask;
import android.util.Log;

/**
 * Created by H151136 on 5/27/2016.
 */
public class WorkTask extends AsyncTask<WorkTaskMethod, Integer, Integer> {
    private static final String TAG = "Onlab.WorkTask";

    @Override
    protected Integer doInBackground(WorkTaskMethod... params) {
        if(params[0] != null) {
            try {
                params[0].setup();
                params[0].process();
                params[0].cleanup();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        Log.v(TAG, "Job done!");
        return null;
    }

    @Override
    protected void onProgressUpdate(Integer... values) {
        super.onProgressUpdate(values);
    }

    @Override
    protected void onPostExecute(Integer integer) {
        super.onPostExecute(integer);
    }
}
