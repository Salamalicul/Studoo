// ============================================================================
//  StudooApkProvider: entrega al instalador de Android el APK descargado.
//  Solo sirve ese archivo y solo para leerlo.
// ============================================================================
package com.studoo.app;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import java.io.File;
import java.io.FileNotFoundException;

public final class StudooApkProvider extends ContentProvider
{
    @Override public boolean onCreate() { return true; }

    private File fileFor (Uri uri) throws FileNotFoundException
    {
        final File file = StudooUpdater.apk;
        if (file == null || ! file.isFile() || ! file.getName().equals (uri.getLastPathSegment()))
            throw new FileNotFoundException (String.valueOf (uri));
        return file;
    }

    @Override public String getType (Uri uri) { return "application/vnd.android.package-archive"; }

    @Override public ParcelFileDescriptor openFile (Uri uri, String mode) throws FileNotFoundException
    {
        return ParcelFileDescriptor.open (fileFor (uri), ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Override public Cursor query (Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
    {
        try
        {
            final File file = fileFor (uri);
            MatrixCursor cursor = new MatrixCursor (new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE });
            cursor.addRow (new Object[] { file.getName(), file.length() });
            return cursor;
        }
        catch (FileNotFoundException e)
        {
            return null;
        }
    }

    @Override public Uri insert (Uri uri, ContentValues values) { return null; }
    @Override public int delete (Uri uri, String selection, String[] selectionArgs) { return 0; }
    @Override public int update (Uri uri, ContentValues values, String selection, String[] selectionArgs) { return 0; }
}
