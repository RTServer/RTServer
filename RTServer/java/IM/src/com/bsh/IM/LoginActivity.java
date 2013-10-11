package com.bsh.IM;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;

import android.content.Intent;
import android.net.wifi.WifiConfiguration.Status;
import android.os.Environment;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import org.jivesoftware.smackx.ServiceDiscoveryManager;
import org.jivesoftware.smackx.filetransfer.FileTransferListener;
import org.jivesoftware.smackx.filetransfer.FileTransferManager;
import org.jivesoftware.smackx.filetransfer.FileTransferRequest;
import org.jivesoftware.smackx.filetransfer.IncomingFileTransfer;

import com.bsh.IM.util.XmppTool;

public class LoginActivity extends BaseActivity implements OnClickListener{
	private EditText passwordET;
	private EditText usernameET;
	private Button loginBT;
	private Button registerBT;

	@Override
	protected void setContentView() {
		setContentView(R.layout.login);
	}

	@Override
	protected void setListener() {
		loginBT.setOnClickListener(this);
	}

	public void findview() {
		passwordET = (EditText) findViewById(R.id.passwordET);
		usernameET = (EditText) findViewById(R.id.usernameET);
		loginBT = (Button) findViewById(R.id.loginBT);
		registerBT = (Button) findViewById(R.id.registerBT);
	}

	public void onClick(View v) {
		switch (v.getId()) {
			case R.id.loginBT:
				final String username = usernameET.getText().toString();
				final String password = passwordET.getText().toString();
				account = username;
				showPb();
				new Thread(new Runnable() {
					public void run() {
						try {
							conn = XmppTool.getConnection();
							conn.login(username, password);
							Message msg = Message.obtain();
							msg.what = DISMISS_PROGRESS;
							handler.sendMessage(msg);
						} catch (Exception e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							XmppTool.closeConnection();
							handler.sendEmptyMessage(2); //Ϊʲô��ʾ�����ļ��ˡ�
						}
					}
				}).start();
				break;
		}
	}
	
	/**
	 * ������������ʧ����߼�
	 */
	@Override
	public void afterDismissProgressLogic() {
		/**
		 * �����ļ������������ļ����͹���ʱ�Զ�����
		 */
		new ServiceDiscoveryManager(conn);
		manager=new FileTransferManager(conn);//��ʼ���ļ����������
		manager.addFileTransferListener(new FileTransferListener() {
			
			public void fileTransferRequest(FileTransferRequest request) {
				final IncomingFileTransfer incoming = request.accept();
				final String filename=incoming.getFileName();
					
				new Thread(){
					public void run() {
						try {
							System.out.println("��ʼ����");
							Message msg=Message.obtain();
							msg.what=DOWNLOAD_FILE;
							handler.sendMessage(msg);
//							InputStream in=incoming.recieveFile();
//							File file=new File(Environment.getExternalStorageDirectory(), filename);
//							FileOutputStream out=new FileOutputStream(file);
//							byte[] b=new byte[1024];
//							int len=0;
//							while((len=in.read(b))!=-1){
//								out.write(b, 0, len);
//							}
//							out.flush();
//							out.close();
							incoming.recieveFile(new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), filename));
						} catch (Exception e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					};
				}.start();
			}
		});
		Intent friendsIntent=new Intent(LoginActivity.this, FriendsActivity.class);
		startActivity(friendsIntent);
	}
}