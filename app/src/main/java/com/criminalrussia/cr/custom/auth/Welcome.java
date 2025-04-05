package criminal.russia.custom.auth;

import android.app.Activity;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

public class Welcome {

    private final Activity mActivity;

    private final ConstraintLayout mInputLayout;

    private final TextView mTitle;
    private final TextView mDescription;

    private final Button mPlay;

    public Welcome(Activity activity) {
        this.mActivity = activity;
        this.mInputLayout = activity.findViewById(R.id.brp_welcome_main);

        Animation animation = AnimationUtils.loadAnimation(activity, R.anim.btn_click);

        mPlay = activity.findViewById(R.id.brp_welcome_btn);

        this.mTitle = activity.findViewById(R.id.brp_welcome_title);
        this.mDescription = activity.findViewById(R.id.brp_welcome_desc);

        this.mTitle.animate().setDuration(0).translationXBy(-2000).start();
        this.mDescription.animate().setDuration(0).translationXBy(-2000).start();
        this.mPlay.animate().setDuration(0).translationXBy(-2000).start();

        this.mPlay.setOnClickListener(v -> {
            NvEventQueueActivity.getInstance().onClickPlayInWelcomeWindow();

            Utils.HideLayout(this.mInputLayout, true);

            v.startAnimation(animation);

            this.mPlay.animate().setDuration(1500).translationXBy(-2000).start();
            this.mDescription.animate().setDuration(1500).setStartDelay(250).translationXBy(-2000).start();
            this.mTitle.animate().setDuration(1500).setStartDelay(500).translationXBy(-2000).start();
        });

        Utils.HideLayout(this.mInputLayout, false);
    }

    public void show(boolean isRegister) {
        Utils.ShowLayout(this.mInputLayout, true);

        if (isRegister) this.mTitle.setText("Закрытое Бето Тестирование Criminal Russia");
        else this.mTitle.setText("Желаем удачного тестирования!");

        this.mTitle.animate().setDuration(1500).translationXBy(2000).start();
        this.mDescription.animate().setDuration(1500).setStartDelay(250).translationXBy(2000).start();
        this.mPlay.animate().setDuration(1500).setStartDelay(500).translationXBy(2000).start();
    }

    public ConstraintLayout getInputLayout() {
        return mInputLayout;
    }

}
