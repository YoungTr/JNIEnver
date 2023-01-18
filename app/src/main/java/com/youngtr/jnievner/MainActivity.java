package com.youngtr.jnievner;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.youngtr.jnievner.databinding.ActivityMainBinding;

import java.io.File;
import java.util.Arrays;

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

        tv.setOnClickListener(view -> {
            DLActivity.start(this);
        });

        visitMethod();

        Person person = getPerson();
        Log.d("JNIEnver", person.toString());

        Person[] persons = getPersons(new String[]{"James", "Bob", "Curry"});
        Log.d("JNIEnver", Arrays.toString(persons));

        dynamicRegister("Tom");

        initFile(getFilesDir().getAbsolutePath() + File.separator + "test.log");


        compress("123456789abcdefghijklmnopqrstuvwxyz123456789abcdefghijklmnopqrstuvwxyz123456789abcdefghijklmnopqrstuvwxyz");

        zlib(getFilesDir().getAbsolutePath() + File.separator + "test.log",
                getFilesDir().getAbsolutePath() + File.separator + "test1.log");

//        String stream = getStream();
//        Log.d("JNIEnver", "get stream: " + stream);

        binding.crashText.setOnClickListener(v -> {
            testCrash();
        });
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

    public native Person getPerson();

    public native Person[] getPersons(String[] names);

    public native void dynamicRegister(String name);

    public native void initFile(String path);

    public native void compress(String source);

    public native String getStream();

    public native void zlib(String source, String dest);

    public native void testCrash();
}