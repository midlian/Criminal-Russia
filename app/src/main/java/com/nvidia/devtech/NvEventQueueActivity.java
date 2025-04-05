//----------------------------------------------------------------------------------
// File:            libs\src\com\nvidia\devtech\NvEventQueueActivity.java
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
package com.nvidia.devtech;

import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.TypedValue;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewParent;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings;
import criminal.russia.R;
import criminal.russia.custom.auth.SpawnSelector;
import criminal.russia.custom.auth.Welcome;
import criminal.russia.custom.bussines.AZS;
import criminal.russia.custom.main.Hud;
import criminal.russia.custom.main.Loading;
import criminal.russia.custom.main.Numbers;
import criminal.russia.custom.main.Speedometer;
import criminal.russia.custom.main.SpeedometerMenu;
import criminal.russia.custom.main.antibot.CaptchaMain;
import criminal.russia.custom.main.dialogs.Dialog;
import criminal.russia.custom.main.quests.Quests;
import criminal.russia.custom.main.quests.QuestsInfo;
import criminal.russia.custom.main.settings.DialogClientSettings;
import criminal.russia.custom.main.tab.Tab;
import criminal.russia.custom.util.Utils;

import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;

public abstract class NvEventQueueActivity
        extends AppCompatActivity
        implements SensorEventListener, InputManager.InputListener, View.OnTouchListener, HeightProvider.HeightListener {
    protected Handler handler = null;

    private int SwapBufferSkip = 0;
    private FirebaseRemoteConfig mFirebaseRemoteConfig;
    protected boolean paused = false;

    protected boolean wantsMultitouch = false;

    protected boolean supportPauseResume = true;
    protected boolean ResumeEventDone = false;

    //Меню
    public static ImageView imageViewBtnOpen;
    public static ImageView imageViewBtnDonateOpen;
    public static ImageView rpc_gps;
    public static ImageView rpc_greenzone;
    public static boolean rpc_greenzone_status;
    public static boolean rpc_gps_status;
    ConstraintLayout constraintlayoutMenu;


    //accelerometer related
    protected boolean wantsAccelerometer = false;
    protected SensorManager mSensorManager = null;
    protected ClipboardManager mClipboardManager = null;
    protected int mSensorDelay = SensorManager.SENSOR_DELAY_GAME; //other options: SensorManager.SENSOR_DELAY_FASTEST, SensorManager.SENSOR_DELAY_NORMAL and SensorManager.SENSOR_DELAY_UI
    protected Display display = null;

    private static final int EGL_RENDERABLE_TYPE = 0x3040;
    private static final int EGL_OPENGL_ES2_BIT = 0x0004;
    private static final int EGL_OPENGL_ES3_BIT = 64;
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    EGL10 egl = null;
    GL11 gl = null;

    private boolean ranInit = false;
    protected EGLSurface eglSurface = null;
    protected EGLDisplay eglDisplay = null;
    protected EGLContext eglContext = null;
    protected EGLConfig eglConfig = null;

    protected SurfaceHolder cachedSurfaceHolder = null;
    private int surfaceWidth = 0;
    private int surfaceHeight = 0;

    private int fixedWidth = 0;
    private int fixedHeight = 0;
    private boolean HasGLExtensions = false;
    private String glVendor = null;
    private String glExtensions = null;
    private String glRenderer = null;
    private String glVersion = null;
    private boolean GameIsFocused = false;
    private boolean viewIsActive = false;

    private FrameLayout mRootFrame = null;
    private SurfaceView mSurfaceView = null;

    private static NvEventQueueActivity instance = null;

    private InputManager mInputManager = null;
    private HeightProvider mHeightProvider = null;
    private DialogClientSettings mDialogClientSettings = null;

    private Hud mHud = null;
    private Dialog mDialog = null;
    private Speedometer mSpeedometer = null;

    //
    private Welcome mWelcome = null;
    private CaptchaMain mCaptcha = null;
    private Loading mLoading = null;
    private SpawnSelector mSpawnSelector = null;
    public Quests mQuests = null;
    public QuestsInfo mQuestsInfo = null;
    private AZS mAZS = null;
    private Numbers mNumbers = null;
    //
    public static SpeedometerMenu mMenuSpeedometer = null;
    private Tab mTab = null;

    private int startGame = 0;

    /* *
     * Helper function to select fixed window size.
     * */
    public void setFixedSize(int fw, int fh)
    {
        fixedWidth = fw;
        fixedHeight = fh;
    }

    public native void onEventBackPressed();


    public native void onSettingsWindowSave();
    public native void onSettingsWindowDefaults(int category);

    public native void setNativeCutoutSettings(boolean b);
    public native void setNativeKeyboardSettings(boolean b);
    public native void setNativeFpsCounterSettings(boolean b);
    public native void setNativeOutfitGunsSettings(boolean b);
    public native void setNativeHpArmourText(boolean b);
    public native void setNativeRadarrect(boolean b);
    public native void setNativePcMoney(boolean b);
    public native void setNativeSkyBox(boolean b);
    public native void setNativeKillList(boolean b);
    public native void setNativeFX(boolean b);
    public native void setNativeTimeInHud(boolean b);
    public native void setNativeOnlineInHud(boolean b);
    public native void setNativeNewDialogs(boolean b);
    public native void setNativeNewHud(boolean b);
    public native void setNativeChatHidden(boolean b);

    public static native void setTogleHud(boolean b);

    public native boolean getNativeCutoutSettings();
    public native boolean getNativeKeyboardSettings();
    public native boolean getNativeFpsCounterSettings();
    public native boolean getNativeOutfitGunsSettings();
    public native boolean getNativeHpArmourText();
    public native boolean getNativeRadarrect();
    public native boolean getNativePcMoney();
    public native boolean getNativeSkyBox();
    public native boolean getNativeKillList();
    public native boolean getNativeFX();
    public native boolean getNativeTimeInHud();
    public native boolean getNativeOnlineInHud();
    public native boolean getNativeNewDialogs();
    public native boolean getNativeNewHud();
    public native boolean getNativeChatHidden();

    public native void setNativeHudElementColor(int id, int a, int r, int g, int b);
    public native byte[] getNativeHudElementColor(int id);

    public native void setNativeHudElementPosition(int id, int x, int y);
    public native int[] getNativeHudElementPosition(int id);

    public native void setNativeHudElementScale(int id, int x, int y);
    public native int[] getNativeHudElementScale(int id);

    public native void setNativeWidgetPositionAndScale(int id, int x, int y, int scale);
    public native int[] getNativeWidgetPositionAndScale(int id);

    public String getHudElementColor(int id)
    {
        byte[] color = getNativeHudElementColor(id);
        String str = null;
        try {
            str = new String(color, "windows-1251");
        }
        catch(UnsupportedEncodingException e)
        {

        }

        return str;

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        onEventBackPressed();
    }

    private int mUseFullscreen = 0;

    private void processCutout()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
        {
            if(mUseFullscreen == 1)
            {
                getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            }
        }
    }

    public void setUseFullscreen(int b)
    {
        mUseFullscreen = b;
    }

    public void showClientSettings()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(mDialogClientSettings != null)
                {
                    mDialogClientSettings = null;
                }
                mDialogClientSettings = new DialogClientSettings();
                mDialogClientSettings.show(getSupportFragmentManager(), "test");
            }
        });
    }

    public void hideSystemUI() {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY



        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

    @Override
    public boolean onTouch(View view, MotionEvent event)
    {
        if(view == mRootFrame)
        {
            if (wantsMultitouch)
            {
                int x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
                // marshal up the data.
                int numEvents = event.getPointerCount();
                for (int i=0; i<numEvents; i++)
                {
                    // only use pointers 0 and 1, 2, 3
                    int pointerId = event.getPointerId(i);
                    if (pointerId == 0)
                    {
                        x1 = (int)event.getX(i);
                        y1 = (int)event.getY(i);
                    }
                    else if (pointerId == 1)
                    {
                        x2 = (int)event.getX(i);
                        y2 = (int)event.getY(i);
                    }
                    else if (pointerId == 2)
                    {
                        x3 = (int)event.getX(i);
                        y3 = (int)event.getY(i);
                    }
                }

                int pointerId = event.getPointerId(event.getActionIndex());
                int action = event.getActionMasked();
                customMultiTouchEvent(action, pointerId, x1, y1, x2, y2, x3, y3);
                //Log.i("redcat", "X1: " + x1 + " | X2" + x2 + " | X3" + x3);
            }
            else // old style input.*/
            {
                touchEvent(event.getAction(), (int)event.getX(), (int)event.getY(), event);
            }
        }
        return true;
    }

    private native void onNativeHeightChanged(int orientation, int height);

    @Override
    public void onHeightChanged(int orientation, int height)
    {
        if(this.mInputManager != null)
            this.mInputManager.onHeightChanged(height);

        if(this.mDialog != null)
            this.mDialog.onHeightChanged(height);

        //if(orientation == Configuration.ORIENTATION_LANDSCAPE)
        //    onNativeHeightChanged(orientation, height + findViewById(R.id.main_input).getHeight());
    }

    /**
     * Helper class used to pass raw data around.
     */
    public class RawData
    {
        /** The actual data bytes. */
        public byte[] data;
        /** The length of the data. */
        public int length;
    }
    /**
     * Helper class used to pass a raw texture around.
     */
    public class RawTexture extends RawData
    {
        /** The width of the texture. */
        public int width;
        /** The height of the texture. */
        public int height;
    }

    /**
     * Helper function to load a file into a {@link RawData} object.
     * It'll first try loading the file from "/data/" and if the file doesn't
     * exist there, it'll try loading it from the assets directory inside the
     * .APK file. This is to allow the files inside the apk to be overridden
     * or not be part of the .APK at all during the development phase of the
     * application, decreasing the size needed to be transmitted to the device
     * between changes to the code.
     *
     * @param filename The file to load.
     * @return The RawData object representing the file's fully loaded data,
     * or null if loading failed.
     */
    public RawData loadFile(String filename)
    {
        InputStream is = null;
        RawData ret = new RawData();
        try {
            try
            {
                is = new FileInputStream("/data/" + filename);
            }
            catch (Exception e)
            {
                try
                {
                    is = getAssets().open(filename);
                }
                catch (Exception e2)
                {
                }
            }
            int size = is.available();
            ret.length = size;
            ret.data = new byte[size];
            is.read(ret.data);
        }
        catch (IOException ioe)
        {
        }
        finally
        {
            if (is != null)
            {
                try { is.close(); } catch (Exception e) {}
            }
        }
        return ret;
    }

    /**
     * Helper function to load a texture file into a {@link RawTexture} object.
     * It'll first try loading the texture from "/data/" and if the file doesn't
     * exist there, it'll try loading it from the assets directory inside the
     * .APK file. This is to allow the files inside the apk to be overridden
     * or not be part of the .APK at all during the development phase of the
     * application, decreasing the size needed to be transmitted to the device
     * between changes to the code.
     *
     * The texture data will be flipped and bit-twiddled to fit being loaded directly
     * into OpenGL ES via the glTexImage2D call.
     *
     * @param filename The file to load.
     * @return The RawTexture object representing the texture's fully loaded data,
     * or null if loading failed.
     */
    public RawTexture loadTexture(String filename)
    {
        RawTexture ret = new RawTexture();
        try {
            InputStream is = null;
            try
            {
                is = new FileInputStream("/data/" + filename);
            }
            catch (Exception e)
            {
                try
                {
                    is = getAssets().open(filename);
                }
                catch (Exception e2)
                {
                }
            }

            Bitmap bmp = BitmapFactory.decodeStream(is);
            ret.width = bmp.getWidth();
            ret.height = bmp.getHeight();
            int[] pixels = new int[bmp.getWidth() * bmp.getHeight()];
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, 0, bmp.getWidth(), bmp.getHeight());

            // Flip texture
            int[] tmp = new int[bmp.getWidth()];
            final int w = bmp.getWidth();
            final int h = bmp.getHeight();
            for (int i = 0; i < h>>1; i++)
            {
                System.arraycopy(pixels, i*w, tmp, 0, w);
                System.arraycopy(pixels, (h-1-i)*w, pixels, i*w, w);
                System.arraycopy(tmp, 0, pixels, (h-1-i)*w, w);
            }

            // Convert from ARGB -> RGBA and put into the byte array
            ret.length = pixels.length * 4;
            ret.data = new byte[ret.length];
            int pos = 0;
            int bpos = 0;
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++, pos++)
                {
                    int p = pixels[pos];
                    ret.data[bpos++] = (byte) ((p>>16)&0xff);
                    ret.data[bpos++] = (byte) ((p>> 8)&0xff);
                    ret.data[bpos++] = (byte) ((p>> 0)&0xff);
                    ret.data[bpos++] = (byte) ((p>>24)&0xff);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return ret;
    }

    /**
     * Function called when app requests accelerometer events.
     * Applications need/should NOT overide this function - it will provide
     * accelerometer events into the event queue that is accessible
     * via the calls in nv_event.h
     *
     * @param values0: values[0] passed to onSensorChanged(). For accelerometer: Acceleration minus Gx on the x-axis.
     * @param values1: values[1] passed to onSensorChanged(). For accelerometer: Acceleration minus Gy on the y-axis.
     * @param values2: values[2] passed to onSensorChanged(). For accelerometer: Acceleration minus Gz on the z-axis.
     * @return True if the event was handled.
     */
    public native boolean accelerometerEvent(float values0, float values1, float values2);

    /**
     * The following indented function implementations are defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    public native void cleanup();
    public native boolean init(boolean z);
    public native boolean initGL();
    public native void initSAMP();
    public native void setWindowSize(int w, int h);
    public native void quitAndWait();
    public native void postCleanup();

    public native void imeClosed();

    public native void lowMemoryEvent(); // TODO: implement this
    public native boolean processTouchpadAsPointer(ViewParent viewParent, boolean z);
    public native void notifyChange(String str, int i);
    public native void changeConnection(boolean z);

    public native void pauseEvent();
    public native void resumeEvent();
    public native boolean touchEvent(int action, int x, int y, MotionEvent event);
    public native boolean multiTouchEvent(int action, int count,
                                          int x0, int y0, int x1, int y1, MotionEvent event);
    public native boolean keyEvent(int action, int keycode, int unicodeChar, int metaState, KeyEvent event);

    public native boolean customMultiTouchEvent(int action, int count, int x1, int y1, int x2, int y2, int x3, int y3);
    /**
     * END indented block, see in comment at top of block
     */

    /**
     * Declaration for function defined in nv_time/nv_time.cpp
     * It initializes and returns time through Nvidia's egl extension for time.
     * It is useful while debugging the demo using PerfHUD.
     *
     * @see: nv_time/nv_time.cpp for implementation details.
     */
    public native void nvAcquireTimeExtension();
    public native long nvGetSystemTime();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        System.out.println("**** onCreate");
        super.onCreate(savedInstanceState);

        instance = this;

        if(supportPauseResume)
        {
            System.out.println("Calling init(false)");
            init(false);
            System.out.println("Calling initSAMP");
            initSAMP();
            System.out.println("Called");
        }
        handler = new Handler();
        if(wantsAccelerometer && (mSensorManager == null)) {
            mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        }

        mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);

        NvUtil.getInstance().setActivity(this);
        NvAPKFileHelper.getInstance().setContext(this);

        display = ((WindowManager)this.getSystemService(WINDOW_SERVICE)).getDefaultDisplay();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
        {
            // TODO: cutout
            //getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        systemInit();

        hideSystemUI();

        //((TextView)findViewById(R.id.main_version_text)).setText(BuildConfig.VERSION_NAME);

        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int i) {
                if ((i & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    hideSystemUI();
                } else {
                    // TODO: The system bars are NOT visible. Make any desired
                    // adjustments to your UI, such as hiding the action bar or
                    // other navigational controls.
                }

            }
        });

        processCutout();
    }



    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    public void onWindowFocusChanged(boolean hasFocus)
    {
        if(mDialogClientSettings != null)
        {
            if(mDialogClientSettings.getDialog() != null) {
                if(mDialogClientSettings.getDialog().isShowing())
                {
                    hideSystemUI();
                    super.onWindowFocusChanged(hasFocus);
                    return;
                }
            }
        }
        if (ResumeEventDone && viewIsActive && !paused)
        {
            if (GameIsFocused && !hasFocus)
            {
                if(mInputManager != null)
                {
                    if(!mInputManager.IsShowing())
                    {
                        pauseEvent();
                    }
                }
                else
                {
                    pauseEvent();
                }
            }
            else if (!GameIsFocused && hasFocus)
            {
                resumeEvent();
            }
            GameIsFocused = hasFocus;
        }
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus)
        {
            hideSystemUI();
        }
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    protected void onResume()
    {
        System.out.println("**** onResume");
        super.onResume();
        if(mSensorManager != null)
            mSensorManager.registerListener(
                    this,
                    mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                    mSensorDelay);
        paused = false;

        if(mHeightProvider != null)
            mHeightProvider.init(mRootFrame);

        if (viewIsActive && ResumeEventDone)
        {
            resumeEvent();
            if (cachedSurfaceHolder != null)
            {
                cachedSurfaceHolder.setKeepScreenOn(true);
            }
        }
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    protected void onRestart()
    {
        System.out.println("**** onRestart");
        super.onRestart();
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    protected void onPause()
    {
        System.out.println("**** onPause");
        super.onPause();
        paused = true;

        if (ResumeEventDone)
        {
            System.out.println("java is invoking pauseEvent(), this will block until\nthe client calls NVEventPauseProcessed");
            pauseEvent();
            System.out.println("pauseEvent() returned");
        }
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    protected void onStop()
    {
        System.out.println("**** onStop");
        if(mSensorManager != null)
            mSensorManager.unregisterListener(this);
        super.onStop();
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application should *probably* not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     *
     * NOTE: An application may need to override this if the app has an
     *       in-process instance of the Service class and the native side wants to
     *       keep running. The app would want to execute the content of the
     *       if(supportPauseResume) clause when it is time to exit.
     */
    @Override
    public void onDestroy()
    {
        System.out.println("**** onDestroy");
        if(supportPauseResume)
        {
            quitAndWait();
            finish();
        }

        if(this.mHeightProvider != null)
            this.mHeightProvider.dismiss();

        super.onDestroy();
        systemCleanup();
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */

    public void mSleep(long milis) {
        try {
            Thread.sleep(milis);
        } catch (InterruptedException e) {
        }
    }

    public void DoResumeEvent()
    {
        new Thread(new Runnable() {
            public void run() {
                while (NvEventQueueActivity.this.cachedSurfaceHolder == null)
                {
                    NvEventQueueActivity.this.mSleep(1000);
                }
                System.out.println("Call from DoResumeEvent");
                NvEventQueueActivity.this.resumeEvent();
                NvEventQueueActivity.this.ResumeEventDone = true;
            }
        }).start();
    }

    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Auto-generated method stub
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    public void onSensorChanged(SensorEvent event) {
        // Auto-generated method stub
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
        {
            float roll = 0.0f;
            float pitch = 0.0f;
            switch (this.display.getRotation()) {
                case 0:
                    roll = -event.values[0];
                    pitch = event.values[1];
                    break;
                case 1:
                    roll = event.values[1];
                    pitch = event.values[0];
                    break;
                case 2:
                    roll = event.values[0];
                    pitch = event.values[1];
                    break;
                case 3:
                    roll = -event.values[1];
                    pitch = event.values[0];
                    break;
            }
            accelerometerEvent(roll, pitch, event.values[2]);
        }
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        return super.onTouchEvent(event);
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        boolean ret = false;

        if(keyCode == KeyEvent.KEYCODE_BACK)
        {
            onEventBackPressed();
        }

        if (keyCode == 24 || keyCode == 25) {
            return super.onKeyDown(keyCode, event);
        }
        if (keyCode == 89 || keyCode == 85 || keyCode == 90) {
            return false;
        }
        if (!(keyCode == 82 || keyCode == 4)) {
            ret = super.onKeyDown(keyCode, event);
        }
        if (!ret) {
            ret = keyEvent(event.getAction(), keyCode, event.getUnicodeChar(), event.getMetaState(), event);
        }
        return ret;
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        if (keyCode == 115 && Build.VERSION.SDK_INT >= 11)
        {
            boolean capsLockOn = event.isCapsLockOn();
            keyEvent(capsLockOn ? 3 : 4, 115, 0, 0, event);
        }
        if (keyCode == 89 || keyCode == 85 || keyCode == 90)
        {
            return false;
        }
        boolean onKeyUp = super.onKeyUp(keyCode, event);
        if (onKeyUp)
        {
            return onKeyUp;
        }
        return keyEvent(event.getAction(), keyCode, event.getUnicodeChar(), event.getMetaState(), event);
    }

    public boolean InitEGLAndGLES2(int i)
    {
        System.out.println("lnitEGLAndGLES2");
        if (cachedSurfaceHolder == null)
        {
            System.out.println("InitEGLAndGLES2 failed, cachedSurfaceHoIder is null");
            return false;
        }

        boolean eglInitialized = true;
        if (eglContext == null)
        {
            eglInitialized = initEGL();
        }
        if (eglInitialized)
        {
            System.out.println("Should we create a surface?");
            if (!viewIsActive)
            {
                System.out.println("Yes! Calling create surface");
                createEGLSurface(this.cachedSurfaceHolder);
                System.out.println("Done creating surface");
            }
            viewIsActive = true;
            SwapBufferSkip = 1;
        }
        else
        {
            System.out.println("initEGlAndGLES2 failed, core EGL init failure");
            return false;
        }

        return true;
    }

    /**
     * Implementation function: defined in libnvevent.a
     * The application does not and should not overide this; nv_event handles this internally
     * And remaps as needed into the native calls exposed by nv_event.h
     */

    private native void onInputEnd(byte[] str);
    @Override
    public void OnInputEnd(String str)
    {
        byte[] toReturn = null;
        try
        {
            toReturn = str.getBytes("windows-1251");
        }
        catch(UnsupportedEncodingException e)
        {

        }

        onInputEnd(toReturn);
    }

    public SurfaceView GetSurfaceView()
    {
        return mSurfaceView;
    }

    protected boolean systemInit()
    {
        final NvEventQueueActivity act = this;

        System.out.println("ln systemInit");

        setContentView(R.layout.main_render_screen);

        SurfaceView view = findViewById(R.id.main_sv);

        mSurfaceView = view;
        mRootFrame = findViewById(R.id.main_fl_root);

        mDialog = new Dialog(this);
        mHud = new Hud(this);
        mLoading = new Loading(this);
        mTab = new Tab(this);
        mCaptcha = new CaptchaMain(this);
        mSpeedometer = new Speedometer(this);
        mMenuSpeedometer = new SpeedometerMenu(this);
        mSpawnSelector = new SpawnSelector(this);

        SurfaceHolder holder = view.getHolder();
        holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
        holder.setKeepScreenOn(true);

        view.setFocusable(true);
        view.setFocusableInTouchMode(true);

        mRootFrame.setOnTouchListener(this);

        mInputManager = new InputManager(this);
        mHeightProvider = new HeightProvider(this).init(mRootFrame).setHeightListener(this);

        DoResumeEvent();

        holder.addCallback(new Callback()
        {
            // @Override
            public void surfaceCreated(SurfaceHolder holder)
            {
                System.out.println("systemInit.surfaceCreated");
                @SuppressWarnings("unused")
                boolean firstRun = cachedSurfaceHolder == null;
                cachedSurfaceHolder = holder;

                if (fixedWidth!=0 && fixedHeight!=0)
                {
                    System.out.println("Setting fixed window size");
                    holder.setFixedSize(fixedWidth, fixedHeight);
                }

                ranInit = true;
                if(!supportPauseResume && !init(true))
                {
                    handler.post(new Runnable()
                                 {
                                     public void run()
                                     {
                                         new AlertDialog.Builder(act)
                                                 .setMessage("Application initialization failed. The application will exit.")
                                                 .setPositiveButton("Ok",
                                                         new DialogInterface.OnClickListener ()
                                                         {
                                                             public void onClick(DialogInterface i, int a)
                                                             {
                                                                 finish();
                                                             }
                                                         }
                                                 )
                                                 .setCancelable(false)
                                                 .show();
                                     }
                                 }
                    );
                }

                if (!firstRun && ResumeEventDone)
                {
                    System.out.println("entering resumeEvent");
                    resumeEvent();
                    System.out.println("returned from resumeEvent");
                }

                setWindowSize(surfaceWidth, surfaceHeight);
                // initGL();
            }

            /**
             * Implementation function: defined in libnvevent.a
             * The application does not and should not overide this; nv_event handles this internally
             * And remaps as needed into the native calls exposed by nv_event.h
             */
            // @Override
            public void surfaceChanged(SurfaceHolder holder, int format,
                                       int width, int height)
            {
                System.out.println("Surface changed: " + width + ", " + height);

                surfaceWidth = width;
                surfaceHeight = height;

                setWindowSize(surfaceWidth, surfaceHeight);
                // initGL();
            }

            /**
             * Implementation function: defined in libnvevent.a
             * The application does not and should not overide this; nv_event handles this internally
             * And remaps as needed into the native calls exposed by nv_event.h
             */
            // @Override
            public void surfaceDestroyed(SurfaceHolder holder)
            {
                System.out.println("systemInit.surfaceDestroyed");
                viewIsActive = false;
                pauseEvent();
                destroyEGLSurface();
            }
        });
        return true;
    }



    /** The number of bits requested for the red component */
    protected int redSize     = 5;
    /** The number of bits requested for the green component */
    protected int greenSize   = 6;
    /** The number of bits requested for the blue component */
    protected int blueSize    = 5;
    /** The number of bits requested for the alpha component */
    protected int alphaSize   = 0;
    /** The number of bits requested for the stencil component */
    protected int stencilSize = 0;
    /** The number of bits requested for the depth component */
    protected int depthSize   = 16;

    /** Attributes used when selecting the EGLConfig */
    protected int[] configAttrs = null;
    /** Attributes used when creating the context */
    protected int[] contextAttrs = null;

    /**
     * Called to initialize EGL. This function should not be called by the inheriting
     * activity, but can be overridden if needed.
     *
     * @return True if successful
     */
    protected boolean initEGL()
    {
        if (configAttrs == null)
            configAttrs = new int[] {EGL10.EGL_NONE};
        int[] oldConf = configAttrs;

        configAttrs = new int[3 + oldConf.length-1];
        int i = 0;
        for (i = 0; i < oldConf.length-1; i++)
            configAttrs[i] = oldConf[i];
        configAttrs[i++] = EGL_RENDERABLE_TYPE;
        configAttrs[i++] = EGL_OPENGL_ES2_BIT;
        configAttrs[i++] = EGL10.EGL_NONE;

        contextAttrs = new int[]
                {
                        EGL_CONTEXT_CLIENT_VERSION, 2,
                        EGL10.EGL_NONE
                };

        if (configAttrs == null)
            configAttrs = new int[] {EGL10.EGL_NONE};
        int[] oldConfES2 = configAttrs;

        configAttrs = new int[13 + oldConfES2.length-1];
        for (i = 0; i < oldConfES2.length-1; i++)
            configAttrs[i] = oldConfES2[i];
        configAttrs[i++] = EGL10.EGL_RED_SIZE;
        configAttrs[i++] = redSize;
        configAttrs[i++] = EGL10.EGL_GREEN_SIZE;
        configAttrs[i++] = greenSize;
        configAttrs[i++] = EGL10.EGL_BLUE_SIZE;
        configAttrs[i++] = blueSize;
        configAttrs[i++] = EGL10.EGL_ALPHA_SIZE;
        configAttrs[i++] = alphaSize;
        configAttrs[i++] = EGL10.EGL_STENCIL_SIZE;
        configAttrs[i++] = stencilSize;
        configAttrs[i++] = EGL10.EGL_DEPTH_SIZE;
        configAttrs[i++] = depthSize;
        configAttrs[i++] = EGL10.EGL_NONE;

        egl = (EGL10) EGLContext.getEGL();
        egl.eglGetError();
        eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        System.out.println("eglDisplay: " + eglDisplay + ", errr: " + egl.eglGetError());
        int[] version = new int[2];
        boolean ret = egl.eglInitialize(eglDisplay, version);
        System.out.println("EGLInitialize returned: " + ret);
        if (!ret)
        {
            return false;
        }
        int eglErr = egl.eglGetError();
        if (eglErr != EGL10.EGL_SUCCESS)
            return false;
        System.out.println("eglInitialize err: " + eglErr);

        final EGLConfig[] config = new EGLConfig[20];
        int num_configs[] = new int[1];
        egl.eglChooseConfig(eglDisplay, configAttrs, config, config.length, num_configs);
        System.out.println("eglChooseConfig err: " + egl.eglGetError());

        int score = 1<<24; // to make sure even worst score is better than this, like 8888 when request 565...
        int val[] = new int[1];
        for (i = 0; i < num_configs[0]; i++)
        {
            boolean cont = true;
            int currScore = 0;
            int r, g, b, a, d, s;
            for (int j = 0; j < (oldConf.length-1)>>1; j++)
            {
                egl.eglGetConfigAttrib(eglDisplay, config[i], configAttrs[j*2], val);
                if ((val[0] & configAttrs[j*2+1]) != configAttrs[j*2+1])
                {
                    cont = false; // Doesn't match the "must have" configs
                    break;
                }
            }
            if (!cont)
                continue;
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_RED_SIZE, val); r = val[0];
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_GREEN_SIZE, val); g = val[0];
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_BLUE_SIZE, val); b = val[0];
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_ALPHA_SIZE, val); a = val[0];
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_DEPTH_SIZE, val); d = val[0];
            egl.eglGetConfigAttrib(eglDisplay, config[i], EGL10.EGL_STENCIL_SIZE, val); s = val[0];

            System.out.println(">>> EGL Config ["+i+"] R"+r+"G"+g+"B"+b+"A"+a+" D"+d+"S"+s);

            currScore = (Math.abs(r - redSize) + Math.abs(g - greenSize) + Math.abs(b - blueSize) + Math.abs(a - alphaSize)) << 16;
            currScore += Math.abs(d - depthSize) << 8;
            currScore += Math.abs(s - stencilSize);

            if (currScore < score)
            {
                System.out.println("--------------------------");
                System.out.println("New config chosen: " + i);
                for (int j = 0; j < (configAttrs.length-1)>>1; j++)
                {
                    egl.eglGetConfigAttrib(eglDisplay, config[i], configAttrs[j*2], val);
                    if (val[0] >= configAttrs[j*2+1])
                        System.out.println("setting " + j + ", matches: " + val[0]);
                }

                score = currScore;
                eglConfig = config[i];
            }
        }
        eglContext = egl.eglCreateContext(eglDisplay, eglConfig, EGL10.EGL_NO_CONTEXT, contextAttrs);
        System.out.println("eglCreateContext: " + egl.eglGetError());

        gl = (GL11) eglContext.getGL();
        return true;
    }

    /**
     * Called to create the EGLSurface to be used for rendering. This function should not be called by the inheriting
     * activity, but can be overridden if needed.
     *
     * @param surface The SurfaceHolder that holds the surface that we are going to render to.
     * @return True if successful
     */

    //public void endGame() {
    //}
    protected boolean createEGLSurface(SurfaceHolder surface)
    {
        eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig, surface, null);

        System.out.println("eglSurface: " + eglSurface + ", err: " + egl.eglGetError());
        int sizes[] = new int[1];

        egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_WIDTH, sizes);
        surfaceWidth = sizes[0];
        egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_HEIGHT, sizes);
        surfaceHeight = sizes[0];

        System.out.println("checking glVendor == null?");
        if (this.glVendor == null) {
            System.out.println("Making current and back");
            makeCurrent();
            unMakeCurrent();
        }

        System.out.println("Done create EGL surface");

        return true;
    }

    /**
     * Destroys the EGLSurface used for rendering. This function should not be called by the inheriting
     * activity, but can be overridden if needed.
     */
    protected void destroyEGLSurface()
    {
        System.out.println("*** destroyEGLSurface");
        if (eglDisplay != null && eglSurface != null)
            egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
        if (eglSurface != null)
            egl.eglDestroySurface(eglDisplay, eglSurface);
        eglSurface = null;
    }

    /**
     * Called to clean up egl. This function should not be called by the inheriting
     * activity, but can be overridden if needed.
     */
    protected void cleanupEGL()
    {
        System.out.println("cleanupEGL");
        destroyEGLSurface();
        if (eglDisplay != null)
            egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
        if (eglContext != null)
            egl.eglDestroyContext(eglDisplay, eglContext);
        if (eglDisplay != null)
            egl.eglTerminate(eglDisplay);

        eglDisplay = null;
        eglContext = null;
        eglSurface = null;

        ranInit = false;
        eglConfig = null;

        cachedSurfaceHolder = null;
        surfaceWidth = 0;
        surfaceHeight = 0;
    }

    /**
     * Implementation function:
     * The application does not and should not overide or call this directly
     * Instead, the application should call NVEventEGLSwapBuffers(),
     * which is declared in nv_event.h
     */

    public boolean swapBuffers()
    {
        //long stopTime;
        //long startTime = nvGetSystemTime();

        if (SwapBufferSkip > 0) {
            SwapBufferSkip--;
            System.out.println("swapBuffer wait");
            return true;
        }
        if (eglSurface == null)
        {
            System.out.println("eglSurface is NULL");
            return false;
        }
        else if (!egl.eglSwapBuffers(eglDisplay, eglSurface))
        {
            System.out.println("eglSwapBufferrr: " + egl.eglGetError());
            return false;
        }
        //stopTime = nvGetSystemTime();
        //String s = String.format("%d ms in eglSwapBuffers", (int)(stopTime - startTime));
        //Log.v("EventAccelerometer", s);

        return true;
    }

    public boolean getSupportPauseResume()
    {
        return supportPauseResume;
    }

    public int getSurfaceWidth()
    {
        return surfaceWidth;
    }

    public int getSurfaceHeight()
    {
        return surfaceHeight;
    }

    /**
     * Implementation function:
     * The application does not and should not overide or call this directly
     * Instead, the application should call NVEventEGLMakeCurrent(),
     * which is declared in nv_event.h
     */

    public void GetGLExtensions()
    {
        if (!HasGLExtensions && gl != null && this.cachedSurfaceHolder != null)
        {
            glVendor = gl.glGetString(GL10.GL_VENDOR);
            glExtensions = gl.glGetString(GL10.GL_EXTENSIONS);
            glRenderer = gl.glGetString(GL10.GL_RENDERER);
            glVersion = gl.glGetString(GL10.GL_VERSION);
            System.out.println("Vendor: " + glVendor);
            System.out.println("Extensions " + glExtensions);
            System.out.println("Renderer: " + glRenderer);
            System.out.println("GIVersion: " + glVersion);

            // initGL();

            if (this.glVendor != null)
            {
                this.HasGLExtensions = true;
            }
        }
    }

    public boolean makeCurrent()
    {
        if (eglContext == null)
        {
            System.out.println("eglContext is NULL");
            return false;
        }
        else if (eglSurface == null)
        {
            System.out.println("eglSurface is NULL");
            return false;
        }
        else if (!egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
        {
            if (!egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
            {
                System.out.println("eglMakeCurrent err: " + egl.eglGetError());
                return false;
            }
        }

        // This must be called after we have bound an EGL context
        //nvAcquireTimeExtension();
        GetGLExtensions();
        return true;
    }

    public int getOrientation()
    {
        return display.getOrientation();
    }

    /**
     * Implementation function:
     * The application does not and should not overide or call this directly
     * Instead, the application should call NVEventEGLUnmakeCurrent(),
     * which is declared in nv_event.h
     */
    public boolean unMakeCurrent()
    {
        if (!egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT))
        {
            System.out.println("egl(Un)MakeCurrent err: " + egl.eglGetError());
            return false;
        }

        return true;
    }

    /**
     * Called when the Activity is exiting and it is time to cleanup.
     * Kept separate from the {@link #cleanup()} function so that subclasses
     * in their simplest form do not need to call any of the parent class' functions. This to make
     * it easier for pure C/C++ application so that these do not need to call java functions from C/C++
     * code.
     *
     * @see #cleanup()
     */
    protected void systemCleanup()
    {
        if (ranInit)
            cleanup();
        cleanupEGL();

        //postCleanup();
    }
    public void callLauncherActivityWithSpecialValue(final int type, final int value) {
        if(type == 1223 || type == 1226 || type == 1227) {
            if(value == 1) {
                runOnUiThread(new Runnable() {
                    public void run() {
                   //     imageViewBtnOpen.setVisibility(View.GONE);
                    //    imageViewBtnDonateOpen.setVisibility(View.GONE);
                    //    rpc_gps.setVisibility(View.INVISIBLE);
                    //    if(rpc_gps_status) Utils.HideLayout(rpc_gps, false);
                    //    if(rpc_greenzone_status) Utils.HideLayout(rpc_greenzone, false);
                    }
                });
            } else {
                runOnUiThread(new Runnable() {
                    public void run() {
//                        imageViewBtnOpen.setVisibility(View.VISIBLE);
               //         imageViewBtnDonateOpen.setVisibility(View.VISIBLE);
                //        if(rpc_gps_status) Utils.ShowLayout(rpc_gps, false);
                      //  if(rpc_greenzone_status) Utils.ShowLayout(rpc_greenzone, false);
                    }
                });
            }
        }
        if(type == 1330) {
            runOnUiThread(new Runnable() {
                public void run() {
                    NvEventQueueActivity.getInstance().mQuests.show();
                }
            });
        }
    }

    public boolean isSomethingUIShitOnScreenActive()
    {
//        boolean clmVis = this.constraintlayoutMenu.getVisibility() == View.VISIBLE;
  //      boolean msVis = mMenuSpeedometer.getInputLayout().getVisibility() == View.VISIBLE;
    //    boolean tabVis = this.mTab.getInputLayout().getVisibility() == View.VISIBLE;
      //  boolean dialogVis = this.mDialog.isActive();

        //System.out.println("dbg5001: isSomethingUIShitOnScreenActive => clm: " + clmVis + " menusped: " + msVis + " tabVis: " + tabVis + " dialogVis: " + dialogVis);

       // return clmVis || msVis || tabVis || dialogVis;
        return false;
    }

    // SHO ZA PARASHA? a? M??
    public void callLauncherActivity(final int type) {
        if (type == 1223) {
            runOnUiThread(new Runnable() {
                public void run() {
//                    imageViewBtnOpen.setVisibility(View.GONE);
//                    imageViewBtnDonateOpen.setVisibility(View.GONE);
//                    rpc_gps.setVisibility(View.INVISIBLE);
                  //  Utils.HideLayout(imageViewBtnOpen, false);
                  //  Utils.HideLayout(imageViewBtnDonateOpen, false);
//                    if(rpc_gps_status) Utils.HideLayout(rpc_gps, false);
//                    if(rpc_greenzone_status) Utils.HideLayout(rpc_greenzone, false);
                }
            });
        }
        if (type == 1226 || type == 1227) {
            runOnUiThread(new Runnable() {
                public void run() {
//                    imageViewBtnOpen.setVisibility(View.VISIBLE);
//                    imageViewBtnDonateOpen.setVisibility(View.VISIBLE);
//                    if(rpc_gps_status) Utils.ShowLayout(rpc_gps, false);
//                    if(rpc_greenzone_status) Utils.sLayout(rpc_greenzone, false);
                    //Utils.ShowLayout(imageViewBtnOpen, true);
                  //  Utils.ShowLayout(imageViewBtnDonateOpen, true);

                }
            });
        }

        if (type == 1230) {
            runOnUiThread(new Runnable() {
                public void run() {
                    rpc_gps_status = true;
                    Utils.ShowLayout(rpc_gps, true);
                }
            });
        }
        if (type == 1231) {
            runOnUiThread(new Runnable() {
                public void run() {
                    rpc_gps_status = false;
                    Utils.HideLayout(rpc_gps, true);
                }
            });
        }

        if (type == 1232) {
            runOnUiThread(new Runnable() {
                public void run() {
                    rpc_greenzone_status = true;
                    //System.out.println("MIHAIL rpc_greenzone_status true");
                    Utils.ShowLayout(rpc_greenzone, true);
                    Utils.ShowLayout(rpc_greenzone, true);
                    Utils.ShowLayout(rpc_greenzone, true);
                }
            });
        }
        if (type == 1233) {
            runOnUiThread(new Runnable() {
                public void run() {
                    rpc_greenzone_status = false;
                    //  System.out.println("MIHAIL rpc_greenzone_status false");
                    Utils.HideLayout(rpc_greenzone, true);
                }
            });
        }
        if (type == 1234) {
            runOnUiThread(new Runnable() {
                public void run() {
                    if(rpc_greenzone_status) Utils.ShowLayout(rpc_greenzone, true);
                    if(rpc_gps_status) Utils.ShowLayout(rpc_gps, true);
                }
            });
        }
    }

    public void setHudLogo(int id) {

    }

    public static void setTogleHuds(boolean b){
        setTogleHud(b);
    }


    public void showCode(int id, String name) {

    }

    public void setHudSatiety(int id) {
        runOnUiThread(new Runnable() {
            public void run() {
                //System.out.println("Mihail Сытость равна "+id);
                mHud.SetEat(id);
            }
        });
    }

    public void setSpeedMenu(int stype, int svalue) {
        runOnUiThread(new Runnable() {
            public void run() {
                if(stype == 1) SpeedometerMenu.setParking(svalue);
                if(stype == 2) SpeedometerMenu.setStrob(svalue);
                if(stype == 3) SpeedometerMenu.setNeon(svalue);
            }
        });
    }

    public static void statusButtonMenu(Boolean status) {
       // if(status == true) imageViewBtnOpen.setVisibility(View.VISIBLE);
       // if(status == false) imageViewBtnOpen.setVisibility(View.GONE);
    }

    public void showInputLayout()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mInputManager.ShowInputLayout();
            }
        });
    }

    public void hideInputLayout()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mInputManager.HideInputLayout();
            }
        });
    }

    public byte[] getClipboardText()
    {
        String retn = " ";

        if(mClipboardManager.getPrimaryClip() != null)
        {
            ClipData.Item item = mClipboardManager.getPrimaryClip().getItemAt(0);
            if(item != null)
            {
                CharSequence sequence = item.getText();
                if(sequence != null)
                {
                    retn = sequence.toString();
                }
            }
        }

        byte[] toReturn = null;
        try
        {
            toReturn = retn.getBytes("windows-1251");
        }
        catch(UnsupportedEncodingException e)
        {

        }

        return toReturn;
    }

    // -- Hud
    public native void showTab();

    public void showHud(boolean isAnimation) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mHud.show(isAnimation);
           //     Utils.ShowLayout(imageViewBtnOpen, false);
         //       if(rpc_gps_status == true) Utils.ShowLayout(rpc_gps, false);
           //     if(rpc_greenzone_status == true) Utils.ShowLayout(rpc_greenzone, false);
            }
        });


    }

    public void hideHud(boolean isAnimation) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Utils.HideLayout(mHud.getInputLayout(), isAnimation);
            //    imageViewBtnOpen.setVisibility(View.GONE);
           //    imageViewBtnDonateOpen.setVisibility(View.GONE);
             //   if(rpc_gps_status == true) {
             //       Utils.HideLayout(rpc_gps, false);
             //   }
              //  if(rpc_greenzone_status == true) {
             //       Utils.HideLayout(rpc_greenzone, false);
              //  }
            }
        });
    }

    public void updateHud() {
        runOnUiThread(() -> this.mHud.updateHud());
    }


    public void setCashMoney(String money) {
        int cash = Integer.parseInt(money);
        //System.out.println("MIHAIL setCashMoney "+ money);
        runOnUiThread(() -> this.mHud.SetMoney(cash));
    }

    public void setMoney(int money) {
        runOnUiThread(() -> this.mHud.SetMoney(money));
    }

    public void setHP(int hp) {
        runOnUiThread(() -> this.mHud.SetHP(hp));
    }

    public void setEat(int eat) {
        runOnUiThread(() -> this.mHud.SetEat(eat));
    }

    public void setArmour(int armour) {
        runOnUiThread(() -> this.mHud.SetArmour(armour));
    }

    public void updateTime() {
        runOnUiThread(() -> this.mHud.UpdateTime());
    }

    public void setAmmo1(int ammo) {
        runOnUiThread(() -> this.mHud.SetAmmo1(ammo));
    }

    public void setAmmo2(int ammo) {
        runOnUiThread(() -> this.mHud.SetAmmo2(ammo));
    }

    public void setOnline(int online) {
        runOnUiThread(() -> this.mHud.SetOnline(online));
    }

    public void setX2(boolean state) {
        runOnUiThread(() -> this.mHud.SetX2(state));
    }

    public void setWantedLevel(int level) {
        runOnUiThread(() -> this.mHud.SetWantedLevel(level));
    }

    public void updateHudIcon(int id) {
        runOnUiThread(() -> this.mHud.UpdateHudIcon(id));
    }

    // -- SpawnSelector
    public void showSpawnSelector(boolean home, boolean fraction) {
        runOnUiThread(() -> this.mSpawnSelector.show(home, fraction));
    }

    // -- Dialog
    public void showDialog(int dialogId, int dialogTypeId, byte[] bArr, byte[] bArr2, byte[] bArr3, byte[] bArr4) {
        final String caption = new String(bArr, StandardCharsets.UTF_8);
        final String content = new String(bArr2, StandardCharsets.UTF_8);
        final String leftBtnText = new String(bArr3, StandardCharsets.UTF_8);
        final String rightBtnText = new String(bArr4, StandardCharsets.UTF_8);
        runOnUiThread(() -> {
            this.mDialog.show(dialogId, dialogTypeId, caption, content, leftBtnText, rightBtnText);
//            if(startGame == 0) {
//                startGame = 1;
//                this.mLoading.hide();
//            }
            Writer writer = null;
            try {
                writer = new BufferedWriter(new OutputStreamWriter(
                        new FileOutputStream(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/log_dialog.txt"), "utf-8"));
                writer.write("ID:"+dialogId+" DialogID: "+dialogTypeId+" Text"+content);
            } catch (IOException ex) {
                // Report
            } finally {
                try {writer.close();} catch (Exception ex) {/*ignore*/}
            }
        });
    }

    public void hideDialogWithoutReset() {
        runOnUiThread(() -> this.mDialog.hideWithoutReset());
    }

    public void showDialogWithOldContent() {
        runOnUiThread(() -> this.mDialog.showWithOldContent());
    }

    // -- AZS
    public void showAZS(int fuelId, int maxFuel, int currentFuel, int price, int balance) {
        runOnUiThread(() -> this.mAZS.show(fuelId, maxFuel, currentFuel, price, balance));
    }

    public native void refuelTheCar(int fuel, int price);

    // -- Loading
    public void setLoadingText(int id) {
        runOnUiThread(() -> this.mLoading.setText(id));
    }

    // -- Numbers
    public void showNumbers(int balance) {
        runOnUiThread(() -> this.mNumbers.show(balance));
    }
    public void generateNumber(boolean status) {
        runOnUiThread(() -> this.mNumbers.generateNumber(status));
    }
    public void buyNumber(boolean status) {
        runOnUiThread(() -> this.mNumbers.buyNumber(status));
    }
    public native void sendCheckGenerateNumber();
    public native void sendGenerateNumber();
    public native void sendCheckNumber(int type, String number, String region);
    public native void sendBuyNumber(int type, String number, String region);


    // -- Quests
    public void showQuests() {
        runOnUiThread(() -> this.mQuests.show());
    }

    public void setQuestInfo(int id, int progress) {
        runOnUiThread(() -> this.mQuests.setQuestInfo(id, progress));
    }

    public native void applyQuest(int id);

    // -- Welcome
    public void showWelcome(boolean isRegister) {
        runOnUiThread(() -> {
            System.out.println("Mihail StartWelcome");
            this.mLoading.hide();
            this.mWelcome.show(isRegister);
        });
    }

    public void hideWelcome() {
        runOnUiThread(() -> Utils.HideLayout(this.mWelcome.getInputLayout(), true));
    }

    // -- Captcha
    public void showCaptcha() {
        runOnUiThread(() -> Utils.ShowLayout(this.mCaptcha.getInputLayout(), true));
    }
    public native void hideCaptcha();


    // -- Tab
    public void showTabWindow(boolean isAnim) {
        runOnUiThread(() -> this.mTab.show(isAnim));
    }

    public void hideTabWindow() {
        runOnUiThread(() -> Utils.HideLayout(this.mTab.getInputLayout(), false));
    }

    public void clearTabStat() {
        runOnUiThread(() -> this.mTab.clearStat());
    }


    public void setTabStat(int id, byte[] bArr, int scores, int pings) {
        String str;
        try {
            str = new String(bArr, "windows-1251");
        } catch (UnsupportedEncodingException unused) {
            str = "";
        }
        String names = str;
        runOnUiThread(() ->

            this.mTab.setStat(id, names, scores, pings));
            //this.mHud.SetOnline(id);
    }

    public native void onTabClose();

    // -- Speedometer
    public void showSpeedometer() {
        runOnUiThread(() -> Utils.ShowLayout(mSpeedometer.getInputLayout(), true));
    }

    public void hideSpeedometer() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Utils.HideLayout(mSpeedometer.getInputLayout(), true);
                Utils.HideLayout(mMenuSpeedometer.getInputLayout(), true);

                NvEventQueueActivity.setTogleHuds(true);
            }
        });
    }

    public void setSpeedometerSpeed(int speed) {
        runOnUiThread(() -> this.mSpeedometer.setSpeed(speed));
    }

    public void setSpeedometerFuel(int fuel) {
        runOnUiThread(() -> this.mSpeedometer.setFuel(fuel));
    }


    public void setSpeedometerCarHP(int hp) {
        runOnUiThread(() -> this.mSpeedometer.setCarHP(hp));
    }

    public void setSpeedometerMileage(int mileage) {
        runOnUiThread(() -> this.mSpeedometer.setMileage(mileage));
    }


    public void setEngineState(int state) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSpeedometer.setEngineState(state);
                mMenuSpeedometer.setEngine(state);
            }
        });
    }


    public void setLightState(int state) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                  mSpeedometer.setLightState(state);
                  mMenuSpeedometer.setLight(state);
            }
        });
    }
    public static void fixEditTextForAndroid10Xiaomi(EditText editText) {
        if (Build.MANUFACTURER.equalsIgnoreCase("Xiaomi") && Build.VERSION.SDK_INT == 29) {
            editText.setCursorVisible(false);
        }
    }

    public static int dpToPx(float f, Context context) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, f, context.getResources().getDisplayMetrics());
    }

    public void setBeltState(int state) {
      runOnUiThread(() -> this.mSpeedometer.setBeltState(state));
    }


    public void setLockState(int state) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSpeedometer.setLockState(state);
                mMenuSpeedometer.setLock(state);
            }
        });
    }

    public void showSpeedMenu() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMenuSpeedometer.show();
            }
        });
    }

    public void finishGameActivity() {
        finishActivity(0);
    }


    public native void switchWeapon();
    public native void spawnPlayer(int id);
    public native void onClickPlayInWelcomeWindow();
    public native void sendDialogResponse(int btnId, int dialogId, int listBoxItem, String inputtext);
    public native void sendClick(String inputtext);
    public static NvEventQueueActivity getInstance() {
        return instance;
    }
}
