package com.falcon.nester;

import java.io.*;
import java.nio.CharBuffer;

import org.apache.http.util.EncodingUtils;

import com.falcon.nesSimon.R;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.*;

public class AboutActivity extends Activity {
	
	public static final String TAG = "AboutActivity";
	ScrollView scvInfo = null;
	Button btnIntroduce = null;
	Button btnLicense = null;
	Button btnLegal = null;
	Button btnUpdate = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.about);
		
		View.OnClickListener onClickListener = new View.OnClickListener(){
			public void onClick(View v)
			{
				btnIntroduce.setBackgroundResource(R.drawable.btn_bg_blue3);
				btnLicense.setBackgroundResource(R.drawable.btn_bg_blue3);
				btnLegal.setBackgroundResource(R.drawable.btn_bg_blue3);
				btnUpdate.setBackgroundResource(R.drawable.btn_bg_blue3);
				v.setBackgroundResource(R.drawable.btn_bg_blue6);
				switch(v.getId())
				{
				case R.id.btn_introduce:
					setAboutInfo(R.string.introduce, R.string.introduce_asset);
					break;
				case R.id.btn_license:
					setAboutInfo(R.string.license, R.string.license_asset);
					break;
				case R.id.btn_legal:
					setAboutInfo(R.string.legal,R.string.legal_asset);
					break;
				case R.id.btn_update:
					break;
				}
//				TextView tv = (TextView)findViewById(R.id.about_info);
//				tv.invalidate();
//				scvInfo.invalidate();
			}
		};
		
		setAboutInfo(R.string.introduce, R.string.introduce_asset);
		//setAboutInfo(R.string.license, R.string.license_asset);
		
		scvInfo = (ScrollView)this.findViewById(R.id.scrollview_info);
		btnIntroduce = (Button)this.findViewById(R.id.btn_introduce);
		btnIntroduce.setOnClickListener(onClickListener);
		btnLicense = (Button)this.findViewById(R.id.btn_license);
		btnLicense.setOnClickListener(onClickListener);
		btnLegal = (Button)this.findViewById(R.id.btn_legal);
		btnLegal.setOnClickListener(onClickListener);
		btnUpdate = (Button)this.findViewById(R.id.btn_update);		
		btnUpdate.setOnClickListener(onClickListener);
	}

	private void setAboutInfo(int titleResId, int infoResId)
	{
		setAboutInfo(this.getString(titleResId),this.getString(infoResId));

	}
	private void setAboutInfo(String title, String assetFile)
	{
//		int offset = 0;
//		int len;
		//char[] buff = new char[32*1024];
		((TextView)this.findViewById(R.id.about_title)).setText(title); 

		try {
			InputStream is = getAssets().open(assetFile);  
			// We guarantee that the available method returns the total  
			// size of the asset...  of course, this does mean that a single  
			// asset can't be more than 2 gigs.  
			int size = is.available();  
			// Read the entire asset into a local byte buffer.  
			byte[] buffer = new byte[size];  
			is.read(buffer);  
			is.close();
			// Finally stick the string into the text view.  
			TextView tv = (TextView)findViewById(R.id.about_info);
			tv.setText(	EncodingUtils.getString(buffer,"GB2312")); 

	//		scvInfo.setOverScrollMode(ScrollView.OVER_SCROLL_IF_CONTENT_SCROLLS);
			
			
		} catch (IOException e) {
			e.printStackTrace();
		}

	}
}
