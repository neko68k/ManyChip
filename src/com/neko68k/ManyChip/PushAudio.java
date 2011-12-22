package com.neko68k.ManyChip;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

// consumer
public class PushAudio extends Thread{
	AudioTrack track;
	   //short[] buffer = new short[1024];	  
	private boolean playing = false;
	private boolean paused = false;
	   public PushAudio(String threadName)
	   {
		   setName(threadName);
	      int minSize =AudioTrack.getMinBufferSize( 44100, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT );        
	      track = new AudioTrack( AudioManager.STREAM_MUSIC, 44100, 
	                                        AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, 
	                                        minSize, AudioTrack.MODE_STREAM);	      
	       
	   }	   
	 
	    

	   /*public PushAudio( String threadName, ManyChipUpdateThread producer ) 
	   { 
	      setName( threadName ); 
	      theProducer = producer; 
	   } */
	   
	   public void PlayStart()
	   {		   
		   track.play();
		   //theProducer.PlayStart();
		   playing = true;
		   if(this.getState() == Thread.State.NEW)
		   {
			   this.start();
		   }
	   }
	   public void PlayQuit(){	   
		   track.stop();	
		   		   		   
		   //theProducer.PlayQuit();
		   paused = true;
		   playing = false;
	   }
	   public void PlayStop(){
		   track.stop();
		   //theProducer.PlayStop();
		   NDKBridge.StopEngine();
		   paused = true;
		   playing = false;		   
		   
	   }
	   public void PlayPause(){		   
		   track.pause();
		   paused = true;
		   //playing = false;
		   //theProducer.PlayPause();
	   }
	   
	   public void UnPause(){		   
		   track.play();		
		   paused = false;
		   //playing = false;
		   //theProducer.PlayUnPause();
	   }
	   
	   public void run() 
	   { 
	      //byte buffer[]; 

	      
		   while( playing == true ){
			   if(paused == false){
			     //buffer = theProducer.take_product(); 
		         //NDKBridge.PSF_Execute();
				   NDKBridge.Play(0);
		      } 	      
		   }
		   track.release();
	   } 
	   
	   public void write(byte buffer[], int bytes){
		   track.write( buffer, 0,  bytes ); 
	   }
} 


