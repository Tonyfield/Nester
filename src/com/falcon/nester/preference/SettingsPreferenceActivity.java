package com.falcon.nester.preference;

import java.util.List;

import com.falcon.nesSimon.R;
import com.falcon.nesSimon.R.xml;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class SettingsPreferenceActivity extends PreferenceActivity {

	final static String TAG     = "SettingsPreferenceActivity";
	final static String ACTION_PREFS_SCREEN     = "com.falcon.nesSimon.preference.PREFS_SCREEN";
	final static String ACTION_PREFS_AUDIO      = "com.falcon.nesSimon.preference.PREFS_AUDIO";
	final static String ACTION_PREFS_CONTROLLER = "com.falcon.nesSimon.preference.PREFS_CONTROLLER";
	final static String ACTION_PREFS_NETWORK    = "com.falcon.nesSimon.preference.PREFS_NETWORK";
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
		{
			return;
		}
		String action = getIntent().getAction();
	    if (action == null )
	    {
	        // Load the legacy preferences headers
	        addPreferencesFromResource(R.xml.settings_header_legacy);
	    }else if( action.equals(ACTION_PREFS_SCREEN)) 
    	{
    		addPreferencesFromResource(R.xml.screen_settings);
    	}else if(action.equals(ACTION_PREFS_AUDIO))
    	{
    		addPreferencesFromResource(R.xml.audio_settings); 		
    	}else if(action.equals(ACTION_PREFS_CONTROLLER))
    	{
    		addPreferencesFromResource(R.xml.pad_settings); 	
    	}else if(action.equals(ACTION_PREFS_NETWORK))
    	{	
   
	    }
		 
	}

	/* ���� Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB ʱ��
	 *  onBuildHeaders ������
	 */
	@Override
	public void onBuildHeaders(List<Header> target) {
		Log.v(TAG, "onBuildHeaders()");
		loadHeadersFromResource(R.xml.settings_header, target);
		super.onBuildHeaders(target);
	}

	public static class ScreenSettingsFragment extends PreferenceFragment {
	    @Override
	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.screen_settings);

	    }
	}
	public static class AudioSettingsFragment extends PreferenceFragment {
	    @Override
	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.audio_settings);

	    }
	}
	
	public static class PadSettingsFragment extends PreferenceFragment {
	    @Override
	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pad_settings);

	    }
	}
	public static class NetworkSettingsFragment extends PreferenceFragment {
	    @Override
	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.audio_settings);

	    }
	}

}
