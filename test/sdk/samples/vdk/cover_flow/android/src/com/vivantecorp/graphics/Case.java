package com.vivantecorp.graphics;

/**
 * Define menu for entry.
 */
public final class Case implements java.io.Serializable {

	private String title;
	private int id;
	private int glVersion = 1;
	public  int red       = 5;
	public  int green     = 6;
	public  int blue      = 5;
	public  int alpha     = 0;
	public  int depth     = 16;
	public  int stencil   = 0;
	public  int msaa      = 1;

	public Case(String title, int id) {
		if (title == null) {
			throw new IllegalArgumentException("Title is null");
		}

		if (id < 0) {
			throw new IllegalArgumentException("case id is negative");
		}

		this.title = title;
		this.id    = id;
	}

	public Case(String title, int id, int msaa) {
		this(title, id);

		this.msaa = msaa;
	}

	public Case(String title, int id, int red, int green, int blue, int alpha, int depth, int stencil, int msaa) {
		this(title, id, msaa);

		this.red     = red;
		this.green   = green;
		this.blue    = blue;
		this.alpha   = alpha;
		this.depth   = depth;
		this.stencil = stencil;
	}

	public String getTitle() {
		return title;
	}

	public int getID() {
		return id;
	}

	public Case setGLVersion(int ver) {
		if (ver != 1 && ver != 2) {
			throw new IllegalArgumentException("Invalid GL Version: " + ver);
		}
		this.glVersion = ver;

		return this;
	}

	public int getGLVersion() {
		return glVersion;
	}
}

