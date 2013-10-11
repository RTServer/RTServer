package com.bsh.IM;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.packet.Presence;
import org.jivesoftware.smackx.ServiceDiscoveryManager;
import org.jivesoftware.smackx.filetransfer.FileTransferListener;
import org.jivesoftware.smackx.filetransfer.FileTransferManager;
import org.jivesoftware.smackx.filetransfer.FileTransferRequest;
import org.jivesoftware.smackx.filetransfer.IncomingFileTransfer;
import org.jivesoftware.smackx.filetransfer.OutgoingFileTransfer;

import android.os.Environment;
import android.view.View;
import android.widget.EditText;

public class SendFileActivity extends BaseActivity {

	private EditText destnationET;
	private EditText filepathET;
	@Override
	protected void setContentView() {
		setContentView(R.layout.sendfile);
	}

	@Override
	void findview() {
		destnationET = (EditText) findViewById(R.id.destnationET);
		filepathET = (EditText) findViewById(R.id.filepathET);
	}

	public void send(View view) {
		final String userId = destnationET.getText().toString();
		final String filepath = filepathET.getText().toString();
		new Thread() {
			public void run() {
				try {
					Presence presence = new Presence(Presence.Type.available);
					conn.sendPacket(presence);
					File file = new File(filepath);
					InputStream in = new FileInputStream(file);
				/**
				 * 如果给spark传输时需用下面的代码
				 */
//					final OutgoingFileTransfer outgoingFileTransfer = manager
//							.createOutgoingFileTransfer(userId + "/Spark 2.6.3");
					final OutgoingFileTransfer outgoingFileTransfer = manager
							.createOutgoingFileTransfer(userId + "/Smack");
					// outgoingFileTransfer.sendFile(file, file.getName());
					outgoingFileTransfer.sendStream(in, file.getName(),
							file.length(), file.getName());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}.start();
	}
}
