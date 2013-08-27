package com.falcon.nester.preference;

import com.falcon.nesSimon.R;

import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.util.Log;

public class SettingsPreferenceFragment extends PreferenceFragment {

	private static final String TAG = "SettingsPreferenceFragment";
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		Log.v(TAG, "onCreate");
		addPreferencesFromResource(R.xml.settings);
	}

}
