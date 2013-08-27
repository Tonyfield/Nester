package com.falcon.nester;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import com.falcon.nesSimon.R;
import com.falcon.nester.preference.SettingsPreferenceActivity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnCancelListener;
import android.content.SharedPreferences.Editor;
import android.content.pm.ActivityInfo;

import android.graphics.Bitmap;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.*;
import android.widget.Toast;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 * 
 * @see SystemUiHider
 */
public class NesActivity extends Activity {
	/**
	 * Whether or not the system UI should be auto-hidden after
	 * {@link #AUTO_HIDE_DELAY_MILLIS} milliseconds.
	 */
	private static final String TAG = "NesActivity";
	private static final int SELECT_FILE_REQUEST = 1;
	private static final String LAST_NES_PATH = "last_nes_path";

	private static final boolean AUTO_HIDE = true;

//	private Bitmap mBackBuffer;
	public NesSimu mNesSimu;
	public NesInput mNesInput;
	private NesView mNesView;
	private MediaPlayer mediaPlayer;

    // A handler on the UI thread.
    private Handler mHandler;
    private String mNesFilename;

	public native void showFrame(Bitmap pTarget, byte[] pSource);

	/**
	 * If {@link #AUTO_HIDE} is set, the number of milliseconds to wait after
	 * user interaction before hiding the system UI.
	 */
	private static final int AUTO_HIDE_DELAY_MILLIS = 3000;

	/**
	 * If set, will toggle the system UI visibility upon interaction. Otherwise,
	 * will show the system UI visibility upon interaction.
	 */
	private static final boolean TOGGLE_ON_CLICK = true;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_nes);

//		final View controlsView = findViewById(R.id.fullscreen_content_controls);
		final ViewGroup contentView = (ViewGroup)findViewById(R.id.fullscreen_content);

		// Set up an instance of SystemUiHider to control the system UI for
		// this activity.

		// Upon interacting with UI controls, delay any scheduled hide()
		// operations to prevent the jarring behavior of controls going away
		// while interacting with the UI.
//		findViewById(R.id.dummy_button).setOnTouchListener(
//				mDelayHideTouchListener);
//		WindowManager.LayoutParams p = this.getWindow().getAttributes();
	    mHandler = new Handler();

		mNesView = new NesView(this);
		mNesSimu = new NesSimu(this, mNesView);
		mNesInput = new NesInput(this);
		mNesView.setOnTouchListener(mNesInput);
		mNesView.setOnKeyListener(mNesInput);
		contentView.addView(mNesView);
		
		Intent intent = this.getIntent();
		if (intent != null)
		{
			Log.d(TAG, " intent.getDataString()= " + intent.getDataString());
			
			String nesfile = intent.getDataString().toLowerCase(Locale.getDefault()).replace("file:///mnt", "/mnt");
			mNesFilename = Uri.decode(nesfile);
			setDefaultNesPath( mNesFilename.substring(0, mNesFilename.lastIndexOf('/')));
			if( mNesSimu.isRamExisted(nesfile) )
			{
				showLoadPrompt(mNesFilename);
			} else {
				boolean ret =  mNesSimu.loadGame(mNesFilename,false);
				if( false == ret )
				{
					Toast.makeText(this, 
							getString(R.string.warning_loadgame_failed)+":"+mNesFilename,
							Toast.LENGTH_LONG).show();
				}
			}
		} else {
			Log.d(TAG, " is invoked without an intent");
		}

		//((ViewGroup)findViewById(R.id.nes_frame)).addView(mNesView);
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		Log.v(TAG, "onDestroy");
		mNesSimu.destroy(false);
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		Log.v(TAG, "onPause");
		mNesSimu.pause();
		super.onPause();
	}
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		Log.v(TAG, "onResume");
		super.onResume();

		SharedPreferences prefs =
				PreferenceManager.getDefaultSharedPreferences(this);		
		try{
			boolean mute = prefs.getBoolean(
					getString(R.string.key_nes_mute_option),
					false);
			boolean soundAccompaniment = prefs.getBoolean(
					getString(R.string.key_nes_audio_option),
					true);
			int vol = prefs.getInt(
					getString(R.string.key_nes_volume_option),
					50);

		Log.v(TAG, String.format("mute = %d, soundAcc=%d, vol = %d", mute?1:0,soundAccompaniment?1:0, vol ));
			if( mute || (false == soundAccompaniment) )
			{
				mNesSimu.setMute(true);
				if(false == mute)
				{
					String music_idx = prefs.getString(
							getString(R.string.key_nes_other_music_option),
							"R.raw.xs_hbl_gh");
/*					if("xs_hbl_gh.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.xs_hbl_gh);
					}else
					if("xs_hbl_wsdh.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.xs_hbl_wsdh);
					}else
					if("mu_jdw_yygmldcs.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.mu_jdw_yygmldcs);
					}else
					if("mu_lsj_cyzy.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.mu_lsj_cyzy);
					}else
					if("mu_zy_mlh.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.mu_zy_mlh);
					}else
					if("mu_xxf_blq.mp3".equals(music_idx) )
					{
						playLocalAudio(R.raw.mu_xxf_blq);
					}else
					if("http://listen.radionomy.com/Radio-Mozart".equals(music_idx) )
					{
						playInternetAudio(music_idx);
					}
*/	
					playInternetAudio("http://listen.radionomy.com/Radio-Mozart",(float)vol/100.0f);
				}
			} else {
				mNesSimu.setVolume((float)vol/100.0f);
				mNesSimu.setMute(false);
			}

			String strSize = prefs.getString(
					this.getString(R.string.key_nes_video_orientation_option),
					this.getString(R.string.video_orientation_options_default_value));
			switch(Integer.parseInt(strSize))
			{
			case 0:
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
				break;
			case 1:
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
				break;
			default:
				break;
			}
		}catch(Exception e)
		{
			e.printStackTrace();
		}
		mNesSimu.resume();
	}
	
	private void showTips(){

		AlertDialog alertDialog = new AlertDialog.Builder(NesActivity.this)
		.setTitle(this.getText(R.string.quit_game_title))
		.setMessage(this.getText(R.string.quit_game_prompt))
		.setPositiveButton(
				this.getText(android.R.string.ok), 
				new DialogInterface.OnClickListener() {

					public void onClick(DialogInterface dialog, int which){
						NesActivity.this.finish();
					}
				})
		.setNegativeButton(
				this.getText(android.R.string.cancel),
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						return;
					}
				})
		.create();  //�����Ի���

		alertDialog.show(); // ��ʾ�Ի���

	}

	private void showLoadPrompt(final String nesfile)
	{
		AlertDialog alertDialog = new AlertDialog.Builder(NesActivity.this)
		.setTitle(this.getText(R.string.load_ram))
		.setMessage(this.getText(R.string.load_ram_msg))
        .setPositiveButton(
        		android.R.string.yes,
        		new DialogInterface.OnClickListener() 
	            {
        			String mNesFile = nesfile;
	                public void onClick(DialogInterface dialog, int which) {
                   	// when prompt load game
	                	Log.v( TAG, "showLoadPrompt:" + mNesFile );
	            		boolean ret =  mNesSimu.loadGame(mNesFile, true);
//		     			mNesSimu.setMute(true);
	            		if( false == ret )
	            		{
	            			Toast.makeText(NesActivity.this, 
	            					getString(R.string.warning_loadgame_failed),
	            					Toast.LENGTH_LONG).show();
	            		}
	                }
	            })
        .setNegativeButton(
        		android.R.string.no,
        		new DialogInterface.OnClickListener() {
        			String mNesFile = nesfile;
	                public void onClick(DialogInterface dialog, int which) {
            			boolean ret =  mNesSimu.loadGame(mNesFile, false);
//		            	mNesSimu.setMute(true);
            			if( false == ret )
            			{
            				Toast.makeText(NesActivity.this, 
            						getString(R.string.warning_loadgame_failed),
            						Toast.LENGTH_LONG).show();
            			}
	                }
	            })
        .create(); //�����Ի���

		alertDialog.show(); // ��ʾ�Ի���

	}

	private void showSavePrompt(){

		AlertDialog alertDialog = new AlertDialog.Builder(NesActivity.this)
		.setTitle(this.getText(R.string.save_ram))
		.setMessage(this.getText(R.string.save_ram_msg))
		.setPositiveButton(
				android.R.string.ok, 
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						if( false == mNesSimu.saveSRAM(mNesFilename+".srm"))
            			{
            				Toast.makeText(NesActivity.this, 
            						getString(R.string.warning_saveram_failed),
            						Toast.LENGTH_LONG).show();
            			}
					}
				})
		.setNegativeButton(
				android.R.string.no,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						return;
					}
				})
		.create();  //�����Ի���

		alertDialog.show(); // ��ʾ�Ի���

	}
	private void showQuitPrompt(){

		AlertDialog alertDialog = new AlertDialog.Builder(NesActivity.this)
		.setTitle(this.getText(R.string.quit_game_title))
		.setMessage(this.getText(R.string.quit_game_prompt))
		.setPositiveButton(
				android.R.string.ok, 
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						mNesSimu.stop(false);
						NesActivity.this.finish();
					}
				})
		.setNeutralButton(
				R.string.save_and_quit,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						mNesSimu.stop(true);
						NesActivity.this.finish();
					}
				})
		.setNegativeButton(
				android.R.string.cancel,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which){
						return;
					}
				})
		.create();  //�����Ի���

		alertDialog.show(); // ��ʾ�Ի���

	}
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {

		if(keyCode==KeyEvent.KEYCODE_BACK && event.getRepeatCount()==0){
	
		this.showTips();
	
		return false;
	
		}
	
		return false;

	}


	private void playInternetAudio(String url, float vol) throws Exception
	{
		killMediaPlayer();
		mediaPlayer = new MediaPlayer();
		mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
		mediaPlayer.setDataSource(url);
		mediaPlayer.setVolume(vol, vol);
		mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener(){
			public void onPrepared(MediaPlayer mp) {
				mp.start();
			}
		});
		mediaPlayer.prepareAsync();
	}
	private void playLocalAudio(int musicResId, float vol) throws Exception
	{
		killMediaPlayer();
		mediaPlayer = MediaPlayer.create(this, musicResId);
		mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
		// calling prepare() is not required in this case
		mediaPlayer.setVolume(vol, vol);
		mediaPlayer.start();
	}
	private void killMediaPlayer() {
		Log.v(TAG, "killMediaPlayer");
		if(mediaPlayer!=null) {
			try {
				mediaPlayer.release();		
			}
			catch(Exception e) {
				e.printStackTrace();
			}
		}
	}
	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		// TODO Auto-generated method stub
		switch( item.getItemId() )
		{
		case R.id.menu_open:
			/*	intent = new Intent(Intent.ACTION_PICK);*/
			Intent intent = new Intent(this,FileListActivity.class);
			intent.putExtra(FileListActivity.PARAMETER_FILEPATH, getDefaultNesPath());
			intent.putExtra(FileListActivity.PARAMETER_SUFFIX, "nes");
			startActivityForResult(intent, SELECT_FILE_REQUEST);
			return true;
		case R.id.menu_load:
			/*	intent = new Intent(Intent.ACTION_PICK);*/
			mNesSimu.loadSRAM(mNesFilename+".srm");			
			return true;
		case R.id.menu_save:
			if( false == mNesSimu.saveSRAM(mNesFilename+".srm"))
			{
				Toast.makeText(NesActivity.this, 
						getString(R.string.warning_saveram_failed),
						Toast.LENGTH_LONG).show();
			}			
			return true;
		case R.id.menu_settings:
			intent = new Intent(this,SettingsPreferenceActivity.class);
			startActivity(intent); 
			return true;
		case R.id.menu_reset:
			mNesSimu.reset();
			return true;
		case R.id.menu_prtscn:
			boolean ret = false;
			Bitmap bmp = mNesView.getNesScreen();
			// Ĭ��ͼƬ�ļ�Ŀ¼
		    File dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
		    if(!dir.exists()) {  
		    	dir.mkdirs();	
		    }
		    // �� ǰ׺��nes_��+��ǰʱ��   ��Ϊ��ͼ�ļ���
			Date date= new Date();
			SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd_hmmss");
		    File file = new File(dir, "nes_" + sdf.format(date) + ".png");
		    try
		    {
		    	file.createNewFile();
			    FileOutputStream fOut = new FileOutputStream(file);
	
			    ret = bmp.compress(Bitmap.CompressFormat.PNG, 85, fOut);
			    fOut.flush();
			    fOut.close();
		    }catch(Exception e){
		    	e.printStackTrace();
		    	Toast.makeText(this, 
		    			String.format(getString(R.string.warning_print_screen_failed),dir.getPath()),
		    			Toast.LENGTH_LONG)
		    			.show();
		    }finally{
		    	if( ret )
		    	{
		    		Toast.makeText(this, 
		    			String.format(getString(R.string.prompt_print_screen_succeed),dir.getPath()),
		    			Toast.LENGTH_LONG)
		    			.show();
		    	}else{
			    	Toast.makeText(this, 
			    			String.format(getString(R.string.warning_print_screen_failed),dir.getPath()),
			    			Toast.LENGTH_LONG)
			    			.show();
		    	}
		    }

			return true;
		case R.id.menu_quit:
			showQuitPrompt();
			//mNesSimu.stop();
			//this.finish();
			return true;
		}
		return super.onMenuItemSelected(featureId, item);
	}


	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		
		switch( requestCode )
		{
		case SELECT_FILE_REQUEST:
			if( RESULT_OK == resultCode )
			{
				Uri selectedUri = data.getData();
				Log.d(TAG, "The output uri:" + selectedUri.toString());
				String nesfile = selectedUri.toString().toLowerCase(Locale.getDefault()).replace("file:///mnt", "/mnt");
				mNesFilename = Uri.decode(nesfile);
				setDefaultNesPath( mNesFilename.substring(0, mNesFilename.lastIndexOf('/')));
				
				if( mNesSimu.isRamExisted(mNesFilename+".srm") )
				{
					showLoadPrompt(mNesFilename);
				} else {
					boolean ret =  mNesSimu.loadGame(mNesFilename,false);
					if( false == ret )
					{
						Toast.makeText(this, 
								getString(R.string.warning_loadgame_failed)+":"+mNesFilename,
								Toast.LENGTH_LONG).show();
					}
				}
			}
			break;
		default:
			break;
		}
		
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		getMenuInflater().inflate(R.menu.nes_menu, menu);
		return true;
	}
	
	public void setDefaultNesPath(String nesPath)
	{
		File dir = new File(nesPath);
		if( dir.exists() && dir.isDirectory() && dir.canRead() )
		{
			SharedPreferences myPrefs = getPreferences(MODE_PRIVATE);
			Editor editor = myPrefs.edit();
			editor.putString(LAST_NES_PATH, nesPath);
			editor.commit();
		}
	}
	public String getDefaultNesPath()
	{
		SharedPreferences myPrefs = getPreferences(MODE_PRIVATE);
		return myPrefs.getString(LAST_NES_PATH, Environment.getExternalStorageDirectory().getPath());
	}

}
