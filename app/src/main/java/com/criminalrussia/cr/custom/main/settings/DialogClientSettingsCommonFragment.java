package criminal.russia.custom.main.settings;

import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;

import androidx.annotation.Nullable;
import androidx.appcompat.widget.SwitchCompat;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;

import criminal.russia.R;
import com.nvidia.devtech.NvEventQueueActivity;

import java.util.HashMap;

public class DialogClientSettingsCommonFragment extends Fragment implements ISaveableFragment {
    private SwitchCompat mSwitchKeyboard;
    private SwitchCompat mSwitchCutout;
    private SwitchCompat mSwitchFPSCounter;
    private SwitchCompat mSwitchOutfit;
    private SwitchCompat mSwitchHpArmour;
    private SwitchCompat mSwitchRadarrect;
    private SwitchCompat mSwitchFX;
    private SwitchCompat mSwitchTimeInHud;
    private SwitchCompat mSwitchOnlineInHud;
    private SwitchCompat mSwitchNewDialogs;
    private SwitchCompat mSwitchNewHud;
    private SwitchCompat mSwitchPCMoney;
    private SwitchCompat mSwitchChatHidden;
    private SwitchCompat mSwitchSkyBox;

    private NvEventQueueActivity mContext = null;

    private View mRootView = null;

    private boolean bChangeAllowed = true;

    private SeekBar.OnSeekBarChangeListener mListenerSeekBars;
    private HashMap<ViewGroup, Drawable> mOldDrawables;

    private ViewGroup mParentView = null;

    public static DialogClientSettingsCommonFragment createInstance(String txt)
    {
        DialogClientSettingsCommonFragment fragment = new DialogClientSettingsCommonFragment();
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(null);
    }

    @Override
    public void onViewStateRestored(@Nullable Bundle savedInstanceState) {
        super.onViewStateRestored(null);
    }

    @Override
    public void onSaveInstanceState(final Bundle outState) {
        outState.putSerializable("android:support:fragments", null);
        super.onSaveInstanceState(outState);
        outState.putSerializable("android:support:fragments", null);
    }

    @Override
    public View onCreateView(LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {



                mContext = (NvEventQueueActivity) getActivity();

        mRootView = inflater.inflate(R.layout.dialog_settings_common,container,false);

        mSwitchKeyboard = mRootView.findViewById(R.id.switch_android_keyboard);
        mSwitchCutout = mRootView.findViewById(R.id.switch_cutout);
        mSwitchFPSCounter = mRootView.findViewById(R.id.switch_fps_counter);
        mSwitchHpArmour = mRootView.findViewById(R.id.switch_info_bar);
        mSwitchOutfit = mRootView.findViewById(R.id.switch_outfit_weapons);
        mSwitchRadarrect = mRootView.findViewById(R.id.switch_radar_rect);
        mSwitchTimeInHud = mRootView.findViewById(R.id.switch_time_in_hud);
        mSwitchOnlineInHud = mRootView.findViewById(R.id.switch_online_in_hud);
        mSwitchNewDialogs = mRootView.findViewById(R.id.switch_new_dialogs);
        mSwitchNewHud = mRootView.findViewById(R.id.switch_new_hud);
        mSwitchFX = mRootView.findViewById(R.id.switch_fx);
        mSwitchPCMoney = mRootView.findViewById(R.id.switch_pc_money);
        mSwitchChatHidden = mRootView.findViewById(R.id.switch_chat_hidden);
        mSwitchSkyBox = mRootView.findViewById(R.id.switch_skybox);

        getValues();
        setSeekBarListeners();

        mSwitchCutout.setOnCheckedChangeListener((compoundButton, b) -> {
            mContext.setNativeCutoutSettings(b);

            DialogFragment newFragment = new NotificationDialogFragment();
            newFragment.show(mContext.getSupportFragmentManager(), "missiles");

        });

        mSwitchKeyboard.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeKeyboardSettings(b));

        mSwitchRadarrect.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeRadarrect(b));

        mSwitchPCMoney.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativePcMoney(b));

        mSwitchChatHidden.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeChatHidden(b));

        mSwitchOutfit.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeOutfitGunsSettings(b));

        mSwitchHpArmour.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeHpArmourText(b));

        mSwitchFPSCounter.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeFpsCounterSettings(b));

        mSwitchTimeInHud.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeTimeInHud(b));

        mSwitchOnlineInHud.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeOnlineInHud(b));

        mSwitchNewDialogs.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeNewDialogs(b));

        mSwitchNewHud.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeNewHud(b));

        mSwitchFX.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeFX(b));

        mSwitchSkyBox.setOnCheckedChangeListener((compoundButton, b)
                -> mContext.setNativeSkyBox(b));

        return mRootView;
    }

    private void setSeekBarListeners()
    {
        mListenerSeekBars = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                if(bChangeAllowed)
                {
                    passValuesToNative();
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar)
            {
                mContext.onSettingsWindowSave();
            }
        };

        for(int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++)
        {
            String widgetX = "hud_element_pos_x_" + i;
            String widgetY = "hud_element_pos_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);

            if(barX != null)
            {
                barX.setOnSeekBarChangeListener(mListenerSeekBars);
            }
            if(barY != null)
            {
                barY.setOnSeekBarChangeListener(mListenerSeekBars);
            }
        }

        for(int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++)
        {
            String widgetX = "hud_element_scale_x_" + i;
            String widgetY = "hud_element_scale_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);

            if(barX != null)
            {
                barX.setOnSeekBarChangeListener(mListenerSeekBars);
            }
            if(barY != null)
            {
                barY.setOnSeekBarChangeListener(mListenerSeekBars);
            }
        }
    }

    @Override
    public void save() {

    }

    @Override
    public void getValues() {
        mSwitchKeyboard.setChecked(mContext.getNativeKeyboardSettings());
        mSwitchCutout.setChecked(mContext.getNativeCutoutSettings());
        mSwitchFPSCounter.setChecked(mContext.getNativeFpsCounterSettings());
        mSwitchHpArmour.setChecked(mContext.getNativeHpArmourText());
        mSwitchOutfit.setChecked(mContext.getNativeOutfitGunsSettings());
        mSwitchPCMoney.setChecked(mContext.getNativePcMoney());
        mSwitchChatHidden.setChecked(mContext.getNativeChatHidden());
        mSwitchRadarrect.setChecked(mContext.getNativeRadarrect());
        mSwitchTimeInHud.setChecked(mContext.getNativeTimeInHud());
        mSwitchOnlineInHud.setChecked(mContext.getNativeOnlineInHud());
        mSwitchNewDialogs.setChecked(mContext.getNativeNewDialogs());
        mSwitchNewHud.setChecked(mContext.getNativeNewHud());
        mSwitchFX.setChecked(mContext.getNativeFX());
        mSwitchSkyBox.setChecked(mContext.getNativeSkyBox());

        bChangeAllowed = false;
        for(int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++)
        {
            String widgetX = "hud_element_pos_x_" + i;
            String widgetY = "hud_element_pos_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);

            int[] pos = mContext.getNativeHudElementPosition(i);

            if(pos[0] == -1)
            {
                pos[0] = 1;
            }
            if(pos[1] == -1)
            {
                pos[1] = 1;
            }

            if(barX != null)
            {
                barX.setProgress(pos[0]);
            }
            if(barY != null)
            {
                barY.setProgress(pos[1]);
            }
        }

        for(int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++)
        {
            String widgetX = "hud_element_scale_x_" + i;
            String widgetY = "hud_element_scale_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);

            int[] pos = mContext.getNativeHudElementScale(i);

            if(pos[0] == -1)
            {
                pos[0] = 1;
            }
            if(pos[1] == -1)
            {
                pos[1] = 1;
            }

            if(barX != null && pos[0] != -1)
            {
                barX.setProgress(pos[0]);
            }
            if(barY != null && pos[1] != -1)
            {
                barY.setProgress(pos[1]);
            }
        }

        bChangeAllowed = true;
    }
    public void passValuesToNative() {
        for (int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++) {
            String widgetX = "hud_element_pos_x_" + i;
            String widgetY = "hud_element_pos_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);
            int x = -1;
            int y = -1;
            if (barX != null) {
                x = barX.getProgress();
            }
            if (barY != null) {
                y = barY.getProgress();
            }

            mContext.setNativeHudElementPosition(i, x, y);
        }

        for (int i = DialogClientSettings.mSettingsComonStart; i < DialogClientSettings.mSettingsComonEnd; i++) {
            String widgetX = "hud_element_scale_x_" + i;
            String widgetY = "hud_element_scale_y_" + i;

            int resIDX = mContext.getResources().getIdentifier(widgetX, "id", mContext.getPackageName());
            int resIDY = mContext.getResources().getIdentifier(widgetY, "id", mContext.getPackageName());

            SeekBar barX = mRootView.findViewById(resIDX);
            SeekBar barY = mRootView.findViewById(resIDY);
            int x = -1;
            int y = -1;
            if (barX != null) {
                x = barX.getProgress();
            }
            if (barY != null) {
                y = barY.getProgress();
            }

            mContext.setNativeHudElementScale(i, x, y);
        }
    }
}
