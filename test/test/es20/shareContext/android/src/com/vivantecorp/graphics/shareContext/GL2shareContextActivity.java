

package com.vivantecorp.graphics.shareContext;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.KeyEvent;
import android.view.Window;
import java.io.File;

public class GL2shareContextActivity extends Activity
{
    GL2JNIView mView;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        mView = new GL2JNIView(this);
        setContentView(mView);
        mView.start();
    }

    protected void onResume()
    {
    	super.onResume();
    }

    protected void onPause()
    {
    	super.onPause();
    }
}
