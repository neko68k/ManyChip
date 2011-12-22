package com.neko68k.ManyChip;
//import android.R;
import com.neko68k.ManyChip.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.widget.RemoteViews;


// this will handle all the threading and shit
// so we can properly stop it and allow the app
// to shut down or otherwise not keep sucking
// up the battery

public class PlayerService extends Service{
	private NotificationManager mNM;
	
	private int NOTIFICATION = 1;
	String text;
	Notification notification = null;
	PendingIntent contentIntent = null;
	
	AudioDevice ad = new AudioDevice("deviceThread");	

	public class LocalBinder extends Binder{
		PlayerService getService(){
			return PlayerService.this;
		}
	}
	
	@Override
	public void onCreate(){
		//mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
		play();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId){
		return START_STICKY;
	}
	
	public void pause(){
		ad.PlayPause();
	}
	public void unpause(){
		ad.UnPause();
	}
	public void stop(){
		ad.PlayQuit();
		//mNM.cancelAll();
	}
	public void play(){
		
		if(notification == null)
			notification=new Notification(android.R.drawable.star_on,
	                "",
	                System.currentTimeMillis());
		//Intent i=new Intent(getApplicationContext(), ManyChip.class);
		setNoteText();
		ad.PlayStart();
		//showNotification();
	}
	
	public void setNoteText(){

		notification.flags|=Notification.FLAG_NO_CLEAR;		
		RemoteViews contentView = new RemoteViews(getPackageName(), R.layout.custom_notification);
		contentView.setImageViewResource(R.id.image, android.R.drawable.star_on);
		contentView.setTextViewText(R.id.title, "ManyChip");
		
		
		notification.contentView = contentView;
		
		
		//text = NDKBridge.getSong(NDKBridge.getCurrentCmd());
		if(text!=null){
			contentView.setTextViewText(R.id.text, "");
			contentView.setTextViewText(R.id.text2, "");
		}
		if(text==null){
			contentView.setTextViewText(R.id.text, "No track list");
			contentView.setTextViewText(R.id.text2, ""); 
		}
		
		

		contentIntent = PendingIntent.getActivity(this, 0, 
				new Intent(this, ManyChip.class).setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|
		Intent.FLAG_ACTIVITY_SINGLE_TOP), 0);
		
		Intent notificationIntent = new Intent(this, ManyChip.class);
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
		notification.contentIntent = contentIntent;
				
		startForeground(1337, notification);	
	}
	
	//@Override
	public void onDestory(){
		//mNM.cancelAll();//(NOTIFICATION);
		ad.PlayQuit();
	}
	
	@Override
	public IBinder onBind(Intent intent){
		return mBinder;
	}
	
	private final IBinder mBinder = new LocalBinder();
	
	private void showNotification(){
		
		 //notification = new Notification(R.drawable.ic_media_play, text, System.currentTimeMillis());
		 
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}
