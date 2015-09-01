package com.vivantecorp.graphics.tiger2DVG;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class RunCaseActivity extends Activity {
	 /** Called when the activity is first created. */
	private final int MAX_CASE_NUM = 30;
    private OES20View mView;
    private int CaseID;
    private boolean[] caseValues = new boolean[MAX_CASE_NUM];

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        init();
        Intent intent = getIntent();
        CaseID = intent.getIntExtra("KEY_CASEID", 0);
        caseValues[CaseID] = true;
        mView = new OES20View(this, caseValues);
        setContentView(mView);
        mView.start();
    }
    @Override
    protected void onResume()
    {
        super.onResume();
    }
    @Override
    protected void onPause()
    {
        super.onPause();
    }

    public void init()
    {
    	for(int i = 0; i < MAX_CASE_NUM; i++)
    	{
    		caseValues[i] = false;
    	}
    }
}
