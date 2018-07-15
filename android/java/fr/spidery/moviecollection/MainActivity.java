package fr.spidery.moviecollection;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.webkit.ConsoleMessage;
import android.webkit.JavascriptInterface;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static java.util.Arrays.asList;



public class MainActivity extends Activity {

    protected WebAppInterface webAppInterface;

    protected static final int REQUEST_PERMISSIONS_MULTIPLE = 12345;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (this.onCheckAndRequestPermissions(this)) {
            webAppInterface = new WebAppInterface(this, "file:///android_asset/www/index.html", savedInstanceState);
        }

        //checkAndRequestPermissions
        //onRequestPermissionsResult
        //onLaunchWebview
    }

    public class ActivityWebAppInterface{
        public void ActivityWebAppInterface(Bundle savedInstanceState) {
            Context context = getApplicationContext();
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if(webAppInterface != null) {
            webAppInterface.saveState(outState);
        }
    }

    protected boolean onCheckAndRequestPermissions(Activity context) {

        List<String> permissions = asList(
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
            // Manifest.permission.ACCESS_NETWORK_STATE,
            // Manifest.permission.INTERNET
        );

        List<String> permissionsNeeded = new ArrayList<>();

        for (String perm: permissions) {
            if( context.checkSelfPermission(perm) != PackageManager.PERMISSION_GRANTED ) {
                permissionsNeeded.add(perm);
            }
        }

        if (permissionsNeeded.isEmpty()) {
            return true;
        }

        context.requestPermissions(permissionsNeeded.toArray(new String[permissionsNeeded.size()]), REQUEST_PERMISSIONS_MULTIPLE);

        return false;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {

        switch (requestCode) {
            case REQUEST_PERMISSIONS_MULTIPLE: {
                List<String> permissionsNeeded = new ArrayList<>();

                for (String perm : permissions) {
                    if (this.checkSelfPermission(perm) != PackageManager.PERMISSION_GRANTED) {
                        permissionsNeeded.add(perm);
                    }
                }

                if(permissionsNeeded.isEmpty()) {
                    // launch webview


                    //TODO

                    return;
                }

                if (!permissionsNeeded.isEmpty()) {

                    // Permission Denied
                    Toast.makeText(this, "Please allow permissions for the app to work!", Toast.LENGTH_SHORT).show();

                    //for (String perm : permissionsNeeded) {
                    //Boolean showRationale = shouldShowRequestPermissionRationale();

                    AlertDialog.Builder builder = new AlertDialog.Builder(this);

                    builder.setTitle("Permission required");
                    builder.setMessage("SDCard permission is required to access the collection.");

                    // pass arguments to dialog
                    final List<String> xpermissionsNeeded = permissionsNeeded;
                    final Activity xparent = this;

                    builder.setPositiveButton("Retry", new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int id) {
                            dialog.dismiss();

                            // retry request last permissions
                            xparent.requestPermissions(xpermissionsNeeded.toArray(new String[xpermissionsNeeded.size()]), REQUEST_PERMISSIONS_MULTIPLE);
                        }
                    });

                    builder.setNegativeButton("Nope", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                        // close app
                        finishAffinity();
                        }
                    });

                    AlertDialog dialog = builder.create();
                    dialog.show();
                }
            }
        }

    }



    public class WebAppInterface {

        protected final WebView webView;
        protected boolean webAppLoaded = false;
        protected boolean isOnline = false;

        protected final String sdcard;


        public WebAppInterface(Activity context, String url, Bundle previousState) {
            context.setContentView(R.layout.activity_main);

            webView = context.findViewById(R.id.activity_webview);

            sdcard = Environment.getExternalStorageDirectory().getAbsolutePath() + "/";

            if (previousState != null) {
                webView.restoreState(previousState);
            }
            else {

                WebSettings webSettings = webView.getSettings();

                webView.setWebViewClient(new WebViewClient() {
                    @Override
                    public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                        int code = error.getErrorCode();
                        String message = error.getDescription().toString();

                        Log.e("CONSOLE", "onReceivedError: " + code + " - " + message);
                    }
                });
                webView.setWebChromeClient(new WebChromeClient() {
                    public boolean onConsoleMessage(ConsoleMessage cm) {
                        Log.d("CONSOLE", cm.message() + " -- From line " + cm.lineNumber() + " of " + cm.sourceId());
                        return true;
                    }
                });

                //encoding
                webSettings.setDefaultTextEncodingName("utf-8");

                //engine
                webSettings.setDomStorageEnabled(true);
                webSettings.setAppCachePath(context.getApplicationContext().getCacheDir().getAbsolutePath());
                webSettings.setAppCacheEnabled(true);

                webSettings.setJavaScriptEnabled(true);

                //black bg
                webView.setBackgroundColor(0);

                webView.addJavascriptInterface(this, "_AndroidBridge");

                webView.loadUrl(url);
            }
        }

        public void saveState(Bundle bundle) {
            webView.saveState(bundle);
        }

        protected String readFile(File file) {
            //Read text from file

            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {

                StringBuilder text = new StringBuilder();
                String line;

                while ((line = reader.readLine()) != null) {
                    text.append(line);
                }

                reader.close();

                return text.toString();

            }
            catch (IOException e) {}

            return null;
        }

        protected boolean saveFile(File file) {


            return false;
        }

        // Checks if external storage is available for read and write
        protected Boolean isExternalStorageWritable() {
            return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED;
        }


        @JavascriptInterface
        public String openCatalogue() {
            String content = null;

            //state = Environment.getExternalStorageState()
            String entrypoint = "Android/data/" + getPackageName();

            File catalogFolder = new File(sdcard + entrypoint); //storage/emulated/0
            File catalogPoster = new File(sdcard + entrypoint, "posters/");
            File catalogFile = new File(sdcard + entrypoint, "moviecollection.json");

            if (!catalogFolder.exists() && !catalogFolder.mkdirs()) {
                Log.e("CONSOLE", "catalogFolder not created");
            }
            if (!catalogPoster.exists() && !catalogPoster.mkdirs()) {
                Log.e("CONSOLE", "catalogPoster not created");
            }

            if(catalogFile.exists()) {
                // read content
                content = readFile(catalogFile); // or null
            }

            return content;
        }

        @JavascriptInterface
        public Boolean saveCatalogue(String filename, String content) {


            return false;
        }

        public void callJavaScript(String methodName, Object...params){
            boolean firstParam = true;

            StringBuilder stringBuilder = new StringBuilder();

            stringBuilder.append("javascript:try{");

            stringBuilder.append(methodName);
            stringBuilder.append("(");

            for (Object param : params) {
                if (firstParam) {
                    stringBuilder.append(",");
                    firstParam = false;
                }
                if(param instanceof String){
                    stringBuilder.append("'"+param+"'");
                }
            }

            stringBuilder.append(")}catch(error){console.log(error);}");

            Log.d("CONSOLE", "Calling javascript: " + stringBuilder.toString());

            webView.loadUrl(stringBuilder.toString());
        }
    }


}



