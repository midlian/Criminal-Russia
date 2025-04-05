package criminal.russia;

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;

import androidx.annotation.NonNull;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings;
import com.wardrumstudios.utils.WarMedia;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;

public class GTASA extends WarMedia {
    public static GTASA gtasaSelf = null;
    static String vmVersion;
    private boolean once = false;

    static {
        vmVersion = null;
        System.out.println("**** Loading SO's");
        try
        {
            vmVersion = System.getProperty("java.vm.version");
            System.out.println("vmVersion " + vmVersion);
            System.loadLibrary("ImmEmulatorJ");
        } catch (ExceptionInInitializerError | UnsatisfiedLinkError e) {
        }

        System.loadLibrary("GTASA");
        System.loadLibrary("samp"); // you look loooooooneeeeeeeeelyyyyyyyyyy... i can fix that....
    }

    public static void staticEnterSocialClub()
    {
        gtasaSelf.EnterSocialClub();
    }

    public static void staticExitSocialClub() {
        gtasaSelf.ExitSocialClub();
    }

    public void AfterDownloadFunction() {

    }

    public void EnterSocialClub() {

    }

    public void ExitSocialClub() {

    }

    public boolean ServiceAppCommand(String str, String str2)
    {
        return false;
    }

    public int ServiceAppCommandValue(String str, String str2)
    {
        return 0;
    }

    public native void main();

    public void onActivityResult(int i, int i2, Intent intent)
    {
        super.onActivityResult(i, i2, intent);
    }

    public void onConfigurationChanged(Configuration configuration)
    {
        super.onConfigurationChanged(configuration);
    }

    private native void setClientIp(byte[] ip);
    private native void setClientPort(int port);

    private void initaliseClientInfo()
    {
        Intent intent = getIntent();

        if(intent == null)
        {
            setClientIp(null);
            return;
        }

        String ip = intent.getStringExtra("ip");
        int port = intent.getIntExtra("port", 7777);

        if(ip == null)
        {
            setClientIp(null);
            return;
        }

        byte[] ipBytes = null;
        try
        {
            ipBytes = ip.getBytes("windows-1251");
        }
        catch(UnsupportedEncodingException e)
        {
            setClientIp(null);
            return;
        }

        if(ipBytes == null)
        {
            setClientIp(null);
            return;
        }

        setClientIp(ipBytes);
        setClientPort(port);
    }

    public void onCreate(Bundle bundle)
    {
        if(!once)
        {
            // initaliseClientInfo();

            once = true;
        }
        System.out.println("GTASA onCreate");
        gtasaSelf = this;
        wantsMultitouch = true;
        wantsAccelerometer = true;

        super.onCreate(bundle);
    }

    public void onDestroy()
    {
        System.out.println("GTASA onDestroy");
        super.onDestroy();
    }

    public boolean onKeyDown(int i, KeyEvent keyEvent)
    {
        return super.onKeyDown(i, keyEvent);
    }

    public void onPause()
    {
        System.out.println("GTASA onPause");
        super.onPause();
    }

    public void onRestart()
    {
        System.out.println("GTASA onRestart");
        super.onRestart();
    }

    public void onResume()
    {
        System.out.println("GTASA onResume");
        super.onResume();
    }

    public void onStart()
    {
        System.out.println("GTASA onStart");
        super.onStart();
    }

    public void onStop()
    {
        System.out.println("GTASA onStop");
        super.onStop();
    }

    public native void setCurrentScreenSize(int i, int i2);
}