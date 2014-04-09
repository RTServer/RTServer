package com.bsh.IM;

import org.jivesoftware.smack.PacketListener;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.packet.Message;
import org.jivesoftware.smack.packet.Packet;
import org.jivesoftware.smackx.muc.MultiUserChat;

import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

public class conferenceActivity extends BaseActivity {
	
	
	protected static final int UPDATE_MESSAGE = 0;
	private TextView conferenceRosterTV;
	private EditText conferenceET;
	private TextView conferenceContentTV;
	private Handler handler1=new Handler(){
		public void handleMessage(android.os.Message msg) {
			switch (msg.what) {
			case UPDATE_MESSAGE:
				String result=msg.getData().getString("result");
				conferenceContentTV.setText(conferenceContentTV.getText()+"\n"+result);
				conferenceET.setText("");
				break;

			}
		};
	};
	private MultiUserChat chat;
	@Override
	protected void setContentView() {
		setContentView(R.layout.conference);
	}
	@Override
	void findview() {
		conferenceRosterTV = (TextView) findViewById(R.id.conferenceRosterTV);
		conferenceET = (EditText) findViewById(R.id.conferenceET);
		conferenceContentTV = (TextView) findViewById(R.id.conferenceContentTV);
	}
	
	@Override
	protected void setListener() {
		String jid=getIntent().getStringExtra("jid");
		chat = new MultiUserChat(conn,jid);
		try {
			chat.join(account);
			chat.addMessageListener(new PacketListener() {
				
				public void processPacket(Packet packet) {
					Message msg=(Message)packet;
					String msgbody=msg.getBody();
					String msgfrom=msg.getFrom();
					android.os.Message message=android.os.Message.obtain();
					message.what=UPDATE_MESSAGE;
					Bundle data=new Bundle();
					data.putString("result", msgfrom.substring(msgfrom.indexOf("/")+1, msgfrom.length())+":"+msgbody);
					message.setData(data);
					handler1.sendMessage(message);
				}
			});
		} catch (XMPPException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}
	
	public void send(View view){
		String sendContent=conferenceET.getText().toString();
		if("".equals(sendContent))
			return;
		try {
			chat.sendMessage(sendContent);
		} catch (XMPPException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
