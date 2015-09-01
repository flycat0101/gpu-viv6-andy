package com.vivantecorp.graphics;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

/**
 * single activity class, run single native application.
 */
public class NativeActivity extends Activity {

	private GLNativeView glView;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		this.requestWindowFeature(Window.FEATURE_NO_TITLE);

		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		// Create our Preview view and set it as the content of our
		// Activity
		Intent intent = getIntent();

		glView = new GLNativeView(
			this,
			(Case) intent.getSerializableExtra("CASE_CONFIG")
			);

		setContentView(glView);
	}

	@Override
	protected void onResume() {
		// Ideally a game should implement onResume() and onPause()
		// to take appropriate action when the activity looses focus
		super.onResume();
		glView.onResume();
	}

	@Override
	protected void onPause() {
		// Ideally a game should implement onResume() and onPause()
		// to take appropriate action when the activity looses focus
		super.onPause();
		if (!isFinishing()) {
			// Exit condition 2: new activity becomes front.
			finish();
		}
		glView.onPause();
	}

	@Override
	public boolean onKeyDown(int key, KeyEvent evt) {
		return glView.onKeyDown(key, evt);
	}
}

/**
 * Surface view for the native application.
 */
class GLNativeView extends GLSurfaceView {

	private Context		context;
	private Case        cas;
	private Native		nati;
	private OurRenderer renderer;

	public GLNativeView(Context context, Case cas) {
		super(context);

		this.context = context;
		this.cas     = cas;
		this.nati	 = new Native();
		this.renderer= new OurRenderer();

		setEGLContextFactory(new OurContextFactory());
		setEGLConfigChooser(new OurConfigChooser());
		setRenderer(renderer);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent evt) {

		if (evt.getAction() == KeyEvent.ACTION_DOWN ||
			evt.getAction() == KeyEvent.ACTION_UP) {
			queueEvent(new KeyEventSender(
					keyCode,evt.getAction() == KeyEvent.ACTION_DOWN));
			return true;
		}

		return false;
	}

	@Override
	public boolean onTouchEvent(MotionEvent evt) {
		int x = (int) evt.getX(0);
		int y = (int) evt.getY(0);

		if (evt.getAction() == KeyEvent.ACTION_DOWN ||
			evt.getAction() == KeyEvent.ACTION_UP) {
			queueEvent(new TouchEventSender(
					x, y, evt.getAction() == KeyEvent.ACTION_DOWN));

			return true;
		}

		return false;
	}

	@Override
	public void surfaceDestroyed (SurfaceHolder holder)  {
		super.surfaceDestroyed(holder);
	}

	@Override
	public void onPause() {
		queueEvent(new FinishEventSender());
	}

	/**
	 * Send key event to renderer.
	 */
	private class KeyEventSender implements Runnable {

		private int key;
		private boolean down;

		public KeyEventSender(int key, boolean down) {
			this.key = key;
			this.down = down;
		}

		public void run() {
			renderer.key(key, down);
		}
	}

	/**
	 * Send touch event to renderer.
	 */
	private class TouchEventSender implements Runnable {

		private int x;
		private int y;
		private boolean down;

		public TouchEventSender(int x, int y, boolean down) {
			this.x = x;
			this.y = y;
			this.down = down;
		}

		public void run() {
			renderer.touch(x, y, down);
		}
	}

	/**
	 * Send pause event to renderer.
	 */
	private class FinishEventSender implements Runnable {

		public void run() {
			renderer.finish();
		}
	}

	/**
	 * Choose egl config.
	 */
	private class OurConfigChooser implements GLSurfaceView.EGLConfigChooser {
		private final static int EGL_OPENGL_ES_BIT = 1;
		private final static int EGL_OPENGL_ES2_BIT = 4;

		public OurConfigChooser() {}

		public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

			int[] conflist = {
				EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
				EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
				EGL10.EGL_RED_SIZE,		cas.red,
				EGL10.EGL_GREEN_SIZE,	cas.green,
				EGL10.EGL_BLUE_SIZE,	cas.blue,
				EGL10.EGL_ALPHA_SIZE,	cas.alpha,
				EGL10.EGL_DEPTH_SIZE,	cas.depth,
				EGL10.EGL_STENCIL_SIZE,	cas.stencil,
				EGL10.EGL_NONE,			EGL10.EGL_NONE,
				EGL10.EGL_NONE
			};

			if (cas.getGLVersion() == 2) {
				conflist[1] = EGL_OPENGL_ES2_BIT;
			}
			else {
				conflist[1] = EGL_OPENGL_ES_BIT;
			}

			if (cas.msaa != 1) {
				conflist[16] = EGL10.EGL_SAMPLES;
				conflist[17] = cas.msaa;
			}

			int[] num_config = new int[1];
			egl.eglChooseConfig(display, conflist, null, 0, num_config);

			if(num_config[0] <= 0)
				throw new IllegalArgumentException("Failed to find a suitable config");

			EGLConfig[] configs = new EGLConfig[num_config[0]];
			egl.eglChooseConfig(display, conflist, configs, num_config[0], num_config);

			int[] value = new int[1];

			// We need to find a suitable config that matches our pixel format
			for(int i = 0; i < num_config[0]; ++i) {
				if((egl.eglGetConfigAttrib(display, configs[i], EGL10.EGL_RED_SIZE,   value) && value[0] == cas.red)
				&& (egl.eglGetConfigAttrib(display, configs[i], EGL10.EGL_GREEN_SIZE, value) && value[0] == cas.green)
				&& (egl.eglGetConfigAttrib(display, configs[i], EGL10.EGL_BLUE_SIZE,  value) && value[0] == cas.blue)
				&& (egl.eglGetConfigAttrib(display, configs[i], EGL10.EGL_ALPHA_SIZE, value) && value[0] == cas.alpha))
					return configs[i];
			}

			return null;
		}
	}

	/**
	 * Create egl context.
	 */
	private class OurContextFactory implements GLSurfaceView.EGLContextFactory {


		public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
			// Used for OpenGL ES 2
			int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
			int[] attrib_list = {
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL10.EGL_NONE
			};

			return egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, (cas.getGLVersion() == 2) ? attrib_list : null);
		}

		public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
			egl.eglDestroyContext(display, context);
		}
	}

	/**
	 * The surface renderer.
	 * All methods in it should be in a single thread.
	 */
	private class OurRenderer implements GLSurfaceView.Renderer {

		private boolean running;
		private boolean finished;

		public void onDrawFrame(GL10 gl) {
			// Call our native display function
			if (running) {

				if (nati.repaint()) {
					running = false;
					// Exit condition 1: application requested finish.
					((Activity) context).finish();
				}
			}
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			if (nati.init(cas.getID(), width, height)) {
				running = true;
				finished = false;
			}
			else {
				running = false;
				finished = true;
			}
		}

		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			// Do nothing.
		}

		public void key(int key, boolean down) {
			if (running && !finished) {
				nati.key(key, down);
			}
		}

		public void touch(int x, int y, boolean down) {
			if (running && !finished) {
				nati.touch(x, y, down);
			}
		}

		/**
		 * Call native finish, set finished state, disable repaint.
		 */
		public void finish() {
			if (!finished) {
				running = false;
				finished = true;
				nati.fini();
			}
		}
	}
}

