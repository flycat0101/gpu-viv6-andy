package com.vivantecorp.graphics;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Process;
import android.view.View;
import android.view.KeyEvent;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.Vector;

/**
 * This class is a wrapper for native applications to run on android.
 * To use this wrapper, two things must to be done.
 * 1. Write a java class implements <code>Native</code> and hook
 * native functions.
 * 2. Write a class extends <code>ActivityWrapper</code>, passing arguments
 * in its constructor.
 */
public class ActivityWrapper extends ListActivity {

	private Vector<Case> cases = new Vector<Case>();

	/**
	 * Append a case to run.
	 * @param cas the case config.
	 */
	protected void appendCase(Case cas) {
		if (cas != null) {
			cases.add(cas);
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		String[] names = new String[cases.size()];

		if (cases.size() == 0) {
			names = new String[] {"Start"};
		}
		else {
			for (int i = 0; i < cases.size(); i++) {
				names[i] = cases.get(i).getTitle();
			}
		}

		setListAdapter(
				new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, names)
			);
	}

	@Override
	public void onDestroy() {
		super.onDestroy();

		Process.killProcess(Process.myPid());
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		Intent intent = new Intent();

		intent.setClass(this, NativeActivity.class);
		intent.putExtra("CASE_CONFIG", cases.get(position));

		startActivity(intent);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent evt) {
		super.onKeyDown(keyCode, evt);

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			finish();
		}

		return true;
	}
}

