package com.example.ledfx;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.core.app.NotificationCompat;

public class AudioCaptureForegroundService extends Service {
    private static final String TAG = "DuplexStreamFS";
    public static final String ACTION_START = "ACTION_START";
    public static final String ACTION_STOP = "ACTION_STOP";
    public static final String CHANNEL_ID = "AudioCaptureForegroundService";

    @Override
    public void onCreate(){

    }

    @Override
    public IBinder onBind(Intent intent) {
        // We don't provide binding, so return null
        return null;
    }

    private Notification buildNotification() {
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

            manager.createNotificationChannel(new NotificationChannel(
                    CHANNEL_ID,
                    "Ledfx notification",
                    NotificationManager.IMPORTANCE_NONE));

            return new NotificationCompat.Builder(this, CHANNEL_ID)
                    .build();

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Receive onStartCommand" + intent);
        switch (intent.getAction()) {
            case ACTION_START:
                Log.i(TAG, "Receive ACTION_START" + intent.getExtras());
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    startForeground(123, buildNotification(),
                            ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK
                                    | ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE);
                }
                break;
            case ACTION_STOP:
                Log.i(TAG, "Receive ACTION_STOP" + intent.getExtras());
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    stopForeground(STOP_FOREGROUND_REMOVE);
                }
                break;
        }
        return START_NOT_STICKY;
    }
}
