package com.criminalrussia.launcher.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.WindowManager;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.liulishuo.filedownloader.FileDownloader;
import criminal.russia.R;
import com.criminalrussia.launcher.Config;
import com.criminalrussia.launcher.utils.Utils;

import net.lingala.zip4j.core.ZipFile;
import net.lingala.zip4j.exception.ZipException;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

public class DownloadActivity extends AppCompatActivity
{
    public static String path = "";
    public static String url = "http://176.124.220.143/dl/game/cache26.zip"; //ссылка на кеш

    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_load);

        // -- FIX DISPLAY
        this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        FileDownloader.setup(this);
        InitialiseDownload(Utils.getDownloadType());
    }

    private void InitialiseDownload(int downloadType) {
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/cache.zip";

        try {
            File file = new File(Config.path_cache);
            if (file.exists()) {
                if (!file.delete()) {
                    System.out.println("Failed to delete the file: ");
                } else {
                    System.out.println("File deleted successfully.");
                }
            }


            File zip = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/cache.zip");
            if (zip.exists()){
                if (!zip.delete()) {
                    System.out.println("Failed to delete the file: ");
                } else {
                    System.out.println("File deleted successfully.");
                }
            }
        } catch (Exception ignored) {

        }


        createDownloadTask();
    }

    public void createDownloadTask() {
        new Thread(() -> {
            try {
                File directory = new File(getExternalFilesDir(null), "downloads");
                if (!directory.exists()) {
                    directory.mkdirs();
                }

                path = new File(directory, "cache.zip").getAbsolutePath();

                URLConnection urlConnection = new URL(url).openConnection();
                urlConnection.setConnectTimeout(10000);
                urlConnection.setReadTimeout(15000);
                urlConnection.connect();

                int length = urlConnection.getContentLength();
                if (length == -1) {
                    runOnUiThread(() -> Toast.makeText(this, "Не удалось определить размер файла", Toast.LENGTH_SHORT).show());
                    return;
                }

                try (InputStream inputStream = urlConnection.getInputStream();
                     FileOutputStream fileOutputStream = new FileOutputStream(path)) {

                    byte[] buffer = new byte[65536];
                    int bytesRead;
                    int totalBytesRead = 0;
                    long lastUpdateTime = System.currentTimeMillis();

                    while ((bytesRead = inputStream.read(buffer)) != -1) {
                        totalBytesRead += bytesRead;
                        fileOutputStream.write(buffer, 0, bytesRead);

                        if (System.currentTimeMillis() - lastUpdateTime > 200) {
                            lastUpdateTime = System.currentTimeMillis();
                            int finalTotalBytesRead = totalBytesRead;
                            runOnUiThread(() -> updateProgress(finalTotalBytesRead, length));
                        }
                    }

                    runOnUiThread(() -> {
                        TextView textProgress = findViewById(R.id.brp_launcher_load_progress_titile);
                        textProgress.setText("Загрузка завершена. Распаковка...");
                    });
                    UnZip();

                    runOnUiThread(() -> {
                        TextView textProgress = findViewById(R.id.brp_launcher_load_progress_titile);
                        textProgress.setText("Распаковка завершена!");
                    });

                } catch (IOException e) {
                    runOnUiThread(() -> Toast.makeText(this, "Ошибка при сохранении файла: " + e.getMessage(), Toast.LENGTH_SHORT).show());
                    e.printStackTrace();
                }
            } catch (Exception e) {
                runOnUiThread(() -> Toast.makeText(this, "Ошибка загрузки: " + e.getMessage(), Toast.LENGTH_SHORT).show());
                e.printStackTrace();
            }
        }).start();
    }

    @SuppressLint("DefaultLocale")
    private void updateProgress(int chet, int length) {
        float progressPercentage = (((float) chet) / length) * 100.0f;

        TextView textprogress3 = findViewById(R.id.brp_launcher_load_progress_text);
        TextView textprogress = findViewById(R.id.brp_launcher_load_progress_text_2);
        ProgressBar progressbar = findViewById(R.id.brp_launcher_load_progress_bar);

        textprogress3.setText(String.format("%.1f%%", progressPercentage));
        textprogress.setText(String.format("%s из %s",
                Utils.bytesIntoHumanReadable(chet),
                Utils.bytesIntoHumanReadable(length)));

        progressbar.setProgress((int) progressPercentage);
    }


    public void UnZip() {
        try {
            // Use the path where the ZIP file is stored in the app's private storage
            File zipFile = new File(getExternalFilesDir(null), "downloads/cache.zip");
            File destinationDirectory = new File(Config.path_cache);

            // Create the destination directory if it does not exist
            if (!destinationDirectory.exists()) {
                destinationDirectory.mkdirs();
            }

            // Extract the ZIP file
            new ZipFile(zipFile).extractAll(Config.path_cache);

            // Delete the ZIP file after extraction
            if (zipFile.exists()) {
                zipFile.delete();
            }

        } catch (ZipException e) {
            runOnUiThread(() -> {
                Toast toast = Toast.makeText(this, "Unzip err!", Toast.LENGTH_LONG);
                toast.show();
            });
            e.printStackTrace();
        }

        // Launch MainActivity and transition
        startActivity(new Intent(this, MainActivity.class));
        overridePendingTransition(0, 0);

        finish();
        runOnUiThread(() -> {
            Toast toast = Toast.makeText(this, "END!", Toast.LENGTH_LONG);
            toast.show();
        });
    }


    @Override
    public void onBackPressed() {
        Log.i("LoadActivity", "onBackPressed");
    }

}
