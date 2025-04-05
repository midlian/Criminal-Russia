package criminal.russia.custom.main;

import android.app.Activity;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.constraintlayout.widget.ConstraintLayout;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

public class SpeedometerMenu {
    private final ConstraintLayout mInputLayout;
    public static ImageView sp_engine_icon;
    private ImageView sp_closed;
    public static ImageView sp_lock_icon;
    public static ImageView sp_stroboscope_icon;
    public static ImageView sp_neon_icon;
    public static ImageView sp_light_icon;
    public static ImageView sp_parking_icon;
    public static ImageView sp_lock_text;
    public static LinearLayout sp_linearLayoutNeon;
    public static LinearLayout sp_linearLayoutLock;
    public static LinearLayout sp_linearLayoutEngine;
    public static LinearLayout sp_linearLayoutPark;
    public static LinearLayout sp_linearLayoutStrob;
    public static LinearLayout sp_linearLayoutLight;
    private Activity mactivity;

    public SpeedometerMenu(Activity activity) {
        this.mInputLayout = activity.findViewById(R.id.speedometerMenu);

        this.sp_engine_icon = activity.findViewById(R.id.sp_engine_icon);
        this.sp_closed = activity.findViewById(R.id.sp_closed);
        mactivity = activity;


        sp_closed.setClickable(true);
        sp_closed.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                closedMenu();
            }
        });

        this.sp_engine_icon = activity.findViewById(R.id.sp_engine_icon);
        this.sp_linearLayoutEngine = activity.findViewById(R.id.sp_linearLayoutEngine);
        sp_linearLayoutEngine.setClickable(true);
        sp_linearLayoutEngine.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/en");
                closedMenu();
            }
        });

        this.sp_lock_icon = activity.findViewById(R.id.sp_lock_icon);
        this.sp_linearLayoutLock = activity.findViewById(R.id.sp_linearLayoutLock);
        sp_linearLayoutLock.setClickable(true);
        sp_linearLayoutLock.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/lock");
                closedMenu();
            }
        });


        this.sp_neon_icon = activity.findViewById(R.id.sp_neon_icon);
        this.sp_linearLayoutNeon = activity.findViewById(R.id.sp_linearLayoutNeon);
        sp_linearLayoutNeon.setClickable(true);
        sp_linearLayoutNeon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/neon");
                closedMenu();
            }
        });

        this.sp_parking_icon = activity.findViewById(R.id.sp_parking_icon);
        this.sp_linearLayoutPark = activity.findViewById(R.id.sp_linearLayoutPark);
        sp_linearLayoutPark.setClickable(true);
        sp_linearLayoutPark.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/park");
                closedMenu();
            }
        });


        this.sp_light_icon = activity.findViewById(R.id.sp_light_icon);
        this.sp_linearLayoutLight = activity.findViewById(R.id.sp_linearLayoutLight);
        sp_linearLayoutLight.setClickable(true);
        sp_linearLayoutLight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/llight");
                closedMenu();
            }
        });

        this.sp_stroboscope_icon = activity.findViewById(R.id.sp_stroboscope_icon);
        this.sp_linearLayoutStrob = activity.findViewById(R.id.sp_linearLayoutStrab);
        sp_linearLayoutStrob.setClickable(true);
        sp_linearLayoutStrob.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NvEventQueueActivity.getInstance().sendClick("/strobs");
                closedMenu();
            }
        });


        Utils.HideLayout(mInputLayout, false);
    }


    public static void setEngine(int state) {
        sp_engine_icon.setImageResource(state == 1 ? R.drawable.sp_engine_icon_on : R.drawable.sp_engine_icon);
    }

    public static void setLock(int state) {
        sp_lock_icon.setImageResource(state == 1 ? R.drawable.sp_lock_icon_on : R.drawable.sp_lock_icon);
    }

    public void setLight(int state) {
        sp_light_icon.setImageResource(state == 1 ? R.drawable.sp_light_icon_on : R.drawable.sp_light_icon);
    }

    public static void setParking(int state) {
        sp_parking_icon.setImageResource(state == 1 ? R.drawable.sp_parking_icon_on : R.drawable.sp_parking_icon);
    }

    public static void setStrob(int state) {
        if(state == 1) sp_stroboscope_icon.setImageResource(R.drawable.sp_stroboscope_icon);
        if(state == 2) sp_stroboscope_icon.setImageResource(R.drawable.sp_stroboscope_icon_off);
        if(state == 0) sp_stroboscope_icon.setImageResource(R.drawable.sp_stroboscope_icon_grey);
    }

    public static void setNeon(int state) {
        if(state == 0) sp_neon_icon.setImageResource(R.drawable.sp_neon_icon_grey);
        if(state == 1) sp_neon_icon.setImageResource(R.drawable.sp_neon_icon);
        if(state == 2) sp_neon_icon.setImageResource(R.drawable.sp_neon_icon_off);
    }

    public void show() {
        Utils.ShowLayout(mInputLayout, true);
        NvEventQueueActivity.getInstance().hideHud(true);

        Speedometer mSpeedometer = new Speedometer(mactivity);
        Utils.HideLayout(mSpeedometer.getInputLayout(), true);
    }

    public void closedMenu() {
        Utils.HideLayout(mInputLayout, true);

        Hud mHud = new Hud(mactivity);
        mHud.show(true);

        Speedometer mSpeedometer = new Speedometer(mactivity);
        Utils.ShowLayout(mSpeedometer.getInputLayout(), true);

//        NvEventQueueActivity.statusButtonMenu(true);

        NvEventQueueActivity.getInstance().showHud(true);
    }




    public ConstraintLayout getInputLayout() {
        return this.mInputLayout;
    }
}
