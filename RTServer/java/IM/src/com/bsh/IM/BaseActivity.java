package com.bsh.IM;

import java.util.ArrayList;

import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smackx.filetransfer.FileTransferManager;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.Toast;

public abstract class BaseActivity extends Activity {
	protected static final int SHOW_PROGRESS = 0;
	protected static final int DISMISS_PROGRESS = 1;
	protected static final int DOWNLOAD_FILE=2;
	public static String account;
	public static XMPPConnection conn;
	public ProgressBar pb;
	public static ArrayList<Activity> activityHolder = new ArrayList<Activity>();
	public static FileTransferManager manager;//创建一个文件传输管理器
	/**
	 * 定义一个全局的handler
	 */
	public Handler handler = new Handler() {
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case SHOW_PROGRESS:
				showPb();
				afterShowProgressLogic();
				break;

			case DISMISS_PROGRESS:
				dismissPb();
				afterDismissProgressLogic();
				break;
			case DOWNLOAD_FILE:
				Toast.makeText(getApplicationContext(), "来文件了", 1).show();
				break;
			}
		}
	};
	
	protected void onCreate(android.os.Bundle savedInstanceState) {
		setContentView();
		init();
		findview();
		setListener();
		super.onCreate(savedInstanceState);
		
	};
	/**
	 * 设置监听事件
	 */
	protected void setListener() {
		// TODO Auto-generated method stub
		
	}
	/**
	 * 为activity设置layout
	 */
	protected void setContentView() {
		// TODO Auto-generated method stub
		
	}
	/**
	 * 创建activity后的一些初始化操作
	 */
	public void init() {
		activityHolder.add(this);
	
	}

	/**
	 * 寻找控件
	 */
	abstract void findview();

	/**
	 * 显示进度条的方法
	 */
	public void showPb() {
		pb = (ProgressBar) findViewById(R.id.progressbar);
		pb.setVisibility(View.VISIBLE);
	}

	/**
	 * 隐藏进度条
	 */
	public void dismissPb() {
		pb.setVisibility(View.GONE);
	}

	/**
	 * handler中显示进度条后的处理逻辑，可以由子类复写，实现自己的逻辑
	 */
	public void afterShowProgressLogic() {
		// TODO Auto-generated method stub

	};

	/**
	 * handler中在消失进度条后的处理逻辑，可以由子类复写，实现自己的逻辑
	 */
	public void afterDismissProgressLogic() {
		// TODO Auto-generated method stub

	}

	/**
	 * 判读当用户按下后退键时，退出应用程序
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			if (keyCode == KeyEvent.KEYCODE_BACK) {
				AlertDialog.Builder builder = new Builder(
						this);
				builder.setTitle("提示");
				builder.setMessage("确定要退出应用程序吗?");
				builder.setPositiveButton("确定", new OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						conn.disconnect();
						for(int i=0;i<activityHolder.size();i++){
							if(activityHolder.get(i)!=null)
								activityHolder.get(i).finish();
						}
						activityHolder.clear();
						
					}
				});
				builder.setNegativeButton("取消", new OnClickListener() {

					public void onClick(DialogInterface dialog, int which) {
						return;
					}
				});
				builder.create().show();
			}

		}

		return true;
	}

}
