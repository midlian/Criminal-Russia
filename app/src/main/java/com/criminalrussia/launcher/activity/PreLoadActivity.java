package com.criminalrussia.launcher.activity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.LinearLayout;

import androidx.appcompat.app.AppCompatActivity;
import criminal.russia.R;
import com.criminalrussia.launcher.utils.Utils;

public class PreLoadActivity extends AppCompatActivity {
    public Button next;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pre_load);

        @SuppressLint({"MissingInflatedId", "LocalSuppress"}) LinearLayout dialog = findViewById(R.id.brp_launcher_pre);
        Utils.ShowView(dialog, true);

        AnimationUtils.loadAnimation(this, R.anim.button_click);
        next = findViewById(R.id.brp_launcher_pre_load_download);

        next.setOnClickListener(v -> {
            Utils.setDownloadType(Utils.DOWNLOAD_GAME);
            startActivity(new Intent(getApplicationContext(), DownloadActivity.class));
            finish();
        });
    }
} 