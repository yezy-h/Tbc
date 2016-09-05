package com.android.jay.tbc;

/**
 * Created by Administrator on 2016/8/28.
 */
public class Utils {
    public static int[] barr2iarr(byte[] barr) {
        int len = barr.length;
        int[] iarr = new int[len];
        for (int i = 0; i < len; i++) {
            if (barr[i] > -1) {
                iarr[i] = barr[i];
            } else {
                iarr[i] = barr[i] + 256;
            }
        }
        return iarr;
    }
}
