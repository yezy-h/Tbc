package com.android.jay.tbc.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.android.jay.tbc.R;

/**
 * Created by H151136 on 5/24/2016.
 */
public class FileExportDialog extends DialogFragment {

    public static final int FILE_TYPE_TXT = 1;
    public static final int FILE_TYPE_CVS = 2;

    private EditText mEditText;
    private SettingInputListern mListener;
    private TextView mTextView;
    private String mTitle;
    private String mText;
    private AlertDialog.Builder builder;
    private RadioGroup mRgFileType;
    private RadioButton mRbTxt;
    private RadioButton mRbCvs;

    public interface SettingInputListern {
        void onSettingInputComplete(String name);
        void onFileTypeSelect(int type);
    }

    public void init(String title, String text, SettingInputListern listener) {
        mTitle = title;
        mText = text;
        mListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        builder = new AlertDialog.Builder(getActivity());
        LayoutInflater inflater = getActivity().getLayoutInflater();
        View view = inflater.inflate(R.layout.dialog_export_file, null);
        mEditText = (EditText) view.findViewById(R.id.dialog_et);
        mTextView = (TextView) view.findViewById(R.id.dialog_tv);
        mTextView.setText(mText);

        mRgFileType = (RadioGroup)view.findViewById(R.id.rg_file_type);
        mRbTxt = (RadioButton)view.findViewById(R.id.rb_txt);
        mRbCvs = (RadioButton)view.findViewById(R.id.rb_cvs);
        mRbTxt.setChecked(true);
        mRgFileType.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(checkedId == R.id.rb_txt) {
                    mListener.onFileTypeSelect(FILE_TYPE_TXT);
                } else if(checkedId == R.id.rb_cvs) {
                    mListener.onFileTypeSelect(FILE_TYPE_CVS);
                }
            }
        });

        builder.setView(view).setPositiveButton(getString(R.string.ok_string),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (mListener != null) {
                            mListener.onSettingInputComplete(mEditText.getEditableText().toString());
                        }
                    }
                }).setNegativeButton(getString(R.string.cancel_string), null)
                .setTitle(mTitle).setIcon(R.mipmap.ic_launcher);

        return builder.create();
    }
}
