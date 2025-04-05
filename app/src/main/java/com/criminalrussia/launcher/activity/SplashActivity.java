package com.criminalrussia.launcher.activity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings;
import com.criminalrussia.launcher.Config;
import com.criminalrussia.launcher.Preferences;
import com.criminalrussia.launcher.model.News;
import criminal.russia.R;

import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import java.util.ArrayList;

import com.criminalrussia.launcher.model.Servers;
import com.criminalrussia.launcher.other.Lists;
import com.criminalrussia.launcher.utils.Utils;

public class SplashActivity extends AppCompatActivity{
    private FirebaseRemoteConfig mFirebaseRemoteConfig;
	
    @SuppressLint("NewApi")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        final LinearLayout privacy = findViewById(R.id.brp_launcher_privacy);
        Button privacyOk = privacy.findViewById(R.id.brp_launcher_privacy_ok);
        Button privacyDecline = privacy.findViewById(R.id.brp_launcher_privacy_decline);
        TextView privacyLink = privacy.findViewById(R.id.brp_launcher_privacy_link);
        AnimationUtils.loadAnimation(this, R.anim.btn_click);

        if (Preferences.getString(this, Preferences.FIRST_START).isEmpty()) {
            runOnUiThread(() -> Utils.ShowView(privacy, true));
        } else if(isOnline(this)) {
            DoIt();
        } else {
            ConstraintLayout notInternetConnection = findViewById(R.id.brp_launcher_not_internet);
            Utils.ShowView(notInternetConnection, true);
        }
        privacyOk.setOnClickListener(view -> {
            Preferences.putString(getApplicationContext(), Preferences.FIRST_START, "true");
            Utils.HideView(privacy, true);

            Timer t = new Timer();
            t.schedule(new TimerTask(){
                @Override
                public void run() {
                    DoIt();
                }
            }, 500L);
        });
        privacyDecline.setOnClickListener(view -> {
            Utils.HideView(privacy, true);
            finish();
        });
        privacyLink.setOnClickListener(view -> startActivity(new Intent("android.intent.action.VIEW", Uri.parse("http://176.124.220.143/dl/mobile/launcher/user_service.pdf"))));
    }

    private void DoIt() {
        Lists.serverList = new ArrayList<>();

        Lists.serverList = new ArrayList<>();
        Lists.newsList = new ArrayList<>();

        Lists.serverList.add(new Servers(true, "Criminal Russia Beta", 0, 500));
        Lists.serverList.add(new Servers(false, "Criminal Russia Test", 0, 1000));

        Lists.newsList.add(
                new News(
                        "https://i.ytimg.com/vi/0L4g6BLtcOM/maxresdefault.jpg",
                        "НОВОСТИ"
                )
        );
        Lists.newsList.add(
                new News(
                        "https://i.ytimg.com/vi/W5D4ZS5bEAA/maxresdefault.jpg",
                        "НОВОСТИ"
                )
        );

        HashMap<String, Object> defaultsRate = new HashMap<>();
            defaultsRate.put("json", String.valueOf(0));

            mFirebaseRemoteConfig = FirebaseRemoteConfig.getInstance();
            FirebaseRemoteConfigSettings configSettings = new FirebaseRemoteConfigSettings.Builder()
                    .setMinimumFetchIntervalInSeconds(3600) // 3600 for release :)
                    .build();

            mFirebaseRemoteConfig.setConfigSettingsAsync(configSettings);
            mFirebaseRemoteConfig.setDefaultsAsync(defaultsRate);

            mFirebaseRemoteConfig.fetchAndActivate().addOnCompleteListener(this, task -> {
                if (task.isSuccessful()) {
                    // files
                    Config.URL_FILES = mFirebaseRemoteConfig.getString("cache_url");
                    Config.VER_CACHE = mFirebaseRemoteConfig.getString("cache_version");

                    // update
                    Config.URL_FILES_UPDATE = mFirebaseRemoteConfig.getString("cache_update_url");
                    Config.URL_APK = mFirebaseRemoteConfig.getString("apk_url");
                    Config.VER_APK = mFirebaseRemoteConfig.getString("apk_version");

                    // social
                    Config.URL_VK = mFirebaseRemoteConfig.getString("vk");
                    Config.URL_TG = mFirebaseRemoteConfig.getString("tg");
                    Config.URL_DS = mFirebaseRemoteConfig.getString("discord");

                    // access
                    final boolean access = mFirebaseRemoteConfig.getBoolean("access");
                    if(access) {
                        startActivity(new Intent(SplashActivity.this, MainActivity.class));
                        finish();
                    } else {
                        ConstraintLayout notInternetConnection = findViewById(R.id.brp_launcher_not_internet);
                        Utils.ShowView(notInternetConnection, true);
                    }
                } else {
                    ConstraintLayout notInternetConnection = findViewById(R.id.brp_launcher_not_internet);
                    Utils.ShowView(notInternetConnection, true);
                }
            });
    }

	public static boolean isOnline(Context context)
    {
        ConnectivityManager cm =
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo netInfo = cm.getActiveNetworkInfo();
        return netInfo != null && netInfo.isConnectedOrConnecting();
    }
}