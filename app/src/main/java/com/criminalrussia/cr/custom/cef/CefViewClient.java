package criminal.russia.custom.cef;

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.ClientCertRequest;
import android.webkit.HttpAuthHandler;
import android.webkit.RenderProcessGoneDetail;
import android.webkit.SafeBrowsingResponse;
import android.webkit.SslErrorHandler;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class CefViewClient extends WebViewClient {
    public CefViewClient() {
        super();
       // Log.i("CEF", "CefClient");
    }

    /** @deprecated */
    @Deprecated
    public boolean shouldOverrideUrlLoading(WebView view, String url) {
        Log.i("CEF", "shouldOverrideUrlLoading");
        view.loadUrl(url);

        return true;
    }

    @Override
    public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
        Log.i("CEF", "shouldOverrideUrlLoading 2s, " + request.getMethod());
//        return super.shouldOverrideUrlLoading(view, request);

        view.loadUrl(request.getUrl().toString());

        return true;
    }

    public void onPageStarted(WebView view, String url, Bitmap favicon) {
     //   Log.i("CEF", "onPageStarted");
        super.onPageStarted(view, url, favicon);
    }

    public void onPageFinished(WebView view, String url) {
      //  Log.i("CEF", "onPageFinished");
        super.onPageFinished(view, url);
    }

    public void onLoadResource(WebView view, String url) {
      //  Log.i("CEF", "onLoadResource");
        super.onLoadResource(view, url);
    }

    public void onPageCommitVisible(WebView view, String url) {
      //  Log.i("CEF", "onPageCommitVisible");
        super.onPageCommitVisible(view, url);
    }

    /** @deprecated */
    @Override
    @Deprecated
    @androidx.annotation.Nullable
    public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
      //  Log.i("CEF", "shouldInterceptRequest");
        return super.shouldInterceptRequest(view, url);
    }

    @Override
    @androidx.annotation.Nullable
    public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
      //  Log.i("CEF", "shouldInterceptRequest");
        return super.shouldInterceptRequest(view, request);
    }

    /** @deprecated */
    @Override
    @Deprecated
    public void onTooManyRedirects(WebView view, Message cancelMsg, Message continueMsg) {
     //   Log.i("CEF", "onTooManyRedirects");
        super.onTooManyRedirects(view, cancelMsg, continueMsg);
    }

    /** @deprecated */
    @Override
    @Deprecated
    @androidx.annotation.Nullable
    public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
     //   Log.i("CEF", "onReceivedError");
        super.onReceivedError(view, errorCode, description, failingUrl);
    }

    @Override
    public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
     //   Log.i("CEF", "onReceivedError");
        super.onReceivedError(view, request, error);
    }

    @Override
    public void onReceivedHttpError(WebView view, WebResourceRequest request, WebResourceResponse errorResponse) {
      //  Log.i("CEF", "onReceivedHttpError");
        super.onReceivedHttpError(view, request, errorResponse);
    }

    @Override
    public void onFormResubmission(WebView view, Message dontResend, Message resend) {
       // Log.i("CEF", "onFormResubmission");
        super.onFormResubmission(view, dontResend, resend);
    }

    @Override
    public void doUpdateVisitedHistory(WebView view, String url, boolean isReload) {
       // Log.i("CEF", "doUpdateVisitedHistory");
        super.doUpdateVisitedHistory(view, url, isReload);
    }

    @Override
    public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
       // Log.i("CEF", "onReceivedSslError");
        super.onReceivedSslError(view, handler, error);
    }

    @Override
    public void onReceivedClientCertRequest(WebView view, ClientCertRequest request) {
      //  Log.i("CEF", "onReceivedClientCertRequest");
        super.onReceivedClientCertRequest(view, request);
    }

    @Override
    public void onReceivedHttpAuthRequest(WebView view, HttpAuthHandler handler, String host, String realm) {
       // Log.i("CEF", "onReceivedHttpAuthRequest");
        super.onReceivedHttpAuthRequest(view, handler, host, realm);
    }

    @Override
    public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event) {
       // Log.i("CEF", "shouldOverrideKeyEvent");
        return super.shouldOverrideKeyEvent(view, event);
    }

    @Override
    public void onUnhandledKeyEvent(WebView view, KeyEvent event) {
      //  Log.i("CEF", "onUnhandledKeyEvent");
        super.onUnhandledKeyEvent(view, event);
    }

    @Override
    public void onScaleChanged(WebView view, float oldScale, float newScale) {
      //  Log.i("CEF", "onScaleChanged");
        super.onScaleChanged(view, oldScale, newScale);
    }

    @Override
    public void onReceivedLoginRequest(WebView view, String realm, @androidx.annotation.NonNull String account, String args) {
       // Log.i("CEF", "onReceivedLoginRequest");
        super.onReceivedLoginRequest(view, realm, account, args);
    }

    @Override
    public boolean onRenderProcessGone(WebView view, RenderProcessGoneDetail detail) {
       // Log.i("CEF", "onRenderProcessGone");
        return super.onRenderProcessGone(view, detail);
    }

    @Override
    public void onSafeBrowsingHit(WebView view, WebResourceRequest request, int threatType, SafeBrowsingResponse callback) {
        //Log.i("CEF", "onSafeBrowsingHit");
        super.onSafeBrowsingHit(view, request, threatType, callback);
    }
}
