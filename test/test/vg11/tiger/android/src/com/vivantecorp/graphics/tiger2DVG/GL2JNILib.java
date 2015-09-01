package com.vivantecorp.graphics.tiger2DVG;

public class GL2JNILib {
	static{
		System.loadLibrary("Tiger2DVG");
	}
	public static native void nativeRunTest(Object surface, boolean[] values);
	public static native void init();
	public static native int getWidth();
	public static native int getHeight();
}
