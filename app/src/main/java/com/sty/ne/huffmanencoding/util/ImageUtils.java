package com.sty.ne.huffmanencoding.util;

import android.media.ExifInterface;

import java.io.IOException;

/**
 * @Author: tian
 * @UpdateDate: 2020/12/29 10:20 PM
 */
public class ImageUtils {
    public static int getImageOrientation(String imageLocalPath) {
        try {
            ExifInterface exifInterface = new ExifInterface(imageLocalPath);
            int orientation = exifInterface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
            return orientation;
        } catch (IOException e) {
            e.printStackTrace();
            return ExifInterface.ORIENTATION_NORMAL;
        }
    }

    public static boolean isRotated(String imageLocalPath) {
        int orientation = getImageOrientation(imageLocalPath);
        switch (orientation) {
            case ExifInterface.ORIENTATION_ROTATE_90:
            case ExifInterface.ORIENTATION_ROTATE_270:
                return true;
            default:
                return false;
        }
    }
}
