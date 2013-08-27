package com.falcon.nester;

import java.io.File;
import java.util.Locale;

import com.falcon.nesSimon.R;
import com.falcon.nester.preference.SettingsPreferenceActivity;


import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

public class MainActivity extends Activity {

	private static final String TAG = "MainActivity";
	private static final int SELECT_FILE_REQUEST = 1;
	private static final int START_GAME_REQUEST = 2;
	private static final String LAST_NES_PATH = "last_nes_path";
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.main);
		
		this.findViewById(R.id.btn_open).setOnClickListener(mBtnClickListener);
		this.findViewById(R.id.btn_settings).setOnClickListener(mBtnClickListener);
		this.findViewById(R.id.btn_quit).setOnClickListener(mBtnClickListener);
		this.findViewById(R.id.btn_about).setOnClickListener(mBtnClickListener);

	}
	
	View.OnClickListener mBtnClickListener = new View.OnClickListener(){
		public void onClick(View v) {
			Intent intent;
	        // Do something in response to button click
	    	switch( v.getId() )
	    	{
	    	case R.id.btn_open:
	    		String nespath = getDefaultNesPath();
	    		File f = new File(nespath);
	    		if( false == f.exists() || false == f.isDirectory() )
	    		{
	    			setDefaultNesPath(Environment.getExternalStorageDirectory().getPath());
	    		}
				intent = new Intent(MainActivity.this,FileListActivity.class);
				intent.putExtra(FileListActivity.PARAMETER_FILEPATH, nespath);
				intent.putExtra(FileListActivity.PARAMETER_SUFFIX, "nes");
				startActivityForResult(intent, SELECT_FILE_REQUEST);
				break;
	    	case R.id.btn_settings:
				intent = new Intent(MainActivity.this,SettingsPreferenceActivity.class);
				startActivity(intent);
	    		break;
	    	case R.id.btn_quit:
	    		Log.v(TAG, "Quit game");
	    		MainActivity.this.finish();
	    		break;
	    	case R.id.btn_about:
				intent = new Intent(MainActivity.this,AboutActivity.class);
				startActivity(intent);
	    		break;

	    	}
	   	}
	};
	
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
				
				Log.v(TAG, "onActivityResult():SELECT_FILE_REQUEST");
				
				/* BEGIN  save the last nes file path*/
				String nesfile = selectedUri.toString().toLowerCase(Locale.getDefault()).replace("file:///mnt", "/mnt");
				nesfile = Uri.decode(nesfile);
				setDefaultNesPath( nesfile.substring(0, nesfile.lastIndexOf('/')));
				/* END  save the last nes file path*/
				Intent intent = new Intent(this,NesActivity.class);
				Log.v(TAG, "onActivityResult():nesfile:"+nesfile);
				intent.setData(selectedUri);
				startActivityForResult(intent, START_GAME_REQUEST);

//				boolean ret =  selectedUri.toString().replace("file:///mnt", "/mnt"));
			}
			break;
		case START_GAME_REQUEST:
//			Toast.makeText(this, 
//					getString(R.string.warning_loadgame_failed)+":"+selectedUri.toString(),
//					Toast.LENGTH_SHORT).show();
			break;
		default:
			break;
		}
		
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
