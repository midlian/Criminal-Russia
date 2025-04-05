package com.criminalrussia.launcher.activity;

import android.Manifest;
import android.annotation.SuppressLint;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Build;
import android.content.pm.PackageManager;

import android.util.Log;
import android.widget.*;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.content.ContextCompat;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import android.content.Intent;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.View.OnClickListener;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.view.animation.Animation;

import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.criminalrussia.launcher.Preferences;
import com.criminalrussia.launcher.adapter.NewsAdapter;
import com.criminalrussia.launcher.adapter.ServersAdapter;
import com.criminalrussia.launcher.model.News;
import com.criminalrussia.launcher.model.Servers;
import com.criminalrussia.launcher.other.Lists;
import criminal.russia.R;
import com.criminalrussia.launcher.utils.Utils;

import java.util.*;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import es.dmoral.toasty.Toasty;

public class MainActivity extends AppCompatActivity implements GLSurfaceView.Renderer {
    public Button play;
    public Button settings;

    private static final int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 100;
    RecyclerView recyclerNews;
    NewsAdapter newsAdapter;
    ArrayList<News> nlist;

    RecyclerView recyclerServers;
    ServersAdapter serversAdapter;
    ArrayList<Servers> slist;

    private FirebaseRemoteConfig mFirebaseRemoteConfig;

    @SuppressLint("NewApi")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        this.getExternalFilesDir("").getPath();

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
        } else if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED || checkSelfPermission(Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_DENIED) {
            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.RECORD_AUDIO}, 1000);
        } else {
            initGL();
        }
		
		Animation animation = AnimationUtils.loadAnimation(this, R.anim.button_click);
        play = (Button) findViewById(R.id.brp_launcher_play);
        settings = (Button) findViewById(R.id.brp_launcher_settings_btn);

        recyclerServers = findViewById(R.id.rvServers);
        recyclerServers.setHasFixedSize(true);
        LinearLayoutManager layoutManagerr = new LinearLayoutManager(this);
        recyclerServers.setLayoutManager(layoutManagerr);

        this.slist = Lists.serverList;
        serversAdapter = new ServersAdapter(getApplicationContext(), this.slist);
        recyclerServers.setAdapter(serversAdapter);

        recyclerNews = findViewById(R.id.rvNews);
        recyclerNews.setHasFixedSize(true);
        LinearLayoutManager layoutManager = new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false);
        recyclerNews.setLayoutManager(layoutManager);

        this.nlist = Lists.newsList;
        newsAdapter = new NewsAdapter(getApplicationContext(), this.nlist);
        recyclerNews.setAdapter(newsAdapter);
		settings.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                onClickSettings();
            }
        });

        play.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
			   onClickPlay();
            }
        });
    }

    void initGL() {
        ConstraintLayout root = findViewById(R.id.motionLayouts);

        GLSurfaceView glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setRenderer(this);
        glSurfaceView.setLayoutParams(new LinearLayout.LayoutParams(1, 1));

        root.addView(glSurfaceView);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initGL();
            }
            else {
                Toasty.error(this, "Предоставьте доступ к памяти!", 5, false);
            }
        }
    }

	public void onClickPlay() {
        if(Utils.isGameInstalled()) {
            if (Preferences.getString(this, Preferences.NICKNAME).isEmpty()) {
                Toast.makeText(this, "Для входа в игру требуется ввести ник-нейм в настройках!", Toast.LENGTH_SHORT).show();
                onClickSettings();
                return;
            } else if(!Preferences.getString(this, Preferences.NICKNAME).contains("_")) {
                Toast.makeText(this, "Ник-нейм должен содержать символ \"_\"", Toast.LENGTH_SHORT).show();
                onClickSettings();
                return;
            }

            startActivity(new Intent(getApplicationContext(), criminal.russia.GTASA.class));
            finish();
        } else {
            Utils.setDownloadType(Utils.DOWNLOAD_GAME);
            startActivity(new Intent(getApplicationContext(), PreLoadActivity.class));
        }
    }
    public void onClickSettings() {
        startActivity(new Intent(getApplicationContext(), SettingsActivity.class));
    }

	@SuppressLint("WrongConstant")
    public boolean isRecordAudioPermissionGranted() {
        if (Build.VERSION.SDK_INT < 23 || checkSelfPermission("android.permission.RECORD_AUDIO") == 0) {
            return true;
        }
        ActivityCompat.requestPermissions(this, new String[]{"android.permission.RECORD_AUDIO"}, 2);
        return false;
    }

    @SuppressLint("WrongConstant")
    public boolean isStoragePermissionGranted() {
        if (Build.VERSION.SDK_INT < 23 || checkSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE") == 0) {
            return true;
        }
        ActivityCompat.requestPermissions(this, new String[]{"android.permission.WRITE_EXTERNAL_STORAGE"}, 1);
        return false;
    }

	public void onDestroy() {
        super.onDestroy();
    }

    public void onRestart() {
        super.onRestart();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig eglConfig) {
    }


    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {}

    @Override
    public void onDrawFrame(GL10 gl) {
        gl.glClear(16384);
    }

    @Override
    public void onBackPressed() {
        Log.i("MainActivity", "onBackPressed");
    }

}