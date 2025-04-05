package com.criminalrussia.launcher.model;

public class Servers {

	private final boolean x2;
	private final String name;
	private final int online; //no work
	private final int maxOnline;  //no work

	public Servers(boolean x2, String name, int online, int maxOnline) {
		this.x2 = x2;
		this.name = name;
		this.online = online;
		this.maxOnline = maxOnline;
	}
	 
	public String getName() {
		return name;
	}

    public Boolean getX2() {
		 return x2;
	}
	
	public int getOnline(){
		return online;
	}

	public int getMaxOnline(){
		return maxOnline;
	}
}