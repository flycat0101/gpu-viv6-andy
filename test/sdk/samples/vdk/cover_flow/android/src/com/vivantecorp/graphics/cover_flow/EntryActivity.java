package com.vivantecorp.graphics.cover_flow;

import com.vivantecorp.graphics.*;

/*
	Case(String title, int id);
	Case(String title, int id, int msaa);
	Case(String title, int id, int red, int green, int blue, int alpha, int depth, int stencil, int msaa);
*/
public class EntryActivity extends ActivityWrapper {

	static {
		System.loadLibrary("cover_flow");
	}

	public EntryActivity() {
		appendCase(new Case("OpenGL ES1.1 CoverFlow", 0));
	}
}

