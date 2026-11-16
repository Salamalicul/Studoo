// ============================================================================
//  StudooUpdater: abre el instalador de Android con la versión nueva de Studoo
//  que se acaba de descargar (lo llama Updater.cpp).
// ============================================================================
package com.studoo.app;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import java.io.File;

public final class StudooUpdater
{
    // El APK que se puede entregar al instalador (lo lee StudooApkProvider)
    static volatile File apk = null;

    // Devuelve 0 si se abrió el instalador, 1 si antes hay que dar permiso para
    // instalar aplicaciones (se abre la pantalla de ajustes) y -1 si hubo un error.
    public static int installApk (Context context, String path)
    {
        try
        {
            final File file = new File (path);
            if (! file.isFile())
                return -1;

            if (Build.VERSION.SDK_INT >= 26 && ! context.getPackageManager().canRequestPackageInstalls())
            {
                Intent settings = new Intent (Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,
                                              Uri.parse ("package:" + context.getPackageName()));
                settings.addFlags (Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity (settings);
                return 1;
            }

            apk = file;
            Uri uri = Uri.parse ("content://" + context.getPackageName() + ".actualizacion/" + Uri.encode (file.getName()));
            Intent intent = new Intent (Intent.ACTION_VIEW);
            intent.setDataAndType (uri, "application/vnd.android.package-archive");
            intent.addFlags (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity (intent);
            return 0;
        }
        catch (Exception e)
        {
            return -1;
        }
    }
}
