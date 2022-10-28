package com.youngtr.jnievner;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class DLActivity extends AppCompatActivity {

    // Used to load the 'jnievner' library on application startup.
    static {
        System.loadLibrary("jnievner");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dlactivity);
        findViewById(R.id.dl_open).setOnClickListener(v -> {
            nativeDLOpen();
        });

    }


    private native void nativeDLOpen();


    public static void start(Context context) {
        context.startActivity(new Intent(context, DLActivity.class));
    }
}