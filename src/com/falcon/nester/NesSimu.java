package com.falcon.nester;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;

public class NesSimu {
	static {
		System.loadLibrary("NesSimon");
	}
	public static final String TAG = "NesSimu";
	
	public static final int PAD_INPUT_A = ( 1 << 0 );
	public static final int PAD_INPUT_B = ( 1 << 1 );
	public static final int PAD_INPUT_SELECT = ( 1  << 2 );
	public static final int PAD_INPUT_START = ( 1  << 3 );
	public static final int PAD_INPUT_UP = (1  << 4 );
	public static final int PAD_INPUT_DOWN = (1 << 5 );
	public static final int PAD_INPUT_LEFT = (1 << 6 );
	public static final int PAD_INPUT_RIGHT = (1 << 7 );
	
	private int mPadInput;
	private Context mContext;

	Bitmap mNesBitmap;
	NesView mDisplayView;
	
	public native boolean Initialize(Bitmap pTarget);
	public native void destroy(boolean bSaveRam);
	public native boolean loadGame(String romFilename, boolean bLoadRam);
	public native boolean reset();
	
	public native boolean isRamExisted(String rom);
	public native boolean loadSRAM(String filename);
	public native boolean saveSRAM(String filename);
	public String saveSRAM2(String filename)
	{
		
		File ramfile = new File(filename);

		try{
			if(!ramfile.exists()) {
				ramfile.createNewFile();
			}
		}
	    catch (IOException e){
	        return e.getMessage();
	    }
		try{
			FileOutputStream outStream = new FileOutputStream(ramfile);
			byte[] sram = getSaveRAM();
			outStream.write(sram);
			outStream.close();
		} catch (FileNotFoundException e) {
	        e.printStackTrace();
	        return e.getMessage();
	    }
	    catch (IOException e){
	        return "error 3";
	    }
		return "ok";
	}
	private native byte[] getSaveRAM();
	
	public native void setMute(boolean bMute);
	/* vol range in [0.0, 1.0] */
	public native void setVolume(float vol);
	public native void stop(boolean bSaveRAM);
	public native void pause();
	public native void resume();
	public native static int getWidth();
	public native static int getHeight();
	public native int setPad1Input(int padMask, int padValue);
	public native int setPad2Input(int padMask, int padValue);
	public static native String getDebugInfo();
	
	public NesSimu(Context cxt, NesView displayView)
	{

		mContext = cxt;
		mDisplayView = displayView;
		mPadInput = 0;
		
		Log.v(TAG, String.format("w,h = %d, %d", displayView.getWidth(), displayView.getHeight()));

		mNesBitmap = Bitmap.createBitmap(getWidth(), getHeight(),Bitmap.Config.ARGB_8888);
		
		Initialize(mNesBitmap);
	}

	public void SetInput(int padInput)
	{
		mPadInput = padInput;		
	}

	public void UpdateView()
	{
		if( null != mDisplayView )
		{
			mDisplayView.post( new Runnable(){
				public void run(){
					mDisplayView.UpdateGame(mNesBitmap);
				}				
			});
		}
	}

}
