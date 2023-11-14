package com.physi.dev.nursinglight;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.widget.Toast;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

public class IntroActivity extends AppCompatActivity {

    private static final int REQ_APP_PERMISSION = 201;
    private final List<String> appPermissions = new LinkedList<>(
            Arrays.asList(
                    android.Manifest.permission.INTERNET,
                    android.Manifest.permission.ACCESS_COARSE_LOCATION,
                    android.Manifest.permission.ACCESS_FINE_LOCATION,
                    android.Manifest.permission.CHANGE_WIFI_STATE,
                    android.Manifest.permission.ACCESS_WIFI_STATE,
                    android.Manifest.permission.BLUETOOTH,
                    android.Manifest.permission.BLUETOOTH_ADMIN)
    );

    private void checkPermissions(){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            appPermissions.add(android.Manifest.permission.BLUETOOTH_SCAN);
            appPermissions.add(Manifest.permission.BLUETOOTH_CONNECT);
        }

        final List<String> reqPermissions = new LinkedList<>();
        for(String permission : appPermissions){
            if(checkSelfPermission(permission) == PackageManager.PERMISSION_DENIED){
                reqPermissions.add(permission);
            }
        }
        if(reqPermissions.size() != 0){
            requestPermissions(reqPermissions.toArray(new String[reqPermissions.size()]), REQ_APP_PERMISSION);
        } else {
            startNextActivity();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQ_APP_PERMISSION) {
            boolean accessStatus = true;
            for (int grantResult : grantResults) {
                if (grantResult == PackageManager.PERMISSION_DENIED) {
                    accessStatus = false;
                    break;
                }
            }

            if (!accessStatus) {
                Toast.makeText(getApplicationContext(), "APP PERMISSION DENIED.", Toast.LENGTH_SHORT).show();
                finish();
            } else {
                startNextActivity();
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_intro);

        checkPermissions();
    }

    @Override
    public void onBackPressed() {
//        super.onBackPressed();
    }

    private void startNextActivity(){
        new Handler().postDelayed(() -> {
            startActivity(new Intent(IntroActivity.this, ConnectActivity.class));
            finish();
        }, 1000);
    }
}