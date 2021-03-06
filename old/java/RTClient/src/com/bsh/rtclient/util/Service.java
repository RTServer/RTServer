package com.bsh.rtclient.util;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;

public class Service {

	/**
	 * 定义连接标示静态变量
	 */
	private static Socket con = null;
	
	/**
	 * 获取连接句柄
	 * @return
	 */
	public static Socket getConnection() {
		if (con == null) {
			try {
				//建立客户端socket连接，指定服务器位置及端口
				con = new Socket("211.88.253.7", 5566);
			} catch (UnknownHostException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return con;
	}
	
	/**
	 * 关闭连接
	 */
	public static void closeConnection() {
		if (con != null) {
			try {
				con.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * 发送数据
	 */
	public static String sendData(String data) {
		String res = "";
		if (con != null) {
			try {
				//得到socket读写流
		        OutputStream os = con.getOutputStream();
		        if (con.isOutputShutdown()) System.out.println("isOutputShutdown");
				PrintWriter pw = new PrintWriter(os);
				//输入流
				InputStream is = con.getInputStream();
				BufferedReader br = new BufferedReader(new InputStreamReader(is));
				//利用流按照一定的操作，对socket进行读写操作
				pw.write(data);
				pw.flush();
				con.shutdownOutput();
				//接收服务器的相应
				String reply = "";
		        while(!((reply = br.readLine()) == null)){
		        	res += reply;
		        }
		        //关闭资源
		        br.close();
		        is.close();
		        pw.close();
		        os.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return res;
	}
}
