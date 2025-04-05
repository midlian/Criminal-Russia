package criminal.russia.custom.cef;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.net.http.SslError;
import android.os.Build;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.ClientCertRequest;
import android.webkit.HttpAuthHandler;
import android.webkit.JavascriptInterface;
import android.webkit.RenderProcessGoneDetail;
import android.webkit.SafeBrowsingResponse;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import androidx.annotation.NonNull;

public class CefBrowser extends WebView {
    public CefBrowser(@NonNull Context context) {
        super(context);

        init(context);
    }

    public CefBrowser(Context context, @androidx.annotation.Nullable AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public CefBrowser(Context context, @androidx.annotation.Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        init(context);
    }

    public String content = "<!DOCTYPE >\n" +
            "<html>\n" +
            "  <head>\n" +
            "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n" +
            "    <script type=\"text/javascript\">\n" +
            "           function init()\n" +
            "           {\n" +
            "              var testVal = 'Привет от Mobile Test!';\n" +
            "              client.triggerClientEvent('Hello Event', 1, 'Hello');\n" +
            "           }\n" +
            "        </script>\n" +
            "  </head>\n" +
            "  <body>\n" +
            "    <div style=\"clear: both;height: 3px;\"> </div>\n" +
            "    <div>\n" +
            "      <input value=\"Нажми на меня\" type=\"button\" name=\"submit\"\n" +
            "           id=\"btnSubmit\" onclick=\"javascript:return init();\" />\n" +
            "    </div>\n" +
            "  </body>\n" +
            "</html>";

    public void init(Context ctx) {
        setBackgroundColor(0x00000000);

        WebSettings webSettings = getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setGeolocationEnabled(true);

        setOnTouchListener(new CefTouchListener());
        setWebViewClient(new CefViewClient());
        setWebChromeClient(new CefChromeClient());

        addJavascriptInterface(new JavaScriptInterface(getContext()), "client");

        loadData(content, "text/HTML", "UTF-8");
    }

    public class JavaScriptInterface {
        Context mContext;

        JavaScriptInterface(Context c) {
            mContext = c;
        }

        @JavascriptInterface
        public void showToast(String toast) {
            Toast.makeText(mContext, toast, Toast.LENGTH_SHORT).show();
        }

        @JavascriptInterface
        public void triggerClientEvent(String event, int count, Object ... args ) {
            Log.i("CEF", String.format("Trigger Client Event: %s %d", event, count));
        }
    }
}
