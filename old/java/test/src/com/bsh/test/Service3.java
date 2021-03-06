package com.bsh.test;

//http://yangguangfu.iteye.com/blog/774194
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Properties;
import java.util.Random;
  
public class Service3 extends Thread {
	public static final String HOST = "211.88.253.7";
	public static final int PORT = 5566;
	  
	boolean exist = false;
	  
	Properties talks = new Properties();
	Random random = new Random();
	String[] keys;
	  
	int messageCount = 0;
	  
	public void run() {
		try {
		   // 对话内容  
		   talks.load(new FileInputStream("E:\\talk.properties"));
		  
		   // 客户端发送 "=" 左边的内容  
		   keys = new String[talks.size()];
		   talks.keySet().toArray(keys);
		  
		   Socket socket = new Socket(HOST, PORT);
		  
		   OutputStream ous = socket.getOutputStream();
		   InputStream ins = socket.getInputStream();
		  
		   // 接收线程，接收服务器的回应  
		   RecieverThread reciever = new RecieverThread(ins);
		   reciever.start();
		  
		   while (!exist) {
		  
		    messageCount++;
		  
		    // 选择一个随机消息  
		    String msg = chooseMessage();
		  
		    synchronized (ins) {
		  
		     // 发送给服务器端  
		     ous.write(msg.getBytes("UTF-8"));
		  
		     System.out.println("[send]\t" + messageCount + ": " + msg);
		  
		     // 然后等待接收线程  
		     ins.wait();
		    }
		  
		    if (msg.equals("Bye")) {
		     break;
		    }
		   }
		  
		   ins.close();
		   ous.close();
		   socket.close();
		  
		} catch (Exception e) {
		   e.printStackTrace();
		}
	  
	}
	  
	public String chooseMessage() {
	  
	int index = random.nextInt(keys.length);
	String msg = keys[index];
	  
	// 如果 10 次就选中 Bye，则重新选择，为了使对话内容多一些  
	if (messageCount < 10 && msg.equalsIgnoreCase("Bye")) {
	   return chooseMessage();
	}
	  
	return msg;
	}
	  
	// 接收线程  
	class RecieverThread extends Thread {
	private InputStream ins;
	  
	public RecieverThread(InputStream ins) {
	   this.ins = ins;
	}
	  
	@Override  
	public void run() {
	  
	   try {
	    String line = null;
	  
	    BufferedReader r = new BufferedReader(new InputStreamReader(
	      ins, "UTF-8"));
	  
	    // readLine()会阻塞，直到服务器输出一个 '\n'  
	    while ((line = r.readLine()) != null) {
	  
	     System.out.println("[Recieved]: " + line);
	  
	     synchronized (ins) {
	      // 接收到服务器的消息，通知下主线程  
	      ins.notify();
	     }
	     if (line.trim().equals("Bye")) {
	      exist = true;
	      break;
	     }
	    }
	   } catch (Exception e) {
	    e.printStackTrace();
	   }
	}
	  
	}
	  
	public static void main(String[] args) throws Exception {
	  
	// 开三个客户端线程  
	for (int i = 0; i < 3; i++) {
	   try {
	    new Service3().start();
	   } catch (Exception e) {
	    e.printStackTrace();
	   }
	}
	  
	}
}