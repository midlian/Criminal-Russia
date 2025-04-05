package com.criminalrussia.launcher.activity;

import static android.app.PendingIntent.getActivity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import criminal.russia.R;
import com.criminalrussia.launcher.Config;
import com.criminalrussia.launcher.Preferences;

import org.ini4j.Wini;

import java.io.File;
import java.io.IOException;

public class SettingsActivity extends AppCompatActivity {
    public Button back;
    public TextView forum_text;
    public Button brp_launcher_reinstall;
    public EditText nickname;
    public ImageView imageViewTelegram;
    public ImageView imageViewVK;
    public ImageView imageViewDiscord;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

		Animation animation = AnimationUtils.loadAnimation(this, R.anim.button_click);
        back = (Button) findViewById(R.id.brp_launcher_settings_back);
        brp_launcher_reinstall = (Button) findViewById(R.id.brp_launcher_reinstall);
        nickname = (EditText) findViewById(R.id.brp_launcher_settings_nick);
        imageViewTelegram = (ImageView) findViewById(R.id.imageViewTelegram);
        imageViewVK = (ImageView) findViewById(R.id.imageViewVK);
        imageViewDiscord = (ImageView) findViewById(R.id.imageViewDiscord);

        forum_text = findViewById(R.id.brp_forum);

        InitLogic();

        nickname.addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {

            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                try {
                        Wini ws = new Wini(new File(Config.path_settings));
                        ws.put("client", "name", nickname.getText());
                        ws.store();

                        Preferences.putString(getApplicationContext(), Preferences.NICKNAME, nickname.getText().toString());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

        back.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                //v.startAnimation(animation);
                startActivity(new Intent(getApplicationContext(), MainActivity.class));
                finish();
            }
        });

        brp_launcher_reinstall.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                v.startAnimation(animation);
                startActivity(new Intent(getApplicationContext(), PreLoadActivity.class));
                finish();
            }
        });

        imageViewTelegram.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                v.startAnimation(animation);
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(Config.URL_TG));
                startActivity(browserIntent);
            }
        });
        forum_text.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                v.startAnimation(animation);
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(" https://forum.criminalrussia-online.ru"));
                startActivity(browserIntent);
            }
        });
        imageViewVK.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                v.startAnimation(animation);
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(Config.URL_VK));
                startActivity(browserIntent);
            }
        });
        imageViewDiscord.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                v.startAnimation(animation);
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(Config.URL_DS));
                startActivity(browserIntent);
            }
        });
    }

    private void InitLogic() {
        try {
            Wini w = new Wini(new File(Config.path_settings));
            nickname.setText(w.get("client", "name"));
            w.store();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
} 