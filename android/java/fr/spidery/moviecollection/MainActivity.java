package fr.spidery.moviecollection;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.util.Log;
import android.content.Intent;
import android.webkit.ConsoleMessage;
import android.webkit.JavascriptInterface;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;
import static java.util.Arrays.asList;



public class MainActivity extends Activity {

    protected WebAppInterface webAppInterface;

    protected static final int REQUEST_PERMISSIONS_MULTIPLE = 12345;


    @Override
    protected void onCreate(Bundle previousState) {
        super.onCreate(previousState);

        List<String> permissions = asList(
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_NETWORK_STATE,
            Manifest.permission.INTERNET
        );

        if (checkAndRequestPermissions(this, permissions)) {
            webAppInterface = new WebAppInterface(this, "file:///android_asset/www/index.html", previousState);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if(webAppInterface != null) {
            webAppInterface.saveState(outState);
        }
    }

    protected boolean checkAndRequestPermissions(Activity activity, List<String> permissions) {

        // Always return true for SDK < M, let the system deal with the permissions
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            Log.w("CONSOLE", "hasPermissions: API version < M, returning true by default");
            return true;
        }

        List<String> permissionsNeeded = new ArrayList<>();

        for (String perm: permissions) {
            if(activity.checkSelfPermission(perm) != PackageManager.PERMISSION_GRANTED) {
                permissionsNeeded.add(perm);
            }
        }
        if (permissionsNeeded.isEmpty()) {
            return true;
        }

        activity.requestPermissions(
            permissionsNeeded.toArray(new String[permissionsNeeded.size()]),
            REQUEST_PERMISSIONS_MULTIPLE
        );

        return false;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {

        switch (requestCode) {

            case REQUEST_PERMISSIONS_MULTIPLE: {

                List<String> permissionsNeeded = new ArrayList<>();

                for (String perm : permissions) {
                    // TODO try shouldShowRequestPermissionRationale if "never ask again"
                    if (this.checkSelfPermission(perm) != PackageManager.PERMISSION_GRANTED) {
                        permissionsNeeded.add(perm);
                    }
                }

                if(permissionsNeeded.isEmpty()) {
                    // has all needed permissions
                    restart(); return;
                }

                //permissions denied
                AlertDialog dialog = this.createDialogRequestPermissions(this, permissionsNeeded);
                dialog.show();
            }

            default: {
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            }
        }
    }

    protected AlertDialog createDialogRequestPermissions(final Activity activity, final List<String> permissions) {
        
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        builder.setTitle("Permission required");
        builder.setMessage("SDCard permission is required to access the collection.");

        builder.setPositiveButton("Retry", new DialogInterface.OnClickListener() {

            public void onClick(DialogInterface dialog, int id) {
                dialog.dismiss();

                // retry request last permissions
                activity.requestPermissions(
                    permissions.toArray(new String[permissions.size()]),
                    REQUEST_PERMISSIONS_MULTIPLE
                );
            }
        });

        builder.setNegativeButton("Quit", new DialogInterface.OnClickListener() {

            public void onClick(DialogInterface dialog, int id) {
                // close app
                finishAffinity();
            }
        });

        return builder.create();
    }

    public void restart() {
        if (Build.VERSION.SDK_INT >= 11) {
            recreate();
        } else {
            Intent intent = getIntent();
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            finish();
            overridePendingTransition(0, 0);

            startActivity(intent);
            overridePendingTransition(0, 0);
        }
    }





    public class WebAppInterface {

        protected final WebView webView;
        // protected boolean webAppLoaded = false;
        // protected boolean isOnline = false;

        // sdcard access
        protected final String sdcard;

        // collection files
        protected final File storageFolder;
        protected final File storageFilename;
        protected final File storagePosters;


        public WebAppInterface(Activity activity, String url, Bundle previousState) {
            
            activity.setContentView(R.layout.activity_main);

            webView = activity.findViewById(R.id.activity_webview);

            if (previousState != null) {
                webView.restoreState(previousState);
            }

            WebSettings webSettings = webView.getSettings();

            //optional, for show console and alert
            webView.setWebViewClient(new MyWebViewClient());
            webView.setWebChromeClient(new MyWebChromeClient());

            //encoding
            webSettings.setDefaultTextEncodingName("utf-8");

            //engine
            webSettings.setDomStorageEnabled(true);
            webSettings.setAppCachePath(activity.getApplicationContext().getCacheDir().getAbsolutePath());
            webSettings.setAppCacheEnabled(true);

            webSettings.setJavaScriptEnabled(true);

            //black bg
            webView.setBackgroundColor(0);

            webView.addJavascriptInterface(this, "AndroidInterface");

            webView.loadUrl(url);


            // create files and folder if not set
    
            sdcard = Environment.getExternalStorageDirectory().getAbsolutePath() + "/";

            //state = Environment.getExternalStorageState()
            String entrypoint = "Android/data/" + getPackageName();

            storageFolder = new File(sdcard + entrypoint); //storage/emulated/0
            storageFilename = new File(sdcard + entrypoint, "moviecollection.json");
            storagePosters = new File(sdcard + entrypoint, "posters/");

            if (!storageFolder.exists() && !storageFolder.mkdirs()) {
                Log.e("CONSOLE", "storageFolder not created");
            }
            if (!storagePosters.exists() && !storagePosters.mkdirs()) {
                Log.e("CONSOLE", "storagePosters not created");
            }
        }

        protected class MyWebViewClient extends WebViewClient {

            @Override
            public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                int code = error.getErrorCode();
                String message = error.getDescription().toString();

                Log.e("CONSOLE", "onReceivedError: " + code + " - " + message);
            }
        }

        protected class MyWebChromeClient extends WebChromeClient {

            @Override
            public boolean onConsoleMessage(ConsoleMessage cm) {
                Log.d("CONSOLE", cm.message() + " -- From line " + cm.lineNumber() + " of " + cm.sourceId());
                return true;
            }
        }

        public void saveState(Bundle outState) {
            webView.saveState(outState);
        }

        // Checks if external storage is available for read and write
        protected Boolean isExternalStorageWritable() {
            return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED;
        }

        @JavascriptInterface
        public String openCollection() {

            // throw new NullPointerException("demo");
            // TODO


            if(storageFilename.exists()) {
                return readFile(storageFilename); // or null
            }

            return null;
        }

        @JavascriptInterface
        public Boolean saveCollection(String storage) {

            if(storageFolder.exists()) {
                return writeFile(storageFilename, storage); // or null
            }

            return false;
        }

        @JavascriptInterface
        public String getPoster(String posterName) {

            if(storagePosters.exists()) {
                File storagePoster = new File(storagePosters, posterName);

                return readFile(storagePoster); // or null
            }

            return null;
        }

        @JavascriptInterface
        public boolean savePoster(String posterName, String storage) {

            if(storagePosters.exists()) {
                File storagePoster = new File(storagePosters, posterName);

                return writeFile(storagePoster, storage); // or null
            }

            return false;
        }


        protected String readFile(File file) {
            try {
                BufferedReader reader = new BufferedReader(new FileReader(file));
                
                StringBuilder text = new StringBuilder();
                String line;

                while ((line = reader.readLine()) != null) {
                    text.append(line);
                }

                reader.close();

                return text.toString();
            }
            catch (IOException e) {
                Log.e("Exception", "readFile failed: " + e.toString());
            }

            return null;
        }

        protected boolean writeFile(File file, String content) {
            try {
                BufferedWriter writer = new BufferedWriter(new FileWriter(file));
                
                writer.write(content);
                
                writer.close();

                return true;
            }
            catch (IOException e) {
                Log.e("Exception", "writeFile failed: " + e.toString());
            } 

            return false;
        }

        // TODO prefer org.json.JSONObject
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

            stringBuilder.append(")}catch(error){console.log('callJavaScript', error);}");

            Log.d("CONSOLE", "Calling javascript: " + stringBuilder.toString());

            if (Build.VERSION.SDK_INT >= 19) {
                webView.evaluateJavascript(stringBuilder.toString(), null);
            } else {
                webView.loadUrl(stringBuilder.toString());
            }
        }
    }


}

