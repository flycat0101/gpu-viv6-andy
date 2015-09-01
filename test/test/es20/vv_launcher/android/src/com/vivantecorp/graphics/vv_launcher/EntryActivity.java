package com.vivantecorp.graphics.vv_launcher;

import com.vivantecorp.graphics.*;

/*
	Case(String title, int id);
	Case(String title, int id, int msaa);
	Case(String title, int id, int red, int green, int blue, int alpha, int depth, int stencil, int msaa);
*/
public class EntryActivity extends ActivityWrapper {

	static {
		System.loadLibrary("vv_launcher");
	}

	public EntryActivity() {
		appendCase(new Case("Run Vivante launcher", 0).setGLVersion(2));
	}
}

