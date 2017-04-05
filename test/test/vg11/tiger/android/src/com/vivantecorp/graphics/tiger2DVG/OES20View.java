package com.vivantecorp.graphics.tiger2DVG;

import android.app.Activity;
import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.Log;


/**view definition */
public class OES20View extends SurfaceView implements SurfaceHolder.Callback{
	private final int     MAX_CASE_NUM = 30;
	private boolean[]     caseValues   = new boolean[MAX_CASE_NUM];
	private SurfaceHolder mHolder;
	private Context       context;
	private int mWidth, mHeight, mConfigWidth, mConfigHeight;
	private boolean bchanged = false;

	/**construct function
	 * @throws InterruptedException */
	public OES20View(Context context, boolean[] cases) {
		super(context);
		this.context = context;
		this.caseValues = cases;
		init();
	}

	public void surfaceCreated(SurfaceHolder holder){
		mHasSurface = true;
		mWidth = getWidth();
		mHeight = getHeight();
		if(mWidth < mConfigWidth){
			mWidth = mConfigWidth;
			bchanged = true;
		}
		if(mHeight < mConfigHeight){
			mHeight = mConfigHeight;
			bchanged = true;
		}
		if(true == bchanged){
			mHolder.setFixedSize(mWidth, mHeight);
		}
	}

	public void surfaceDestroyed(SurfaceHolder holder){
		mHasSurface = false;
		mDone = true;
		try{
			mthread.join();
		}catch (InterruptedException ex){
			mthread.interrupt();
		}
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h){
	}

	/*new render thread and start the thread*/
	public void start(){
		mthread = new RenderThread();
		mthread.start();
	}
	/**All cases runnable are all set to false in initialized step*/
	private void init(){
		mHolder = getHolder();
		mHolder.addCallback(this);
		GL2JNILib.init();
		mConfigWidth = GL2JNILib.getWidth();
		mConfigHeight = GL2JNILib.getHeight();
	}
	/** Because of run_all task, all cases runnable are all set to true,then start to run*/
	public void caseRun(){
		GL2JNILib.nativeRunTest(mHolder.getSurface(), caseValues);
	}

	public boolean needToWait(){
		return (!mHasSurface)&&(!mDone);
	}
	/*create a work thread for rendering*/
	class RenderThread extends Thread{
		RenderThread(){};
		@Override
		public void run(){
			try{
				guardedRun();
			}catch(InterruptedException e){
				e.printStackTrace();
			}

		}

		public void guardedRun() throws InterruptedException{
			    while(needToWait()){
			    }
			if(!mDone){
				caseRun();
				mDone = true;
			}

			((Activity)(context)).finish();
		}

	}
	private volatile boolean mHasSurface;
	private volatile boolean mDone;
	private RenderThread mthread;
}
