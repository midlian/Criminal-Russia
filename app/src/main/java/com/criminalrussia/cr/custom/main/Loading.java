package criminal.russia.custom.main;

import android.app.Activity;
import android.content.Intent;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.criminalrussia.launcher.activity.SplashActivity;

public class Loading {

    private final ConstraintLayout mInputLayout;
    private final TextView mInfo;

    private final Button mBackInLauncherBtn;

   // private final MediaPlayer mMediaPlayer;

    private boolean mIsPlayMusic = false;

    public Loading(Activity activity) {
        Animation animation = AnimationUtils.loadAnimation(activity, R.anim.btn_click);

        this.mInputLayout = activity.findViewById(R.id.brp_loading_main);

        this.mInfo = activity.findViewById(R.id.brp_loading_info);

        //this.mMediaPlayer = MediaPlayer.create(activity, R.raw.loading);
        //this.mMediaPlayer.setOnCompletionListener(mp -> restartMusic());
        //this.mMediaPlayer.start();

        this.mBackInLauncherBtn = activity.findViewById(R.id.brp_loading_back_in_launcher);
        this.mBackInLauncherBtn.setOnClickListener(view -> {
            view.startAnimation(animation);
            //stopMusic();
            activity.startActivity(new Intent(activity, SplashActivity.class));
            activity.finish();
        });
    }

    public void setText(int id) {
        if(id == 0) {
            // Utils.ShowLayout(this.getInputLayout(), true);
            this.mInfo.setText("Загрузка игры...");
        } else if(id == 2) {
            this.mInfo.setText("Подключение к серверу...");
        } else if(id == 3) {
            this.mInfo.setText("Сервер полон.. Переподключение...");
        } else if(id == 4) {
            this.mBackInLauncherBtn.setVisibility(View.VISIBLE);
            this.mInfo.setText("Игрок с данным ник-неймом находится в игре...");
        } else if(id == 5) {
            // this.mBackInLauncherBtn.setVisibility(View.VISIBLE);
            this.mInfo.setText("Сервер не отвечает.. Переподключение...");
        } else if(id == 6) {
            hide();
        }
    }

    public void hide() {
        Utils.HideLayout(this.getInputLayout(), true);
    }

    public ConstraintLayout getInputLayout() {
        return mInputLayout;
    }

}
