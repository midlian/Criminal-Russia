package criminal.russia.custom.main;

import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;
import com.triggertrap.seekarc.SeekArc;

import java.util.Formatter;

public class Speedometer {
    private final RelativeLayout mInputLayout;

    private final TextView mSpeed;
    private final TextView mFuel;
    private final TextView mCarHP;
    private final TextView mMileage;

    private final SeekArc mSpeedLine;
    private final SeekArc mFuelLine;
    private final SeekArc mCarHPLine;

    private final ImageView mEngine;
    private final ImageView mLight;
    private final ImageView mBelt;
    private final ImageView mLock;
    private final ImageView mSpeedArrow;
    private final ImageView mSpeedFuelIcon;

    private final Drawable mPetrol;
    private final Drawable mLightning;

    public Speedometer(Activity activity) {
        this.mInputLayout = activity.findViewById(R.id.speedometer);

        this.mSpeed = activity.findViewById(R.id.speed_text);
        this.mFuel = activity.findViewById(R.id.speed_fuel_text);
        this.mCarHP = activity.findViewById(R.id.speed_car_hp_text);
        this.mMileage = activity.findViewById(R.id.speed_mileage_text);

        this.mSpeedLine = activity.findViewById(R.id.speed_line);
        this.mFuelLine = activity.findViewById(R.id.speed_fuel_line);
        this.mCarHPLine = activity.findViewById(R.id.speed_car_hp_line);

        this.mEngine = activity.findViewById(R.id.speed_engine_ico);
        this.mLight = activity.findViewById(R.id.speed_light_ico);
        this.mBelt = activity.findViewById(R.id.speed_belt_ico);
        this.mLock = activity.findViewById(R.id.speed_lock_ico);
        this.mSpeedArrow = activity.findViewById(R.id.speed_arrow);
        this.mSpeedFuelIcon = activity.findViewById(R.id.speed_fuel_icon);

        this.mPetrol = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.ico_petrol);
        this.mLightning = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.ic_lightning);

        Utils.HideLayout(mInputLayout, false);
    }

    // -- Setters
    public void setSpeed(int speed) {
        this.mSpeed.setText(String.valueOf(speed));

        float rot = (float) (-122 + speed * 0.938);
        if (rot > 121.8f)
            rot = 121.8f;

        this.mSpeedArrow.setRotation(rot);

        this.mSpeedLine.setProgress(speed);
    }

    public void setFuel(int fuel) {
        // -- Костыль, но мне поебать
        if (fuel > 1000) {
            fuel -= 1000;

            this.mFuel.setText(new Formatter().format("%d %s", fuel * 2, "%").toString());

            this.mFuelLine.setProgress(fuel * 2);
            this.mSpeedFuelIcon.setImageDrawable(this.mLightning);

            return;
        } else if (fuel > 100)
            fuel = 100;

        this.mFuel.setText(new Formatter().format("%d л", fuel).toString());

        this.mFuelLine.setProgress(fuel);
        this.mSpeedFuelIcon.setImageDrawable(this.mPetrol);
    }

    public void setCarHP(int hp) {
        if (hp > 100)
            hp = 100;

        this.mCarHP.setText(new Formatter().format("%d %s", hp, "%").toString());

        this.mCarHPLine.setProgress(hp);
    }

    public void setMileage(int mileage) {
        this.mMileage.setText(new Formatter().format("%06d", mileage).toString());
    }

    public void setEngineState(int state) {
        mEngine.setBackgroundResource(state == 1 ? R.drawable.ico_engine_on : R.drawable.ico_engine_off);
    }

    public void setLightState(int state) {
        mLight.setBackgroundResource(state == 1 ? R.drawable.ico_lights_on : R.drawable.ico_lights_off);
    }

    public void setBeltState(int state) {
        mBelt.setBackgroundResource(state == 1 ? R.drawable.ico_seatbelt_on : R.drawable.ico_seatbelt_off);
    }

    public void setLockState(int state) {
        mLock.setBackgroundResource(state == 1 ? R.drawable.ico_lock_on : R.drawable.ico_lock_off);
    }

    public RelativeLayout getInputLayout() {
        return this.mInputLayout;
    }
}
