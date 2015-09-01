

package com.vivantecorp.graphics.tutorial7;

// Wrapper for native library

public class GL2JNILib {

     static {
         System.loadLibrary("gl2tutorial7");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native boolean init(int width, int height);
     public static native boolean repaint();
     public static native boolean key(int key, boolean down);
}
