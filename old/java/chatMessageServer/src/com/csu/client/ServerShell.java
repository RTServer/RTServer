package com.csu.client;

import java.util.Scanner;

import com.csu.server.ChatServer;

public class ServerShell {
	
	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		System.out.println("请输入端口号");
		int port = input.nextInt();
		try {
			ChatServer server = new ChatServer(port);
			server.start();
			System.out.println("服务已启动");
			System.out.println("退出服务请输入quit");
			String quit = input.next();
			if (quit.equals("quit")) {
				server.quit();
				System.out.println("服务已结束");
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
