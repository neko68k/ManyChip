package com.neko68k.ManyChip;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;



public class ManyChip extends Activity {
	
	boolean paused = false;
	boolean playing = false;
	boolean mIsBound = false;
	AudioDevice ad;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        if(!mIsBound){
			doBindService();
		}
        
        //NDKBridge.Load("/emmc/chips/NSF/Kirbys Adventure.nsf");
        //NDKBridge.Load("/emmc/chips/GYM/Streets of Rage/05 Moon Beach.vgm");
        //NDKBridge.Load("/emmc/chips/PSF/FFT/122 brave story.minipsf");
        //NDKBridge.Load("/emmc/chips/PSF/Gran Turismo/arcade.psf");
        //NDKBridge.Load("/emmc/chips/GSF/mmbn5gsf/12 Boss Battle.minigsf");   
        
        
        // handle push audio for PSF/GSF files
       /* if(NDKBridge.GetDecoder()==6||NDKBridge.GetDecoder()==7){
        	//NDKBridge.playerService = new PlayerService();
        	
        	
        	NDKBridge.pa = new PushAudio("pushAudioThd");
        	NDKBridge.pa.PlayStart();
        }
        else{
        	//ad = new AudioDevice("deviceThread");
        	NDKBridge.playerService = new PlayerService();
        	NDKBridge.Play(1);
        	NDKBridge.playerService.play();  
        }*/
        
        //ad.PlayStart();
         
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }
    
    
 // service connection stuff
    private ServiceConnection mConnection = new ServiceConnection(){    	
    	public void onServiceConnected(ComponentName className, IBinder service){
    		NDKBridge.playerService = ((PlayerService.LocalBinder)service).getService();
    	}
    	
    	public void onServiceDisconnected(ComponentName className){
    		NDKBridge.playerService = null;
    	}
    };
    
    void doBindService(){
    	bindService(new Intent(ManyChip.this, PlayerService.class), mConnection, Context.BIND_AUTO_CREATE);
    	mIsBound = true;
    }
    
    void doUnbindService(){
    	if(mIsBound){
    		unbindService(mConnection);
    		mIsBound = false;
    	}
    }
    
    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data){
	    super.onActivityResult(requestCode, resultCode, data);
    	
    	if(resultCode == RESULT_OK){
    		// open mc
	    	if(requestCode == 65535){    
	    		
	            
	    		
	    		if(NDKBridge.pa != null){
	    			NDKBridge.pa.PlayStop();
	    			NDKBridge.pa = null;
	    		}
	    		else if(NDKBridge.playerService != null){
	    			NDKBridge.playerService.stop();
	    			NDKBridge.playerService = null;
	    		}
	    		String fn = data.getStringExtra("com.neko68k.ManyChip.FN");
	    		NDKBridge.Load(fn);   
	    		
	            // handle push audio for PSF/GSF files
	            if(NDKBridge.GetDecoder()==6||NDKBridge.GetDecoder()==7){
	            	//NDKBridge.playerService = new PlayerService();
	            	
	            	if(NDKBridge.pa == null){
	            		NDKBridge.pa = new PushAudio("pushAudioThd");
	            	}
	            	NDKBridge.pa.PlayStart();
	            }
	            else{
	            	//ad = new AudioDevice("deviceThread");
	            	
	            	if(NDKBridge.playerService == null){
	            		NDKBridge.playerService = new PlayerService();
	            	}
	            	NDKBridge.Play(1);
	            	NDKBridge.playerService.play();  
	            }
	    	}
    	}
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.open:
        	Intent intent = new Intent().setClass(this, FileBrowser.class);
        	startActivityForResult(intent, 65535);
            return true;  
        default:
            return super.onOptionsItemSelected(item);
        }
    }
}
    
    
    
    
    
    
    
    