#ifndef JNIUTIL_H
#define JNIUTIL_H

#include <jni.h>
#include <string.h>
#include <malloc.h>

/* http://java.sun.com/docs/books/jni/html/exceptions.html#26050 */
void JNU_ThrowByName(JNIEnv *, const char *, const char *);

/* http://java.sun.com/docs/books/jni/html/other.html#26021 */
jstring JNU_NewStringNative(JNIEnv *, const char *);

/* http://java.sun.com/docs/books/jni/html/other.html#26043 */
char *JNU_GetStringNativeChars(JNIEnv *, jstring);

void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg)
{
	jclass cls = (*env)->FindClass(env, name);
	/* if cls is NULL, an exception has already been thrown */
	if (cls != NULL)
	{
		(*env)->ThrowNew(env, cls, msg);
	}
	/* free the local ref */
	(*env)->DeleteLocalRef(env, cls);
}

jmethodID jmethod_str = 0;

jstring JNU_NewStringNative(JNIEnv *env, const char *str)
{
	jclass jcls_str;
	jstring result;
	jbyteArray bytes;
	int len;
	if (str == NULL)
	{
		return NULL;
	}

	jcls_str = (*env)->FindClass(env, "java/lang/String");
	if (!jmethod_str)
		jmethod_str = (*env)->GetMethodID(env, jcls_str, "<init>", "([B)V");

	bytes = 0;

	if ((*env)->EnsureLocalCapacity(env, 2) < 0)
	{
		return NULL; /* out of memory error */
	}
	len = strlen(str);
	bytes = (*env)->NewByteArray(env, len);
	if (bytes != NULL)
	{
		(*env)->SetByteArrayRegion(env, bytes, 0, len, (jbyte *) str);
		result = (jstring) (*env)->NewObject(env, jcls_str, jmethod_str, bytes);
		(*env)->DeleteLocalRef(env, bytes);
		return result;
	} /* else fall through */
	return NULL;
}

jmethodID MID_String_getBytes = 0;

char *JNU_GetStringNativeChars(JNIEnv *env, jstring jstr)
{
	jbyteArray bytes = 0;
	char *result = 0;
	jthrowable exc;

	if ((*env)->EnsureLocalCapacity(env, 2) < 0)
	{
		return 0; /* out of memory error */
	}

	if (!MID_String_getBytes)
		MID_String_getBytes = (*env)->GetMethodID(env, (*env)->FindClass(env,
				"java/lang/String"), "getBytes", "()[B");

	bytes = (jbyteArray) (*env)->CallObjectMethod(env, jstr, MID_String_getBytes);
	exc = (*env)->ExceptionOccurred(env);

	if (!exc)
	{
		jint len = (*env)->GetArrayLength(env, bytes);
		result = (char *)malloc(len + 1);
		if (result == 0)
		{
			JNU_ThrowByName(env, "java/lang/OutOfMemoryError", 0);
			(*env)->DeleteLocalRef(env, bytes);
			return 0;
		}

		(*env)->GetByteArrayRegion(env, bytes, 0, len, (jbyte *) result);
		result[len] = 0; /* NULL-terminate */
	}
	else
	{
		(*env)->DeleteLocalRef(env, exc);
	}

	(*env)->DeleteLocalRef(env, bytes);

	return result;
}

#endif
