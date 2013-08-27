package com.falcon.nester;

import java.nio.ByteBuffer;

import com.falcon.nesSimon.R;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.Bitmap.Config;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;

public class NesInput implements OnTouchListener, OnKeyListener {

	public static final String TAG = "NesInput";
	
	//  input device type
	public static final int INPUT_DEVICE_TOUCHPANEL = 0;
	public static final int INPUT_DEVICE_KEYBOARD = 1;
	public static final int INPUT_DEVICE_PAD = 2;
	
	public static final int INPUT_DEVICE_MAX = INPUT_DEVICE_PAD;
	public static final int INPUT_DEVICE_DEFALT = INPUT_DEVICE_TOUCHPANEL;
	
	public static final int PAD_INPUT_A_ID = 0;
	public static final int PAD_INPUT_B_ID = 1;
	public static final int PAD_INPUT_SELECT_ID = 2;
	public static final int PAD_INPUT_START_ID = 3;
	public static final int PAD_INPUT_UP_ID = 4;
	public static final int PAD_INPUT_DOWN_ID = 5;
	public static final int PAD_INPUT_LEFT_ID = 6;
	public static final int PAD_INPUT_RIGHT_ID = 7;
	public static final int PAD_INPUT_X_ID = 8;
	public static final int PAD_INPUT_Y_ID = 9;
	public static final int PAD_INPUT_MAX_ID = 9;
	public static final int PAD_INPUT_NULL_ID = 0xFF;

	public static final int PAD_INPUT_NULL = 0;
	public static final int PAD_INPUT_A = ( 1 << PAD_INPUT_A_ID );
	public static final int PAD_INPUT_B = ( 1 << PAD_INPUT_B_ID );
	public static final int PAD_INPUT_SELECT = ( 1  << PAD_INPUT_SELECT_ID );
	public static final int PAD_INPUT_START = ( 1  << PAD_INPUT_START_ID );
	public static final int PAD_INPUT_UP = (1  << PAD_INPUT_UP_ID );
	public static final int PAD_INPUT_DOWN = (1 << PAD_INPUT_DOWN_ID );
	public static final int PAD_INPUT_LEFT = (1 << PAD_INPUT_LEFT_ID );
	public static final int PAD_INPUT_RIGHT = (1 << PAD_INPUT_RIGHT_ID );
	public static final int PAD_INPUT_X = ( 1 << PAD_INPUT_X_ID );
	public static final int PAD_INPUT_Y = ( 1 << PAD_INPUT_Y_ID );

	public static final int KEYCODE_MAX = 0xFF;
	
	private int mWidth;
	private int mHeight;
	private ByteBuffer mTouchMaskBitmapBuf = null;
	public Rect   mRectCtlDir;
	public Rect   mRectCtlSelect;
	public Rect   mRectCtlStart;
	public Rect   mRectCtlA;
	public Rect   mRectCtlB;
	private PadState mPadState;


	// keyboard input map
	int[] mapKey2Input; // �� keyCode ӳ�䵽 inputID
	int[] mapInput2Key; // �� inputID ӳ�䵽 keyCode
	
	int mInputDevice;
//	NesSimu mNesSimu;
	Context mContext;

	public NesInput(Context context)
	{
		this(context, INPUT_DEVICE_DEFALT );
	}
	public NesInput(Context context, int inputDevice)
	{
		mContext = context;
//		Log.v(TAG, String.format("mNesSimu = %x, inputDevice = %x",mNesSimu, inputDevice));
		setInputDeviceType(inputDevice);
		
		mWidth = 0;
		mHeight = 0;

		// touch panel device
		mRectCtlDir = new Rect();
		mRectCtlSelect = new Rect();
		mRectCtlStart = new Rect();
		mRectCtlA = new Rect();
		mRectCtlB = new Rect();
		mPadState = new PadState();

		// keyboard device
		mapInput2Key = new int[16];
		mapKey2Input = new int[256];
		for(int k=0 ; k<256; k++)
		{
			mapKey2Input[k] = PAD_INPUT_NULL;
		}
		mapKey2Input[KeyEvent.KEYCODE_I] = PAD_INPUT_UP_ID;
		mapInput2Key[PAD_INPUT_UP_ID] = KeyEvent.KEYCODE_I;

		mapKey2Input[KeyEvent.KEYCODE_K] = PAD_INPUT_DOWN_ID;
		mapInput2Key[PAD_INPUT_DOWN_ID] = KeyEvent.KEYCODE_K;
		
		mapKey2Input[KeyEvent.KEYCODE_J] = PAD_INPUT_LEFT_ID;
		mapInput2Key[PAD_INPUT_LEFT_ID] = KeyEvent.KEYCODE_J;
		
		mapKey2Input[KeyEvent.KEYCODE_L] = PAD_INPUT_RIGHT_ID;
		mapInput2Key[PAD_INPUT_RIGHT_ID] = KeyEvent.KEYCODE_L;

		mapKey2Input[KeyEvent.KEYCODE_A] = PAD_INPUT_A_ID;
		mapInput2Key[PAD_INPUT_A_ID] = KeyEvent.KEYCODE_A;

		mapKey2Input[KeyEvent.KEYCODE_S] = PAD_INPUT_B_ID;
		mapInput2Key[PAD_INPUT_B_ID] = KeyEvent.KEYCODE_S;
		
		mapKey2Input[KeyEvent.KEYCODE_X] = PAD_INPUT_START_ID;
		mapInput2Key[PAD_INPUT_START_ID] = KeyEvent.KEYCODE_X;
		
		mapKey2Input[KeyEvent.KEYCODE_Z] = PAD_INPUT_SELECT_ID;
		mapInput2Key[PAD_INPUT_SELECT_ID] = KeyEvent.KEYCODE_Z;
		
		
	}

	public int setInputDeviceType(int inputDevice)
	{
		mInputDevice = (inputDevice > INPUT_DEVICE_MAX)?INPUT_DEVICE_MAX:inputDevice;
		return mInputDevice;
	}
	public int getInputDeviceType(int inputDevice)
	{
		return mInputDevice;
	}

	private boolean processMotionEvent(MotionEvent event) {
    	//printSamples(event);
    	float x,y;
    	int k;
		int pad1Input;
    	int action = event.getAction() & MotionEvent.ACTION_MASK;
    	int index;
    	int count = event.getPointerCount(); 
    	NesSimu ns = ((NesActivity)mContext).mNesSimu;
    	//printSamples(event);
    	
		switch(action)
		{
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
		case MotionEvent.ACTION_MOVE:
		//case MotionEvent.ACTION_HOVER_MOVE:
		{
			pad1Input = 0;
			for(index=0 ; index<count; index++ )
			{
				x = event.getX(index);
				y = event.getY(index);
				k = (int)x+(int)y*mWidth;
				
				if(0 < k && k < mTouchMaskBitmapBuf.capacity() && null != ns )
				{
					pad1Input |= mTouchMaskBitmapBuf.get(k);
					ns.setPad1Input(0xFF, pad1Input);
				}else{
					Log.v(TAG, String.format("error location capacity = %d, k = %d",mTouchMaskBitmapBuf.capacity(), k ));
				}
			}
			//mNesSimu.setPad1Input(0xFF, mPadState.getPadVal());
			break;	
		}
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
		case MotionEvent.ACTION_CANCEL:
			index = event.getActionIndex();
			x = event.getX(index);
			y = event.getY(index);
			k = (int)x+(int)y*mWidth;
			if(0 < k && k < mTouchMaskBitmapBuf.capacity() && null != ns )
			{
				pad1Input = mTouchMaskBitmapBuf.get((int)x+(int)y*mWidth);
				ns.setPad1Input(pad1Input, 0);
			}else{
				Log.v(TAG, String.format("error location capacity = %d, k = %d",mTouchMaskBitmapBuf.capacity(), k ));
			}
			break;
		default:
			break;
		}
		return true;
	}
	private boolean processMotionEvent2(MotionEvent event) {
    	//printSamples(event);
    	float x,y;
		int pad1Input;
    	int action = event.getAction() & MotionEvent.ACTION_MASK;
    	int index = event.getActionIndex();
    	int id = event.getPointerId(index);
    	NesSimu ns = ((NesActivity)mContext).mNesSimu;
    	//printSamples(event);
    	
		switch(action)
		{
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
		case MotionEvent.ACTION_MOVE:
		//case MotionEvent.ACTION_HOVER_MOVE:
		{
			x = event.getX(index);
			y = event.getY(index);
//			mFocusin = true;
			pad1Input = mTouchMaskBitmapBuf.get((int)x+(int)y*mWidth);
			Log.v(TAG, String.format(" DOWN: id = %d, pad1Input = %02X, ",id, pad1Input));
			if( null != ns)
			{
				mPadState.setCoor(id, x, y, pad1Input);
				//ns.setPad1Input(0xFF, pad1Input);
				ns.setPad1Input(0xFF, mPadState.getPadVal());
			}else{
				Log.v(TAG,"mPadState = null");
			}
			//mNesSimu.setPad1Input(0xFF, mPadState.getPadVal());
			break;	
		}
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
			x = event.getX(index);
			y = event.getY(index);
//			mFocusin = true;
			pad1Input = mTouchMaskBitmapBuf.get((int)x+(int)y*mWidth);
			Log.v(TAG, String.format(" UP: id = %d, pad1Input = %02X, ",id, pad1Input));
			if( null != ns )
			{
				mPadState.setCoor(id, x, y, 0);
				ns.setPad1Input(0xFF, mPadState.getPadVal());
				//ns.setPad1Input(pad1Input, 0);
			}else{
				Log.v(TAG,"mPadState = null");
			}

			break;
		default:
			break;
		}
		return true;
	}

	public void updateVirtualPadLayout(View view, int width,int height)
	{
		Rect vRect = new Rect();
        Path path = new Path();

        int tmpWidth, tmpHeight;
		if( view.getGlobalVisibleRect (vRect) )
		{
			Log.v(TAG, String.format("vRect.left=%d,vRect.right=%d��vRect.top=%d vRect.bottom= %d",
					vRect.left, vRect.right, vRect.top, vRect.bottom));
			mWidth = (vRect.right+1);
			mHeight = (vRect.bottom+1);
			if( null != mTouchMaskBitmapBuf )
			{
				mTouchMaskBitmapBuf = null;
			}
			mTouchMaskBitmapBuf = ByteBuffer.allocate(mWidth * mHeight );
			Bitmap touchMaskBitmap = Bitmap.createBitmap(mWidth, mHeight, Config.ALPHA_8);
			
			Canvas cv = new Canvas(touchMaskBitmap); 
			
			Paint paint = new Paint();

			int maxLen = (vRect.width()> vRect.height())?vRect.width():vRect.height();
	        tmpWidth = tmpHeight = maxLen/4;
			mRectCtlDir.left = vRect.left;
			mRectCtlDir.bottom = vRect.bottom;
			mRectCtlDir.right = mRectCtlDir.left + (tmpWidth - 1);
			mRectCtlDir.top = mRectCtlDir.bottom - (tmpHeight - 1);
			int centerX = mRectCtlDir.centerX();
			int centerY = mRectCtlDir.centerY();

			tmpWidth  = mRectCtlDir.width()*1/5;  //  = (1- 1/5)/2
			tmpHeight = mRectCtlDir.height()*1/5;
		
			// �ϼ�
			paint.setAlpha(PAD_INPUT_UP);
			path.reset();
	        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
	        path.lineTo(mRectCtlDir.right-tmpWidth, mRectCtlDir.top);  
	        path.lineTo(mRectCtlDir.left+tmpWidth, mRectCtlDir.top); 
	        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����
	        cv.drawPath(path, paint); 
	        
	        // ���
			paint.setAlpha(PAD_INPUT_LEFT);
			path.reset();
	        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
	        path.lineTo(mRectCtlDir.left, mRectCtlDir.top+tmpHeight);  
	        path.lineTo(mRectCtlDir.left, mRectCtlDir.bottom-tmpHeight);  
	        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
	        cv.drawPath(path, paint); 

			// �¼�
			paint.setAlpha(PAD_INPUT_DOWN);
			path.reset();
	        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
	        path.lineTo(mRectCtlDir.left+tmpWidth, mRectCtlDir.bottom);  
	        path.lineTo(mRectCtlDir.right-tmpWidth, mRectCtlDir.bottom); 
	        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
	        cv.drawPath(path, paint); 

			// �Ҽ�
			paint.setAlpha(PAD_INPUT_RIGHT);
			path.reset();
	        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
	        path.lineTo(mRectCtlDir.right, mRectCtlDir.bottom-tmpHeight);  
	        path.lineTo(mRectCtlDir.right, mRectCtlDir.top+tmpHeight); 
	        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
	        cv.drawPath(path, paint); 
 
    		SharedPreferences prefs =
    				PreferenceManager.getDefaultSharedPreferences((NesActivity)mContext);		
			boolean has4directions = prefs.getBoolean(
					((NesActivity)mContext).getString(R.string.key_nes_pad_direction_option),
					false);
			if( !has4directions )
			{
				// ���ϼ�
				paint.setAlpha(PAD_INPUT_LEFT|PAD_INPUT_UP);
				path.reset();
				path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
		        path.lineTo(mRectCtlDir.left+tmpWidth, mRectCtlDir.top);  
		        path.lineTo(mRectCtlDir.left, mRectCtlDir.top+tmpHeight);  
		        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
		        cv.drawPath(path, paint);
		        
		        // ���¼�
				paint.setAlpha(PAD_INPUT_LEFT|PAD_INPUT_DOWN);
				path.reset();
		        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
		        path.lineTo(mRectCtlDir.left, mRectCtlDir.bottom-tmpHeight);  
		        path.lineTo(mRectCtlDir.left+tmpWidth, mRectCtlDir.bottom); 
		        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
		        cv.drawPath(path, paint); 

				// ���¼�
				paint.setAlpha(PAD_INPUT_RIGHT|PAD_INPUT_DOWN);
				path.reset();
		        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
		        path.lineTo(mRectCtlDir.right-tmpWidth, mRectCtlDir.bottom); 
		        path.lineTo(mRectCtlDir.right, mRectCtlDir.bottom-tmpHeight);  
		        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
		        cv.drawPath(path, paint); 

				// ���ϼ�
				paint.setAlpha(PAD_INPUT_RIGHT|PAD_INPUT_UP);
				path.reset();
		        path.moveTo(centerX, centerY);// �˵�Ϊ����ε����   
		        path.lineTo(mRectCtlDir.right, mRectCtlDir.top+tmpHeight); 
		        path.lineTo(mRectCtlDir.right-tmpWidth, mRectCtlDir.top);  
		        path.close(); // ʹ��Щ�㹹�ɷ�յĶ����   
		        cv.drawPath(path, paint); 

			}
						
	        // Select key
			tmpWidth =  maxLen / 8;
			tmpHeight = tmpWidth / 2;
			mRectCtlSelect.right = vRect.centerX();
			mRectCtlSelect.bottom = vRect.bottom;
			mRectCtlSelect.top = mRectCtlSelect.bottom - (tmpHeight - 1);
			mRectCtlSelect.left = mRectCtlSelect.right - (tmpWidth - 1);
			Log.v(TAG, String.format(" select button rect = %d, %d, %d, %d", 
					mRectCtlSelect.left, mRectCtlSelect.right,mRectCtlSelect.top, mRectCtlSelect.bottom));
			paint.setAlpha(PAD_INPUT_SELECT);
	        cv.drawRect(mRectCtlSelect, paint);

	        // Start key
			tmpWidth =  maxLen / 8;
			tmpHeight = tmpWidth / 2;
			mRectCtlStart.left = mRectCtlSelect.right;
			mRectCtlStart.bottom = vRect.bottom;
			mRectCtlStart.top = mRectCtlStart.bottom - (tmpHeight - 1);
			mRectCtlStart.right = mRectCtlStart.left + (tmpWidth -1);
			Log.v(TAG, String.format(" start button rect = %d, %d, %d, %d", 
					mRectCtlStart.left, mRectCtlStart.right,mRectCtlStart.top, mRectCtlStart.bottom));
			paint.setAlpha(PAD_INPUT_START);
			cv.drawRect(mRectCtlStart, paint);
		
	        // A key
			tmpWidth = tmpHeight = maxLen / 10;
			mRectCtlA.right = vRect.right - 20;
			mRectCtlA.bottom = vRect.bottom - 20 - tmpHeight;
			mRectCtlA.left = mRectCtlA.right - (tmpWidth-1);
			mRectCtlA.top = mRectCtlA.bottom - (tmpHeight-1);
			Log.v(TAG, String.format(" A button rect = %d, %d, %d, %d", 
					mRectCtlA.left, mRectCtlA.right,mRectCtlA.top, mRectCtlA.bottom));
			paint.setAlpha(PAD_INPUT_A);
	        cv.drawRect(mRectCtlA, paint);

	        // B key
			tmpWidth = tmpHeight =  maxLen / 10;
			mRectCtlB.right = vRect.right - 20 - tmpWidth;
			mRectCtlB.bottom = vRect.bottom - 20;
			mRectCtlB.left = mRectCtlB.right - (tmpWidth-1);
			mRectCtlB.top = mRectCtlB.bottom - (tmpHeight-1);
			Log.v(TAG, String.format(" B button rect = %d, %d, %d, %d", 
					mRectCtlB.left, mRectCtlB.right,mRectCtlB.top, mRectCtlB.bottom));
			paint.setAlpha(PAD_INPUT_B);
	        cv.drawRect(mRectCtlB, paint);
			
	        cv.save(Canvas.ALL_SAVE_FLAG);  
	        cv.restore();
	        touchMaskBitmap.copyPixelsToBuffer(mTouchMaskBitmapBuf);
		}
	}	
	public boolean setKeyboardMap(int keyCode, int input )
	{
		if( keyCode <= KEYCODE_MAX 
				&& input <=PAD_INPUT_MAX_ID 
				&& keyCode != KeyEvent.KEYCODE_HOME )
		{
			int oldInput = mapKey2Input[keyCode];
			int oldKeyCode = mapInput2Key[input];
			if( oldKeyCode != keyCode )
			{
				// ����ϵ�
				mapKey2Input[oldKeyCode] = PAD_INPUT_NULL;
				if( oldInput <= PAD_INPUT_MAX_ID )
				{  // ���������ֵ��Ч�����
					mapInput2Key[oldInput] = KeyEvent.KEYCODE_UNKNOWN;
				}
				mapKey2Input[keyCode] = input;
				mapInput2Key[input] = keyCode;
			}
			return true;
		}
		return false;
	}

	@Override
	public boolean onKey(View view, int keyCode, KeyEvent arg2) {
		if( mInputDevice == INPUT_DEVICE_KEYBOARD )
		{
			int input_id = mapKey2Input[keyCode];
			if(input_id <= PAD_INPUT_MAX_ID )
			{
				NesSimu ns = ((NesActivity)mContext).mNesSimu;
				ns.SetInput(1<<input_id);
			}
			return true;
		}
		return false;
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {

		if( mInputDevice == INPUT_DEVICE_TOUCHPANEL )
		{
			processMotionEvent(event);
			return true;
		}
		return false;
	}
	private class PadState {
		public float[] mPressX;
		public float[] mPressY;
		public int[] mPadVal;
		private static final int MAX_ONTOUCH_NUM = 8;  // 4 may be meet the requirement.
		public PadState()
		{
			int k;
			
			mPressX = new float[MAX_ONTOUCH_NUM];
			mPressY = new float[MAX_ONTOUCH_NUM];
			mPadVal = new int[MAX_ONTOUCH_NUM];
			
			for( k=0 ; k< MAX_ONTOUCH_NUM ; k++)
			{
				mPadVal[k] = 0;
			}
		}

		public void setCoor(int id, float x, float y, int padval)
		{
			if( id >= MAX_ONTOUCH_NUM )
			{
				return;
			}
			mPressX[id] = x;
			mPressY[id] = y;
			mPadVal[id] = padval;
		}
		
		public int getPadVal()
		{
			return mPadVal[0] |mPadVal[1] | mPadVal[2] | mPadVal[3]; 
		}
	}
}
