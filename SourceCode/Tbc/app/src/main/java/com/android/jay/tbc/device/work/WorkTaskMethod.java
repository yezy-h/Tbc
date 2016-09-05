package com.android.jay.tbc.device.work;

/**
 * Created by H151136 on 5/27/2016.
 */
public interface WorkTaskMethod {
    void setup() throws InterruptedException;
    void process() throws InterruptedException;
    void cleanup() throws InterruptedException;
}
