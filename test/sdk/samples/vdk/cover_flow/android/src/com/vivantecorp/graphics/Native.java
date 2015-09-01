package com.vivantecorp.graphics;

/**
 * Native function interface.
 */
public class Native {

	/**
	 * run specific case.
	 * @param caseID caseID to be run.
	 * @param width native window width.
	 * @param height native window height.
	 * @return true if success.
	 */
	public native boolean init(int caseID, int width, int height);

	/**
	 * finalize application.
	 */
	public native void fini();

	/**
	 * render a frame.
	 * @return Done if true
	 */
	public native boolean repaint();

	/**
	 * keyboard event.
	 * @param key key code.
	 * @param down key down or up.
	 */
	public native void key(int key, boolean down);

	/**
	 * touch screen/mouse click event.
	 * @param x the x coordinate of the event.
	 * @param y the y coordinate of the event.
	 * @param down touch down or up.
	 */
	public native void touch(int x, int y, boolean down);
}

