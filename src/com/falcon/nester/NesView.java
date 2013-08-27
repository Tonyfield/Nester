package com.falcon.nester;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import com.falcon.nesSimon.R;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.Rect;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class NesView extends SurfaceView 
implements SurfaceHolder.Callback
{

	public static final String TAG = "NesView";
	public static final int SCALE_TYPE_FIT = 0;
	public static final int SCALE_TYPE_1X = 1;
	public static final int SCALE_TYPE_2X = 2;

	private int mLastScaleType;
	private int mScaleType;
	
	private Context mContext;

	private float mScaleValue;
	private float mStartX;
	private float mStartY;
	SurfaceHolder mHolder;

	private Bitmap mNesBitmap;
	private Bitmap mNesControllerDir; // up/down/left/start key
	private Bitmap mNesControllerSelect; // Select  key
	private Bitmap mNesControllerStart; // Start  key
	private Bitmap mNesControllerA; // right-hand  A/B key
	private Bitmap mNesControllerB; // right-hand  A/B key

	void printSamples(MotionEvent ev) {
		final int historySize = ev.getHistorySize();
		final int pointerCount = ev.getPointerCount();
		Log.v(TAG, String.format("  historySize :%d: pointerCount=%d, action = %d", historySize, pointerCount, ev.getAction())); 
		for (int h = 0; h < historySize; h++) {
			Log.v(TAG, String.format(" historical At time %d:", ev.getHistoricalEventTime(h)));
			for (int p = 0; p < pointerCount; p++) { 
				Log.v(TAG, String.format(" historical(%d) pointer id:%d: (%f,%f)", 
						h, ev.getPointerId(p), ev.getHistoricalX(p, h), ev.getHistoricalY(p, h))); 
			}  
		}
		System.out.printf("At time %d:", ev.getEventTime());
		for (int p = 0; p < pointerCount; p++) {
			Log.v(TAG, String.format("current pointer(%d) id:%d: (%f,%f)", p, ev.getPointerId(p), ev.getX(p), ev.getY(p))); 
		} 
	}

	public NesView(Context context)
	{
		super(context);
		mContext = context;
		mStartX = mStartY = 0.0f;
		mLastScaleType = -1;
		mScaleType = SCALE_TYPE_FIT;
		mScaleValue = 1.0f;
		mHolder = getHolder();
		mHolder.addCallback(this);
	
		mNesControllerDir = BitmapFactory.decodeResource(context.getResources(), R.drawable.nes_controller_dir);
		mNesControllerSelect = BitmapFactory.decodeResource(context.getResources(), R.drawable.nes_controller_select);
		mNesControllerStart = BitmapFactory.decodeResource(context.getResources(), R.drawable.nes_controller_start);
		mNesControllerA = BitmapFactory.decodeResource(context.getResources(), R.drawable.nes_controller_a);
		mNesControllerB = BitmapFactory.decodeResource(context.getResources(), R.drawable.nes_controller_b);
		//setWillNotDraw(false);
	}

	@Override
	public void onWindowFocusChanged(boolean hasWindowFocus) {
		NesSimu ns = ((NesActivity)mContext).mNesSimu;
		if( hasWindowFocus ){
			ns.resume();
			Log.v(TAG, "onWindowFocusChanged( TRUE )");
		} else {
			ns.pause();
			Log.v(TAG, "onWindowFocusChanged( FALSE )");
		}
		super.onWindowFocusChanged(hasWindowFocus);
	}

	public boolean SetScaleType(int scaleType)
	{
		Log.v(TAG, String.format("SetScaleType(%d)", scaleType));
		switch( scaleType )
		{
		case SCALE_TYPE_FIT:
		case SCALE_TYPE_1X:
		case SCALE_TYPE_2X:
			mScaleType = scaleType;
			return true;
		}
		return false;
	}

	public void surfaceCreated(SurfaceHolder holder)
	{
//			outputDebugInfo("surfaceCreated,");
		Log.v(TAG, "surfaceCreated()");
	}
	
	public void surfaceChanged(SurfaceHolder pHolder, 
			int pFormat,
			int pWidth,
			int pHeight) 
	{
		Log.v(TAG,"surfaceChanged called");
		
		SharedPreferences prefs =
				PreferenceManager.getDefaultSharedPreferences(mContext);
		String strSize = prefs.getString(
				mContext.getString(R.string.key_nes_video_size_option),
				mContext.getString(R.string.video_size_options_default_value));
		SetScaleType(Integer.parseInt(strSize));
		UpdateScaleType();
		((NesActivity)mContext).mNesInput.updateVirtualPadLayout(this, pWidth, pHeight);
		
    }

	public void surfaceDestroyed(SurfaceHolder holder) 
	{
		Log.v(TAG,"surfaceDestroyed called");
	}
	
	public void UpdateScaleType() 
	{
        Log.v(TAG, String.format(" mScaleType = %d", mScaleType));
        switch( mScaleType )
        {
        case SCALE_TYPE_1X:
        	mScaleValue = 1.0f;
        	break;
        case SCALE_TYPE_2X:
        	mScaleValue = 2.0f;
        	break;
        case SCALE_TYPE_FIT:
        default:
    		float sx = (float)this.getWidth()/(float)NesSimu.getWidth();
    		float sy = (float)this.getHeight()/(float)NesSimu.getHeight();
    		mScaleValue = (sx < sy) ? sx : sy ;
        	break;
        }
        mLastScaleType = mScaleType;
        if( this.getWidth() > NesSimu.getWidth()*mScaleValue )
        {
        	mStartX = (this.getWidth() - (float)NesSimu.getWidth()*mScaleValue)/2.0f/mScaleValue;
        } else {
        	mStartX = 0;
        }
        Log.v(TAG, String.format("view size = (%d,%d) NesSimu.getWidth()=%d,mScaleValue = %f, mStartX = %f",
        		this.getWidth(),this.getHeight(), NesSimu.getWidth(), mScaleValue, mStartX));
        if( this.getHeight()*0.8f > NesSimu.getHeight()*mScaleValue )
        {
        	mStartY = (this.getHeight()*0.8f - (float)NesSimu.getHeight()*mScaleValue)/2.0f/mScaleValue;
        } else {
        	mStartY = 0;
        }
	}

	//�ڲ�����ڲ���  
    class ShowNesThread implements Runnable{  
    	public ShowNesThread(Bitmap nesBitmap)
    	{
    		mNesBitmap = nesBitmap;
    	}
        @Override  
        public void run() {  
//        	Log.v(TAG, "ShowNesThread.run() "+mHolder.toString());
            Canvas canvas = mHolder.lockCanvas(null);//��ȡ����  
            NesInput in = ((NesActivity)mContext).mNesInput;
            if( null == canvas )
            {
            	return;
            }
            Paint mPaint = new Paint();
            
            canvas.drawRect(NesView.this.getLeft(),NesView.this.getTop(), 
            		NesView.this.getRight(),NesView.this.getBottom(),mPaint);

            canvas.scale(mScaleValue, mScaleValue);      
            canvas.drawBitmap(mNesBitmap, mStartX, mStartY, mPaint);
            
            canvas.scale(1.0f/mScaleValue, 1.0f/mScaleValue);
    		SharedPreferences prefs =
    				PreferenceManager.getDefaultSharedPreferences((NesActivity)mContext);		
			int pad_alpha = prefs.getInt(
					((NesActivity)mContext).getString(R.string.key_nes_virtual_pad_transparency_option),
					50);

            mPaint.setAlpha(pad_alpha*255/100);
            canvas.drawBitmap(mNesControllerDir, null, in.mRectCtlDir, mPaint);

/*            if( null != mViewBitmap )
            {
            	canvas.drawBitmap(mViewBitmap, 0.0f, 0.0f, mPaint);
            }
*/
            canvas.drawBitmap(mNesControllerSelect,null, in.mRectCtlSelect, mPaint);
            canvas.drawBitmap(mNesControllerStart,null, in.mRectCtlStart, mPaint);
            canvas.drawBitmap(mNesControllerA,null, in.mRectCtlA, mPaint);
            canvas.drawBitmap(mNesControllerB,null, in.mRectCtlB, mPaint);
            mPaint.setAlpha(0xFF);
            mHolder.unlockCanvasAndPost(canvas);//�������ύ���õ�ͼ��  
         }     
    }  
    
    public void UpdateGame(Bitmap nesBitmap) 
	{
    	if(View.VISIBLE == getWindowVisibility() && this.getWidth()>0 )
    	{
            if( mLastScaleType != mScaleType )
            {
            	UpdateScaleType();
            }
    		new Thread(new ShowNesThread(nesBitmap)).start();
    	}
	}
    
	public Bitmap getNesScreen()
	{
    	return mNesBitmap;
	}

}
