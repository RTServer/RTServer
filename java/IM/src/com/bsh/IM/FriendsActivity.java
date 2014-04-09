package com.bsh.IM;

import android.app.TabActivity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;

public class FriendsActivity extends TabActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		BaseActivity.activityHolder.add(this);
		TabHost tab=getTabHost();
		TabSpec spec1=tab.newTabSpec("lianxiren");
		TabSpec spec2=tab.newTabSpec("conference");
		TabSpec spec3=tab.newTabSpec("sendfile");
		spec3.setIndicator("发送文件");
		spec1.setIndicator("联系人");
		spec2.setIndicator("会议室");
		Intent intent1=new Intent(this,ContactsActivity.class);
		Intent intent2=new Intent(this, GroupActivity.class);
		Intent intent3=new Intent(this,SendFileActivity.class);
		spec1.setContent(intent1);
		spec2.setContent(intent2);
		spec3.setContent(intent3);
		tab.addTab(spec1);
		tab.addTab(spec2);
		tab.addTab(spec3);
	}
}
