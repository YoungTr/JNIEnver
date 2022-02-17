package com.youngtr.jnievner;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.youngtr.jnievner.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'jnievner' library on application startup.
    static {
        System.loadLibrary("jnievner");
    }

    private ActivityMainBinding binding;

    private String name = "Cat";

    private static String staticName = "Static Cat";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        visitField();
        tv.setText(tv.getText() + ":" + name + ":" + staticName);

        visitMethod();
    }

    /**
     * A native method that is implemented by the 'jnievner' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void visitField();

    public native void visitMethod();

    public static int add(int a, int b) {
        return a + b;
    }

    public int minus(int total) {
        return total - 100;
    }
}