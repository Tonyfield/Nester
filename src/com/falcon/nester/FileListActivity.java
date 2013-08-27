/**
 * 
 */
package com.falcon.nester;

import java.io.File;
import java.io.FilenameFilter;
import java.text.DateFormat;
import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import com.falcon.nesSimon.R;

import android.app.ListActivity;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;

/**
 * @author Tracy
 *
 */
public class FileListActivity extends ListActivity 
implements SimpleAdapter.ViewBinder, FilenameFilter
{
	public static final String RETURN_FILENAME = "filename";
	public static final String PARAMETER_FILEPATH = "filePath";
	public static final String PARAMETER_SUFFIX = "suffix";
	
	private static final String DEFAULT_FILEPATH = Environment.getRootDirectory().getPath();
	private static final String TAG = "FileListActivity";

	private static final String FILE_ICON= "fileIcon";
	private static final String FILE_NAME = "fileName";
//	private static final String FILE_DESC = "fileDescription";
	private static final String FILE_ATTR = "fileAttribute";
	
//	private LinearLayout mParentContainer;
//	private ListView mFileList;
	private String mFilePath = DEFAULT_FILEPATH;
	private String mSuffix = "*";
//	private String mCurrentPath = DEFAULT_FILEPATH;
	
	private ArrayList<Map<String, Object>> mFileMapList;
	private static final Map<String, Integer> mFile2IconTbl;

	static {
		mFile2IconTbl = new HashMap<String, Integer>();
		// text type
		mFile2IconTbl.put(".txt",  R.drawable.file_txt);
		mFile2IconTbl.put(".c",    R.drawable.file_c);
		mFile2IconTbl.put(".cpp",  R.drawable.file_cpp);
		mFile2IconTbl.put(".h",    R.drawable.file_h);
		mFile2IconTbl.put(".hpp",  R.drawable.file_hpp);
		mFile2IconTbl.put(".java", R.drawable.file_java);
		// internet file type
		mFile2IconTbl.put(".htm",  R.drawable.file_html);
		mFile2IconTbl.put(".html", R.drawable.file_html);
		mFile2IconTbl.put(".xhtml",R.drawable.file_html);
		mFile2IconTbl.put(".php",  R.drawable.file_html);
		mFile2IconTbl.put(".xml",  R.drawable.file_xml);
		// pdf type
		mFile2IconTbl.put(".pdf",  R.drawable.file_pdf);
		// bin type
		mFile2IconTbl.put(".bin",  R.drawable.file_bin);
		// apk type
		mFile2IconTbl.put(".apk",  R.drawable.file_apk);
		// eps type
		mFile2IconTbl.put(".eps",  R.drawable.file_eps);
		// nes type
		mFile2IconTbl.put(".nes",  R.drawable.file_nes);
		// zip type
		mFile2IconTbl.put(".zip",  R.drawable.file_zip);
		mFile2IconTbl.put(".rar",  R.drawable.file_zip);
		mFile2IconTbl.put(".7z",  R.drawable.file_zip);
		// package file
		mFile2IconTbl.put(".tgz",  R.drawable.file_tgz);
		mFile2IconTbl.put(".jar",  R.drawable.file_jar);
		// image type
		mFile2IconTbl.put(".bmp", R.drawable.file_image);
		mFile2IconTbl.put(".jpg", R.drawable.file_image);
		mFile2IconTbl.put(".jpeg",R.drawable.file_image);
		mFile2IconTbl.put(".gif", R.drawable.file_image);
		mFile2IconTbl.put(".png", R.drawable.file_image);
		mFile2IconTbl.put(".tiff",R.drawable.file_image);
		// audio type
		mFile2IconTbl.put(".mid", R.drawable.file_audio);
		mFile2IconTbl.put(".midi", R.drawable.file_audio);
		mFile2IconTbl.put(".mp3", R.drawable.file_audio);
		mFile2IconTbl.put(".wav", R.drawable.file_audio);
		// video type
		mFile2IconTbl.put(".3gp", R.drawable.file_video);
		mFile2IconTbl.put(".avi", R.drawable.file_video);
		mFile2IconTbl.put(".asf", R.drawable.file_video);
		mFile2IconTbl.put(".mov", R.drawable.file_video);
		mFile2IconTbl.put(".mpg", R.drawable.file_video);
		mFile2IconTbl.put(".mpeg", R.drawable.file_video);
		mFile2IconTbl.put(".mp4", R.drawable.file_video);
	}
	private SimpleAdapter mListAdapter;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		ListView lv = getListView();
		
		Intent intent = this.getIntent();
		if( intent != null )
		{
			mFilePath = intent.getStringExtra(PARAMETER_FILEPATH);
			if( mFilePath == null )
			{
				mFilePath = DEFAULT_FILEPATH;
			}
			mSuffix   = intent.getStringExtra(PARAMETER_SUFFIX);
			if( mSuffix == null )
			{
				mSuffix = DEFAULT_FILEPATH;
			}
		}
		String[] from = {FILE_ICON, FILE_NAME, FILE_ATTR};
		int[] to = {R.id.itemFileIcon, R.id.itemFileName, R.id.itemFileAttribute};
		mFileMapList = new ArrayList<Map<String,Object>>();
		mListAdapter = new SimpleAdapter(this, mFileMapList, R.layout.file_list_item_2, from, to);
		mListAdapter.setViewBinder(this);
		
		openFolder(lv, mFilePath);

	}
	@Override
	protected void onListItemClick(ListView lv, View v, int position, long id) {
		// TODO Auto-generated method stub
		//Uri uri = ContentUris.withAppendedId(getIntent().getData(), id);
//		super.onListItemClick(l, v, position, id);
		ViewGroup vg = (ViewGroup)v;
		TextView  tv = (TextView)vg.findViewById(R.id.itemFileName);
		Intent intent = this.getIntent();
		String action = intent.getAction();
		String filePath = String.valueOf(tv.getText());
		//mListAdapter.getView(position, v, lv);
		Log.d(TAG, filePath);
		if( filePath.equals("..") )
		{
			int end = mFilePath.lastIndexOf('/');
			filePath = mFilePath.substring(0, end);
		}else{
			filePath = mFilePath + "/" + filePath;
		}
		File selectedFile = new File(filePath);
		
		if( selectedFile.isDirectory() )
		{
			openFolder(lv, filePath );
			return;
		}
		if( selectedFile.isFile() /*&& action.equals(Intent.ACTION_PICK)*/ )
		{
			if(false == selectedFile.canRead())
			{
				// ����ļ��в��ɶ�������
				Toast.makeText(this, R.string.warning_file_no_read_authority, Toast.LENGTH_SHORT).show();
				return;
			}

			Log.v(TAG, "select : "+ String.valueOf(position)+ ":" + tv.getText());
			Intent result = new Intent(action, Uri.fromFile(selectedFile));
			setResult(RESULT_OK, result);
			finish();
		}
	}

	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		Log.d(TAG, "onBackPressed !");
		super.onBackPressed();
	}
	
	private void openFolder(ListView lv, String path) {
		if( null == path || null == lv )
		{
			Log.e(TAG, "Paramater is error !");
			return;
		}
		File file = new File(path);
		if( false == file.isDirectory() )
		{
			// ���ǰ·������Ŀ¼����mFilePath��ֵ��mFilePath��Ϊ��ЧĿ¼
			Log.e(TAG, "\"" + path+ "\" is not a directory !");
			path = mFilePath;
		}
		if(false == file.canRead())
		{
			// ����ļ��в��ɶ�������
			Toast.makeText(this, R.string.warning_folder_no_read_authority, Toast.LENGTH_SHORT).show();
			return;
		}

		mFileMapList.clear();

//		ArrayList<Map<String, String>> fileMapList = new ArrayList<Map<String,String>>();
		HashMap<String, Object> imageMap;
		
		File[] filelist = file.listFiles(this);
		if( false == file.getName().toLowerCase(Locale.getDefault()).equals("/mnt") )
		{
			imageMap = new HashMap<String, Object>();
			imageMap.put(FILE_ICON, R.drawable.go_back);
			imageMap.put(FILE_NAME, "..");
			mFileMapList.add(imageMap);
		}
		Log.e(TAG, file.toString() + ":" + filelist.toString());
		for(File f : filelist)
		{
			String fn = f.getName();
			imageMap = new HashMap<String, Object>();
			imageMap.put(FILE_NAME, fn);	
			imageMap.put(FILE_ICON, getFileIcon(f));
			imageMap.put(FILE_ATTR, getFileAttr(f));
			mFileMapList.add(imageMap);
		}
		lv.setAdapter(mListAdapter);
		mFilePath = path;
		this.setTitle(mFilePath);
	}
	@Override
	public boolean setViewValue(View view, Object data, String text) {
		// TODO Auto-generated method stub
		Log.v(TAG, view.toString() + ":" + text);
		if( view instanceof ImageView  )
		{
			((ImageView) view).setImageResource(((Integer)data).intValue());
            return true;
		}
		return false;
	}
	
	@Override
	public boolean accept(File dir, String filename) {
		// �ļ����׺ƥ�����
		String regExp = ".*\\."+ mSuffix.replace(".","\\." ).replace('?','.').toLowerCase(Locale.getDefault())+"$";
		File f = new File(dir.getAbsolutePath() +"/" + filename);
//		Log.e(TAG, "accept( " + dir.getAbsolutePath() +"," + filename + ")");
		if( f.isDirectory() || filename.toLowerCase(Locale.getDefault()).matches(regExp) )
		{
			return true;
		}
		return false;
	}

	private int getFileIcon(File file) {
		
		String filename = file.getName();
		int drawable_id = R.drawable.file_unknown;
		
		if( file.isDirectory())
		{
			return R.drawable.folder_normal;
		}
		String suffix = filename.substring(filename.lastIndexOf('.')).toLowerCase(Locale.getDefault());
		Integer suffix_i = mFile2IconTbl.get(suffix);
		// �ļ����׺ƥ�����
		if( null != suffix_i )
		{
			drawable_id = suffix_i.intValue();
		}
		return drawable_id;
	}
	private String getFileAttr(File file) {
		
		String fileAttr = "";
		final String seperator = " | ";
		
		if( false == file.exists())
		{
			return fileAttr;
		}
		// append file size
		if( file.isFile() )
		{
//			fileAttr = NumberFormat.getInstance().format( file.length());
			long len = file.length();
			if( len < 100 )
			{
				fileAttr = String.valueOf(len) + " B";
			}else if( len < 1000000 )
			{
				fileAttr = String.valueOf(len/1000) 
						+ "." + String.valueOf(len%1000)
						+ " K";				
			}else{
				fileAttr = String.valueOf(len/1000000) 
						+ "." + String.valueOf((len/1000)%1000)
						+ " M";
			}
			fileAttr += seperator;
		}
		// add last modified DateTime
		Date date= new Date(file.lastModified());
//		DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.DEFAULT, DateFormat.DEFAULT);
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		fileAttr += sdf.format(date);
		fileAttr += seperator;
		// add file mod
		if(file.isDirectory())
		{
			fileAttr += "d";
		}
		else if( (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) 
				&& file.canExecute() )
		{
			fileAttr += "x";
		}else{
			fileAttr += "-";
		}
		if( file.canRead() )
		{
			fileAttr += "r";
		}else{
			fileAttr += "-";
		}
		if( file.canWrite() )
		{
			fileAttr += "w";
		}else{
			fileAttr += "-";
		}
		Log.d(TAG, file.getName()+ " : " + fileAttr);
		return fileAttr;
	}
}
