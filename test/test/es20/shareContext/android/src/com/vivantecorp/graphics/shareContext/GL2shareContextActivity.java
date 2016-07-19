package com.vivantecorp.graphics.shareContext;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.KeyEvent;
import android.view.Window;
import android.view.Display;
import android.view.ViewGroup;
import android.widget.AbsoluteLayout;
import java.io.File;

public class GL2shareContextActivity extends Activity implements Runnable
{
     private volatile boolean done = false;

    private ViewGroup layout;
    private GL2JNIView[] views;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        Display display = getWindowManager().getDefaultDisplay();
        int width  = display.getWidth();
        int height = display.getHeight();
        AbsoluteLayout.LayoutParams params;
        params = new AbsoluteLayout.LayoutParams(
                     width, height, 0, 0);

        layout = new AbsoluteLayout(this);
        views  = new GL2JNIView[2];
        views[0] = new GL2JNIView(this);
        views[1] = new GL2JNIView(this);
        layout.addView(views[0], params);
        layout.addView(views[1], params);

        setContentView(layout);
    }

    private Thread updater;

    protected void onResume()
    {
    	super.onResume();

        updater = new Thread(this);
        updater.start();
    }

    protected void onPause()
    {
    	super.onPause();
        done = true;
        if (updater != null) updater.interrupt();
    }

    // Thread routine.
    public void run() {
        // Wait for surfaces creation.
        int layerCount = 2;
        for (int i = 0; i < layerCount; i++) {


            while (!views[i].isSurfaceCreated()) {
                try {
                    Thread.sleep(200);
                }
                catch (InterruptedException e) {
                    System.err.println(e);
                    return;
                }
            }
        }

        GL2JNILib.init(views[0].getSurface(), views[1].getSurface());

        // Main loop.
        while (!done) {
            GL2JNILib.repaint();
        }

        GL2JNILib.finish();
    }
}
