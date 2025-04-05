package criminal.russia.custom.cef;

import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.GeolocationPermissions;
import android.webkit.JsPromptResult;
import android.webkit.JsResult;
import android.webkit.PermissionRequest;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebStorage;
import android.webkit.WebView;

public class CefChromeClient extends WebChromeClient {
    public CefChromeClient() {
        super();
        Log.i("CEF", "CefChromeClient");
    }

    @Override
    public void onProgressChanged(WebView view, int newProgress) {
        Log.i("CEF", "onProgressChanged");
        super.onProgressChanged(view, newProgress);
    }

    @Override
    public void onReceivedTitle(WebView view, String title) {
        Log.i("CEF", "onReceivedTitle");
        super.onReceivedTitle(view, title);
    }

    @Override
    public void onReceivedIcon(WebView view, Bitmap icon) {
        Log.i("CEF", "onReceivedIcon");
        super.onReceivedIcon(view, icon);
    }

    @Override
    public void onReceivedTouchIconUrl(WebView view, String url, boolean precomposed) {
        Log.i("CEF", "onReceivedTouchIconUrl");
        super.onReceivedTouchIconUrl(view, url, precomposed);
    }

    @Override
    public void onShowCustomView(View view, WebChromeClient.CustomViewCallback callback) {
        Log.i("CEF", "onShowCustomView");
        super.onShowCustomView(view, callback);
    }

    /** @deprecated */
    @Override
    @Deprecated
    public void onShowCustomView(View view, int requestedOrientation, WebChromeClient.CustomViewCallback callback) {
        Log.i("CEF", "onShowCustomView");
        super.onShowCustomView(view, requestedOrientation, callback);
    }

    @Override
    public void onHideCustomView() {
        Log.i("CEF", "onHideCustomView");
        super.onHideCustomView();
    }

    @Override
    public boolean onCreateWindow(WebView view, boolean isDialog, boolean isUserGesture, Message resultMsg) {
        Log.i("CEF", "onCreateWindow");
        return super.onCreateWindow(view, isDialog, isUserGesture, resultMsg);
    }

    @Override
    public void onRequestFocus(WebView view) {
        Log.i("CEF", "onRequestFocus");
        super.onRequestFocus(view);
    }

    @Override
    public void onCloseWindow(WebView window) {
        Log.i("CEF", "onCloseWindow");
        super.onCloseWindow(window);
    }

    @Override
    public boolean onJsAlert(WebView view, String url, String message, JsResult result) {
        Log.i("CEF", "onJsAlert");
        return true;
    }

    @Override
    public boolean onJsConfirm(WebView view, String url, String message, JsResult result) {
        Log.i("CEF", "onJsConfirm");
        return true;
    }

    @Override
    public boolean onJsPrompt(WebView view, String url, String message, String defaultValue, JsPromptResult result) {
        Log.i("CEF", "onJsPrompt");
        return true;
    }

    @Override
    public boolean onJsBeforeUnload(WebView view, String url, String message, JsResult result) {
        Log.i("CEF", "onJsBeforeUnload");
        return super.onJsBeforeUnload(view, url, message, result);
    }

    /** @deprecated */
    @Override
    @Deprecated
    public void onExceededDatabaseQuota(String url, String databaseIdentifier, long quota, long estimatedDatabaseSize, long totalQuota, WebStorage.QuotaUpdater quotaUpdater) {
        Log.i("CEF", "onExceededDatabaseQuota");
        super.onExceededDatabaseQuota(url, databaseIdentifier, quota, estimatedDatabaseSize, totalQuota, quotaUpdater);
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin, GeolocationPermissions.Callback callback) {
        Log.i("CEF", "onGeolocationPermissionsShowPrompt");
        super.onGeolocationPermissionsShowPrompt(origin, callback);
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
        Log.i("CEF", "onGeolocationPermissionsHidePrompt");
        super.onGeolocationPermissionsHidePrompt();
    }

    @Override
    public void onPermissionRequest(PermissionRequest request) {
        Log.i("CEF", "onPermissionRequest");
        super.onPermissionRequest(request);
    }

    @Override
    public void onPermissionRequestCanceled(PermissionRequest request) {
        Log.i("CEF", "onPermissionRequestCanceled");
        super.onPermissionRequestCanceled(request);
    }

    /** @deprecated */
    @Override
    @Deprecated
    public boolean onJsTimeout() {
        Log.i("CEF", "onJsTimeout");
        return super.onJsTimeout();
    }

    /** @deprecated */
    @Override
    @Deprecated
    public void onConsoleMessage(String message, int lineNumber, String sourceID) {
        Log.i("CEF", "onConsoleMessage 1:" + message);
        super.onConsoleMessage(message, lineNumber, sourceID);
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        Log.i("CEF", "onConsoleMessage 2:" + consoleMessage.message());
        return super.onConsoleMessage(consoleMessage);
    }

    @Override
    @androidx.annotation.Nullable
    public Bitmap getDefaultVideoPoster() {
        Log.i("CEF", "getDefaultVideoPoster");
        return super.getDefaultVideoPoster();
    }

    @Override
    @androidx.annotation.Nullable
    public View getVideoLoadingProgressView() {
        Log.i("CEF", "getVideoLoadingProgressView");
        return super.getVideoLoadingProgressView();
    }

    @Override
    public void getVisitedHistory(ValueCallback<String[]> callback) {
        Log.i("CEF", "getVisitedHistory");
        super.getVisitedHistory(callback);
    }

    @Override
    public boolean onShowFileChooser(WebView webView, ValueCallback<Uri[]> filePathCallback, WebChromeClient.FileChooserParams fileChooserParams) {
        Log.i("CEF", "onShowFileChooser");
        return super.onShowFileChooser(webView, filePathCallback, fileChooserParams);
    }
}
