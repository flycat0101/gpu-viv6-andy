

package com.vivantecorp.graphics.tutorial3;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.KeyEvent;

import java.io.File;


public class GL2Tutorial3Activity extends Activity {

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
//		Log.i("tutorial3",String.format("key %d pressed!",key));
       if( key==4 )
        {
            finish();
            return true;
        }
       return GL2JNILib.key(key,evt.getAction() == KeyEvent.ACTION_DOWN);

    }
}
