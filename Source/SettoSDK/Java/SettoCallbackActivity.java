// Copyright Setto. All Rights Reserved.

package com.setto.sdk;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

/**
 * Activity to handle URL scheme callbacks from Setto payment
 * Receives: setto-{merchantId}://callback?status=success&payment_id=xxx&tx_hash=xxx
 */
public class SettoCallbackActivity extends Activity {

    // Native callback - implemented in C++
    public static native void nativeOnPaymentCallback(String url);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        handleIntent(getIntent());
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        handleIntent(intent);
    }

    private void handleIntent(Intent intent) {
        if (intent != null && Intent.ACTION_VIEW.equals(intent.getAction())) {
            Uri uri = intent.getData();
            if (uri != null) {
                String url = uri.toString();
                // Call native callback
                try {
                    nativeOnPaymentCallback(url);
                } catch (UnsatisfiedLinkError e) {
                    e.printStackTrace();
                }
            }
        }

        // Close this activity and return to game
        finish();
    }
}
