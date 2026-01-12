// Copyright Setto. All Rights Reserved.

package com.setto.sdk;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import androidx.browser.customtabs.CustomTabsIntent;
import androidx.browser.customtabs.CustomTabColorSchemeParams;

/**
 * Android helper class for Setto SDK
 * Uses Chrome Custom Tabs for secure browser-based payment
 */
public class SettoSDK {

    /**
     * Opens the payment URL using Chrome Custom Tabs
     * Falls back to system browser if Custom Tabs not available
     */
    public static void openBrowser(Activity activity, String url) {
        try {
            // Build Custom Tabs intent with bottom sheet behavior
            CustomTabColorSchemeParams colorScheme = new CustomTabColorSchemeParams.Builder()
                .setToolbarColor(0xFFFFFFFF)
                .build();

            CustomTabsIntent customTabsIntent = new CustomTabsIntent.Builder()
                .setDefaultColorSchemeParams(colorScheme)
                .setShowTitle(true)
                .setShareState(CustomTabsIntent.SHARE_STATE_OFF)
                .setInitialActivityHeightPx(activity.getResources().getDisplayMetrics().heightPixels * 3 / 4)
                .setCloseButtonPosition(CustomTabsIntent.CLOSE_BUTTON_POSITION_END)
                .build();

            customTabsIntent.launchUrl(activity, Uri.parse(url));
        } catch (Exception e) {
            // Fallback to system browser
            try {
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                activity.startActivity(browserIntent);
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
}
