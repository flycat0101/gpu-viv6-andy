package com.vivantecorp.graphics.shareContext;

import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.egl.EGLDisplay;

// Wrapper for native library

public class GL2JNILib
{
     static
     {
         System.loadLibrary("gl2shareContext");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native boolean init(Object surface, Object surface2);
     public static native boolean repaint();
     public static native boolean key(int key, boolean down);
     public static native boolean finish();
}
