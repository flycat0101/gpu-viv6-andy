

package com.vivantecorp.graphics.tutorial7;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.KeyEvent;

import java.io.File;


public class GL2Tutorial7Activity extends Activity {

    GL2JNIView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new GL2JNIView(getApplication());
	setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    public boolean onKeyDown(int key, KeyEvent evt) {
       if( key==4 )
        {
            finish();
            return true;
        }
       return GL2JNILib.key(key,evt.getAction() == KeyEvent.ACTION_DOWN);

    }
}
