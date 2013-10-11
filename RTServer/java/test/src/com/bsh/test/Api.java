package com.bsh.test;

import org.json.JSONObject;

public class Api {
	private static String token = null;
	private static int id = 0;

	public static void login(final String name, final String password) {
		Service.getConnection();
		String res = Service.sendData("{\"action\":\"login\",\"name\":\"" + name + "\",\"password\":\"" + password + "\"}");
		if (res != null) {
			JSONObject jsonobj = new JSONObject(res);
			id = jsonobj.getInt("id");
			token = jsonobj.getString("token");
		}
	}
	
	public static void message(int toid, String msg) {
		String res = Service.sendData("{\"action\":\"message\",\"id\":" + id + ",\"token\":\"" + token + "\",\"toid\":" + toid + ",\"content\":\"" + msg + "\"}");
		System.out.println(res);
	}
	
	public static void logout() {
		Service.closeConnection();
	}
	
	public static int getId() {
		return id;
	}
	
	public static String getToken() {
		return token;
	}
	
}
