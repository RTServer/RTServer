package com.bsh.IM;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smackx.muc.HostedRoom;
import org.jivesoftware.smackx.muc.MultiUserChat;

import android.content.Intent;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class GroupActivity extends BaseActivity {

	private static final String Tag = "GroupActivity";
	private ListView conferencesLV;
	private List<HostedRoom> hostedRooms;

	@Override
	void findview() {
		conferencesLV = (ListView) findViewById(R.id.conferencesLV);
	}
	
	@Override
	protected void setContentView() {
		setContentView(R.layout.allconferences);
	}
	
	@Override
	protected void setListener() {
		try {
			Collection<HostedRoom> hostedrooms=MultiUserChat.getHostedRooms(conn, "conference.wanglq.com");
			hostedRooms = new ArrayList<HostedRoom>();
			for(HostedRoom hostedroom:hostedrooms){
				Log.i(Tag,hostedroom.getJid());
				hostedRooms.add(hostedroom);
			}
			/**
			 * 为listview设置adapter
			 */
			conferencesLV.setAdapter(new BaseAdapter() {
				
				public View getView(int position, View convertView, ViewGroup parent) {
					// TODO Auto-generated method stub
					View view=View.inflate(getApplicationContext(),R.layout.conferencelist, null);
					TextView conferenceJidTV=(TextView) view.findViewById(R.id.conferenceJidTV);
					TextView conferenceNameTV=(TextView) view.findViewById(R.id.conferenceNameTV);
					HostedRoom hostedroom=hostedRooms.get(position);
					conferenceJidTV.setText(hostedroom.getJid());
					conferenceNameTV.setText(hostedroom.getName());
					return view;
				}
				
				public long getItemId(int position) {
					return position;
				}
				
				public Object getItem(int position) {
					return hostedRooms.get(position);
				}
				public int getCount() {
					return hostedRooms.size();
				}
			});
			
			/**
			 * 为listview设置点击事件
			 */
			conferencesLV.setOnItemClickListener(new OnItemClickListener() {

				public void onItemClick(AdapterView<?> parent, View view,
						int position, long id) {
					Intent intent=new Intent(GroupActivity.this,conferenceActivity.class);
					intent.putExtra("jid", hostedRooms.get(position).getJid());
					startActivity(intent);
				}
				
			});
		} catch (XMPPException e) {
			e.printStackTrace();
		}
	}

}
