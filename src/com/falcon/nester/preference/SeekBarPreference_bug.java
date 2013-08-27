package com.falcon.nester.preference;

import com.falcon.nesSimon.R;

import android.content.Context;
import android.content.res.TypedArray;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

public class SeekBarPreference_bug extends Preference 
implements OnSeekBarChangeListener
{
    
    private final String TAG = "SeekBarPreference";
    
    private static final String ANDROIDNS="http://schemas.android.com/apk/res/android";
    private static final String SEEKBAR_PREFS="com.falcon.SeekBarPreference";
    private static final int DEFAULT_VALUE = 50;
    
    Context mContext;
	private int mMaxValue      = 100;
	private int mMinValue      = 0;
	private int mInterval      = 1;
	private int mCurrentValue  = 50;
	private String mUnitsLeft  = "";
	private String mUnitsRight = "";
	private SeekBar mSeekBar;
    
	private TextView mCurrentValueText;

    public SeekBarPreference_bug(Context context, AttributeSet attrs) {
        super(context, attrs);
        initPreference(context, attrs);
    }

    public SeekBarPreference_bug(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initPreference(context, attrs);
    }

    private void initPreference(Context context, AttributeSet attrs) {
    	Log.v(TAG,"initPreference");
    	mContext = context;
        updateValuesFromXml(attrs);
        this.setWidgetLayoutResource(R.layout.preference_widget_seekbar);
    }
    
    private void updateValuesFromXml(AttributeSet attrs) {
        mMaxValue = attrs.getAttributeIntValue(SEEKBAR_PREFS, "max", 100);
        mMinValue = attrs.getAttributeIntValue(SEEKBAR_PREFS, "min", 0);
       
        mUnitsLeft = getAttributeStringValue(attrs, SEEKBAR_PREFS, "unitsLeft", "");
        String value = getAttributeStringValue(attrs, SEEKBAR_PREFS, "value", "");
        mUnitsRight = getAttributeStringValue(attrs, SEEKBAR_PREFS, "unitsRight", "");
        
        Log.v(TAG, String.format("(%d, %d)v=%s,", mMinValue, mMaxValue,value));
        try {
        	mInterval = attrs.getAttributeIntValue(SEEKBAR_PREFS, "interval", 1);
//            String newInterval = attrs.getAttributeValue(SEEKBAR_PREFS, "interval");
            //if(newInterval != null)
            //    mInterval = Integer.parseInt(newInterval);
            if( mInterval <= 0 )
            {
            	mInterval = 1;
            }
        }
        catch(Exception e) {
            Log.e(TAG, "Invalid interval value", e);
        }
        
    }
    
    private String getAttributeStringValue(AttributeSet attrs, String namespace, String name, String defaultValue) {
        String value = attrs.getAttributeValue(namespace, name);
        if(value == null)
            value = defaultValue;
        
        return value;
    }
    
    @Override
    protected View onCreateView(ViewGroup parent){
    	Log.v(TAG, "onCreateView()");
        Log.v(TAG, String.format("onCreateView()cls = %s, %s", parent.toString(), parent.getClass() ));
        return super.onCreateView(parent);
    }
    
    @Override
    public void onBindView(View view) {
        super.onBindView(view);
        
        Log.v(TAG, String.format("onBindView()l = %d, r = %d", view.getLeft(), view.getRight() ));
//        RelativeLayout layout = (RelativeLayout)view;
        View layout = view;

        Log.v(TAG, String.format("onBindView()cls = %s, %s", view.toString(), view.getClass() ));
        mSeekBar = (SeekBar)layout.findViewById(R.id.seekBar1);
        //mSeekBar.setMinimumWidth(minWidth);

        mCurrentValueText = (TextView)layout.findViewById(R.id.seekBarPrefValue);
        if( null != mCurrentValueText ){
        	mCurrentValueText.setText(String.valueOf(mCurrentValue));
        	mCurrentValueText.setMinimumWidth(30);
        }
        if( null != mSeekBar )
        {
        	mSeekBar.setMax(mMaxValue - mMinValue);
        	mSeekBar.setProgress(mCurrentValue - mMinValue);
        	mSeekBar.setOnSeekBarChangeListener(this);
        }

        TextView unitsRight = (TextView)layout.findViewById(R.id.seekBarPrefUnitsRight);
        if( null != unitsRight ){
        	unitsRight.setText(mUnitsRight);
        }
        
        TextView unitsLeft = (TextView)layout.findViewById(R.id.seekBarPrefUnitsLeft);
        if( null != unitsLeft ){
        	unitsLeft.setText(mUnitsLeft);  
        }

    }
    
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        int newValue = progress + mMinValue;
        
        if(newValue > mMaxValue)
            newValue = mMaxValue;
        else if(newValue < mMinValue)
            newValue = mMinValue;
        else if(mInterval != 1 && newValue % mInterval != 0)
            newValue = Math.round(((float)newValue)/mInterval)*mInterval;  
        
        // change rejected, revert to the previous value
        if(!callChangeListener(newValue)){
            seekBar.setProgress(mCurrentValue - mMinValue); 
            return; 
        }

        // change accepted, store it
        mCurrentValue = newValue;
        if( null != mCurrentValueText ){
        	mCurrentValueText.setText(String.valueOf(newValue));
        }
        persistInt(newValue);

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        notifyChanged();
    }


    @Override 
    protected Object onGetDefaultValue(TypedArray ta, int index){
        
        int defaultValue = ta.getInt(index, DEFAULT_VALUE);
        return defaultValue;
        
    }

    @Override
    protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {

        if(restoreValue) {
            mCurrentValue = getPersistedInt(mCurrentValue);
        }
        else {
            int temp = 0;
            try {
                temp = (Integer)defaultValue;
            }
            catch(Exception ex) {
                Log.e(TAG, "Invalid default value: " + defaultValue.toString());
            }
            
            persistInt(temp);
            mCurrentValue = temp;
        }
        
    }
    
} 

