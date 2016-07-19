package com.vivantecorp.graphics.sample1;

// Wrapper for native library

public class GL2JNILib {

     static {
         System.loadLibrary("gl2sample1_jni");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native boolean init(int width, int height);
     public static native boolean repaint();

    /**
     * keyboard event.
     * @param key key code.
     * @param down key down or up.
     */
	public static native void key(int key, boolean down);


}
