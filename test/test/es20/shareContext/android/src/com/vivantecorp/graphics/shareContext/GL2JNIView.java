

package com.vivantecorp.graphics.shareContext;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceView;
import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGL;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;
import android.view.SurfaceHolder;
import android.app.Activity;

/**
 * An implementation of SurfaceView that uses the dedicated surface for
 * displaying an OpenGL animation.  This allows the animation to run in a
 * separate thread, without requiring that it be driven by the update mechanism
 * of the view hierarchy.
 *
 * The application-specific rendering code is delegated to a GLView.Renderer
 * instance.
 */
class GL2JNIView extends SurfaceView implements SurfaceHolder.Callback
{
	private SurfaceHolder   mHolder;
	private Context         context;

	private boolean         mHasSurface;
	private boolean         mDone;
    private boolean         first_run;
	private RenderThread    mthread;

	public GL2JNIView(Context context)
	{
		super(context);
		this.context=context;
        first_run = true;
		init();
	}

	public void surfaceCreated(SurfaceHolder holder)
	{
		mHasSurface = true;
	}

	public void surfaceDestroyed(SurfaceHolder holder)
	{
		mHasSurface = false;
		mDone = true;
		try
        {
			mthread.join();
		}
        catch (InterruptedException ex)
        {
			mthread.interrupt();
		}
	}

	public void surfaceChanged(SurfaceHolder holder,int format, int w, int h)
	{
	}

	public void start()
	{
		mthread = new RenderThread();
		mthread.start();
	}

	private void init()
	{
		mHolder = getHolder();
		mHolder.addCallback(this);
	}

	public void caseRun()
	{
        while(!mDone)
        {
            if(first_run == true)
            {
                GL2JNILib.init(mHolder.getSurface());
                first_run = false;
            }
            else
            {
                GL2JNILib.repaint();
            }
        }
	}

    public boolean needToWait()
    {
        return (!mHasSurface)&&(!mDone);
    }

    class RenderThread extends Thread
    {
        RenderThread()
        {
        }

        public void run()
        {
            try
            {
                guardedRun();
            }
            catch(InterruptedException e)
            {
                e.printStackTrace();
            }
        }
        public void guardedRun() throws InterruptedException
        {
            while (needToWait())
            {
            }

            if (!mDone)
            {
                caseRun();
            }

            ((Activity)(context)).finish();
        }
    }
}
