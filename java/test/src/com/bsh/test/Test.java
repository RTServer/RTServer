package com.bsh.test;

public class Test {
	
	public static void main(String[] args) {
		final String USERID = "bsh";
		final String PWD = "123";
		Api.login(USERID, PWD);
		Api.message(2, "ni hao");
		System.out.println("Name:" + USERID + "	Token:" + Api.getToken());
	}
}