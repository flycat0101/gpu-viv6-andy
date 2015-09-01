package com.vivantecorp.graphics.tiger2DVG;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class TigerActivity extends ListActivity{
	/** Called when the activity is first created. */
	public String[] casenames =  new String[]{"tiger2DVG"};

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setListAdapter(
        	new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, casenames)
        );
    }

    @Override
    public void onDestroy()
    {
    	super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent evt){
    	super.onKeyDown(keyCode, evt);

    	if(keyCode == KeyEvent.KEYCODE_BACK){
    		finish();
    	}
    	return true;
    }
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
    {

    	Log.w("ListCases",String.format("The position is %d, the id is %d", position, id));
      	Intent intent = new Intent();

    	intent.setClass(this, RunCaseActivity.class);
    	intent.putExtra("KEY_CASEID", position);

    	startActivity(intent);
    }
}
