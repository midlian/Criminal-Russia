package com.criminalrussia.launcher.model;

public class News {

	private final String imageUrl;
	private final String title;

	public News(String imageUrl, String title) {
		this.imageUrl = imageUrl;
		this.title = title;
	}
	public String getImageUrl() {
		return imageUrl;
	}

	public String getTitle() {
		return title;
	}

}