package com.alexbatalov.fallout2ce;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Enumeration;

public class MainActivity extends Activity {
    int selected;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Activity tmp = this;

        final File externalFilesDir = getExternalFilesDir(null);
        File[] files = externalFilesDir.listFiles(new Filter());

        String[] single_list = new String[files.length + 1];
        single_list[0] = "选择一个MOD";

        for (int i = 0; i < files.length; i++) {
            single_list[i + 1] = files[i].getName();
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("选择要启动的MOD");
        builder.setIcon(R.mipmap.ic_launcher);
        builder.setCancelable(false);
        builder.setSingleChoiceItems(single_list, 0, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                selected = which;
                if (which != 0)
                    Run.mod = files[which-1].getName();
                else
                    Run.mod = null;
            }
        });
        builder.setPositiveButton("启动", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                if (selected == 0) {
                    System.exit(0);
                } else {
                    Intent intent = new Intent(tmp, Run.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_TASK_ON_HOME | Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                    startActivity(intent);
                    finish();
                }
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }

    @Override
    public void onBackPressed() {

    }
}
