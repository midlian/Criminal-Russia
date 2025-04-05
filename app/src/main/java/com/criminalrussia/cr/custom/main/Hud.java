package criminal.russia.custom.main;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.Resources;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings;
import criminal.russia.R;
import criminal.russia.custom.animations.text.EvaporateTextView;
import criminal.russia.custom.main.tab.Tab;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Formatter;
import java.util.HashMap;

public class Hud {
    private final ConstraintLayout mInputLayout;
    private final ConstraintLayout mDateMain;
    private final ConstraintLayout mTimeMain;

    private final LinearLayout mLogoMain;
    private final ConstraintLayout mOnlineMain;

    private final ConstraintLayout mMain;

    private final EvaporateTextView mMoney;
    private int mCurrentMoney = 0;

    private final EvaporateTextView mOnline;
    private int mCurrentOnline;

    private final EvaporateTextView mTime;
    private final TextView mDate;

    private final EvaporateTextView mHp;
    private final EvaporateTextView mArmour;
    private final EvaporateTextView mEat;

    private int mCurrentHp = 0;
    private int mCurrentArmour = 0;
    private int mCurrentEat = 0;

    private final EvaporateTextView mAmmo1;
    private final EvaporateTextView mAmmo2;

    private int mCurrentAmmo1 = 0;
    private int mCurrentAmmo2 = 0;

    private final ProgressBar mHpProgress;
    private final ProgressBar mArmourProgress;
    private final ProgressBar mEatProgress;

    private final ArrayList<ImageView> mWanted;

    private final ImageView mWeapon;
    private final ImageView mBullet;

    private final ImageView mX2;

    private int resourceId;

    public Hud(Activity activity) {
        this.mInputLayout = activity.findViewById(R.id.bhud_main);

        this.mDateMain = activity.findViewById(R.id.brp_hud_date);
        this.mTimeMain = activity.findViewById(R.id.brp_hud_time);

        this.mLogoMain = activity.findViewById(R.id.brp_hud_logo);
        this.mOnlineMain = activity.findViewById(R.id.brp_hud_online);

        this.mMain = activity.findViewById(R.id.brp_hud_main);

        this.mOnline = activity.findViewById(R.id.hud_online_text);
        this.mOnline.setAnimationListener(hTextView -> {});

        this.mHp = activity.findViewById(R.id.hud_health_text);
        this.mArmour = activity.findViewById(R.id.hud_armour_text);
        this.mEat = activity.findViewById(R.id.hud_eat_text);

        this.mHp.setAnimationListener(hTextView -> {});
        this.mArmour.setAnimationListener(hTextView -> {});
        this.mEat.setAnimationListener(hTextView -> {});

        this.mMoney = activity.findViewById(R.id.hud_balance_text);
        this.mMoney.setAnimationListener(hTextView -> {});

        this.mDate = activity.findViewById(R.id.hud_date_text);

        this.mTime = activity.findViewById(R.id.hud_time_text);
        this.mTime.setAnimationListener(hTextView -> {});

        this.mAmmo1 = activity.findViewById(R.id.hud_ammo_text);
        this.mAmmo2 = activity.findViewById(R.id.hud_max_ammo_text);

        this.mAmmo1.setAnimationListener(hTextView -> {});
        this.mAmmo2.setAnimationListener(hTextView -> {});

        this.mHpProgress = activity.findViewById(R.id.hud_health_pb);
        this.mArmourProgress = activity.findViewById(R.id.hud_armour_pb);
        this.mEatProgress = activity.findViewById(R.id.hud_eat_pb);

        this.mBullet = activity.findViewById(R.id.hud_bullet);

        this.mX2 = activity.findViewById(R.id.brp_hud_x2);

        this.mWeapon = activity.findViewById(R.id.hud_fist_icon);
        this.mWeapon.setOnClickListener(view -> NvEventQueueActivity.getInstance().switchWeapon());

        // TODO -- ReWrite
        this.mWanted = new ArrayList<>();
        mWanted.add(activity.findViewById(R.id.hud_star_1));
        mWanted.add(activity.findViewById(R.id.hud_star_2));
        mWanted.add(activity.findViewById(R.id.hud_star_3));
        mWanted.add(activity.findViewById(R.id.hud_star_4));
        mWanted.add(activity.findViewById(R.id.hud_star_5));

        Animation animation = AnimationUtils.loadAnimation(activity, R.anim.btn_click);

        this.mOnlineMain.setOnClickListener(view -> {
            view.startAnimation(animation);
            NvEventQueueActivity.getInstance().showTab();
        });
        //this.mLogoMain.setOnClickListener(view -> {
        //    view.startAnimation(animation);
        //    NvEventQueueActivity.getInstance().showCaptcha();
        //});

        SetX2(false);
        Utils.HideLayout(this.mInputLayout, false);
    }

    public void show(boolean isAnimation) {
        Utils.ShowLayout(this.mInputLayout, isAnimation);

        updateHud();
    }

    public void updateHud() {
        int visibility = View.VISIBLE;

        // -- Date & time
        if (!NvEventQueueActivity.getInstance().getNativeTimeInHud())
            visibility = View.GONE;

        this.mDateMain.setVisibility(visibility);
        this.mTimeMain.setVisibility(visibility);
        UpdateTime();

        // -- Logo & online
        ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) this.mMain.getLayoutParams();

        if (!NvEventQueueActivity.getInstance().getNativeOnlineInHud()) {
            visibility = View.GONE;

            params.topMargin = NvEventQueueActivity.getInstance().getResources().getDimensionPixelSize(R.dimen._10sdp);
        } else {
            visibility = View.VISIBLE;

            params.topMargin = NvEventQueueActivity.getInstance().getResources().getDimensionPixelSize(R.dimen._3sdp);
        }

        this.mMain.setLayoutParams(params);

        this.mLogoMain.setVisibility(visibility);
        this.mOnlineMain.setVisibility(visibility);
    }

    public void SetMoney(int money) {
        if (this.mCurrentMoney == money)
            return;

        if (money > 999999999)
            return;

        mMoney.animateText(new Formatter().format("%s", new DecimalFormat("###,###,###").format(money)).toString());

        this.mCurrentMoney = money;
    }

    public void SetAmmo1(int ammo) {
        if (ammo == this.mCurrentAmmo1)
            return;

        this.mAmmo1.animateText(String.valueOf(ammo));
        this.mCurrentAmmo1 = ammo;
    }

    public void SetAmmo2(int ammo) {
        if (ammo == this.mCurrentAmmo2)
            return;

        this.mAmmo2.animateText(new Formatter().format("/%d", ammo).toString());
        this.mCurrentAmmo2 = ammo;
    }

    public void SetOnline(int online) {
        if (online == this.mCurrentOnline)
            return;

        this.mOnline.animateText(String.valueOf(online));
        this.mCurrentOnline = online;
    }

    public void SetHP(int hp) {
        if (hp == this.mCurrentHp)
            return;

        if (hp > 100)
            hp = 100;

        this.mHp.animateText(String.valueOf(hp));
        this.mHpProgress.setProgress(hp);
        this.mCurrentHp = hp;

        UpdateTime();
    }

    public void SetEat(int eat) {
        if (eat == this.mCurrentEat)
            return;

        if (eat > 100)
            eat = 100;

        this.mEat.animateText(String.valueOf(eat));
        this.mEatProgress.setProgress(eat);
        this.mCurrentEat = eat;
    }

    public void SetArmour(int armour) {
        if (armour == this.mCurrentArmour)
            return;

        if (armour > 100)
            armour = 100;

        this.mArmour.animateText(String.valueOf(armour));
        this.mArmourProgress.setProgress(armour);
        this.mCurrentArmour = armour;
    }

    public void SetX2(boolean state) {
        int visibility = View.GONE;

        if (state)
            visibility = View.VISIBLE;

        this.mX2.setVisibility(visibility);
    }

    @SuppressLint("SimpleDateFormat")
    public void UpdateTime() {
        this.mTime.animateText(new SimpleDateFormat("HH:mm").format(Calendar.getInstance().getTime()));
        this.mDate.setText(new SimpleDateFormat("dd.MM.yyyy").format(Calendar.getInstance().getTime()));
    }

    public void SetWantedLevel(int level) {
        for (int i = 0; i < this.mWanted.size(); i++)
            this.mWanted.get(i).setBackgroundResource(R.drawable.ic_star);

        if (level > 5)
            level = 5;

        for (int i = 0; i < level; i++)
            this.mWanted.get(i).setBackgroundResource(R.drawable.ic_y_star);
    }

    public void UpdateHudIcon(int id) {
        // -- Re:Init
        updateHud();

        int visible;

        if (id == 0) visible = View.GONE;
        else visible = View.VISIBLE;

        this.mAmmo1.setVisibility(visible);
        this.mAmmo2.setVisibility(visible);
        this.mBullet.setVisibility(visible);

        // ---

        Resources resources = NvEventQueueActivity.getInstance().getResources();

        String name = new Formatter().format("ic_gun_%d", id).toString();
        String pkgName = NvEventQueueActivity.getInstance().getPackageName();

        resourceId = resources.getIdentifier(name, "drawable", pkgName);

        this.mWeapon.animate().setDuration(250).translationX(100.0f).alpha(0.0f).setListener(null).start();
        this.mWeapon.animate().setDuration(0).translationX(-200.0f).alpha(0.0f).setStartDelay(250).setListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);

                mWeapon.setImageResource(resourceId);
            }
        }).start();
        this.mWeapon.animate().setDuration(250).translationX(0).alpha(1.0f).setStartDelay(250).setListener(null).start();
    }

    public ConstraintLayout getInputLayout() {
        return mInputLayout;
    }
}
