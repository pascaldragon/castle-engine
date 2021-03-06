/* -*- tab-width: 4 -*- */
package net.sourceforge.castleengine;

import android.util.Log;

import com.google.android.gms.analytics.Logger;
import com.google.android.gms.analytics.GoogleAnalytics;
import com.google.android.gms.analytics.HitBuilders;
import com.google.android.gms.analytics.Tracker;
import com.google.android.gms.analytics.ecommerce.Product;
import com.google.android.gms.analytics.ecommerce.ProductAction;

/**
 * Google Analytics integration with Castle Game Engine Android application.
 */
public class ComponentGoogleAnalytics extends ComponentAbstract
{
    private static final String TAG = "${NAME}.castleengine.ComponentGoogleAnalytics";

    private Tracker mTracker;

    private String mAnalyticsPropertyId;

    public ComponentGoogleAnalytics(MainActivity activity)
    {
        super(activity);
    }

    public String getName()
    {
        return "google-analytics";
    }

    private synchronized Tracker getAppTracker() {
        if (mTracker == null && mAnalyticsPropertyId != null) {
            GoogleAnalytics analytics = GoogleAnalytics.getInstance(
                getActivity().getApplication());
            // analytics.getLogger().setLogLevel(Logger.LogLevel.VERBOSE);
            mTracker = analytics.newTracker(mAnalyticsPropertyId);
            mTracker.enableAdvertisingIdCollection(true);
            Log.i(TAG, "Created Google Analytics tracker with tracking id " + mAnalyticsPropertyId);
        }
        return mTracker;
    }

    private void initialize(String analyticsPropertyId)
    {
        mAnalyticsPropertyId = analyticsPropertyId;
    }

    /**
     * Send analytics screen view.
     * Following https://developers.google.com/analytics/devguides/collection/analyticsjs/screens
     *
     *   Screens in Google Analytics represent content users are viewing
     *   within an app. The equivalent concept for a website is pages.
     *   Measuring screen views allows you to see which content is being
     *   viewed most by your users, and how are they are navigating between
     *   different pieces of content.
     */
    private void sendScreenView(String screenName)
    {
        Tracker t = getAppTracker();
        if (t == null) {
            return;
        }

        // Set screen name.
        t.setScreenName(screenName);

        // Send a screen view.
        t.send(new HitBuilders.ScreenViewBuilder().build());
    }

    /**
     * Send analytics event.
     * See https://developers.google.com/analytics/solutions/mobile-implementation-guide
     * about the meaning of events and such.
     */
    private void sendEvent(String category, String action, String label,
        long value)
    {
        Tracker t = getAppTracker();
        if (t == null) {
            return;
        }
        t.send(new HitBuilders.EventBuilder()
            .setCategory(category)
            .setAction(action)
            .setLabel(label)
            .setValue(value)
            .build());
    }

/*
    This is the wrong place to do it. It should be called
    by ComponentGoogleBiling when transaction is successfully finished.

    private void sendEventPurchase(String category, String action, String label,
        long value, String productName)
    {
        Tracker t = getAppTracker();
        if (t == null) {
            return;
        }

        Product product = new Product()
            .setName(productName)
            .setPrice(1);
        ProductAction productAction = new ProductAction(ProductAction.ACTION_PURCHASE);
        t.send(new HitBuilders.EventBuilder()
            .setCategory(category)
            .setAction(action)
            .setLabel(label)
            .setValue(value)
            .addProduct(product)
            .setProductAction(productAction)
            .build());
    }
*/

    private void sendTiming(String category, String variable,
        String label, long timeMiliseconds)
    {
        Tracker t = getAppTracker();
        if (t == null) {
            return;
        }
        t.send(new HitBuilders.TimingBuilder()
            .setCategory(category)
            .setVariable(variable)
            .setLabel(label)
            .setValue(timeMiliseconds)
            .build());
    }

    @Override
    public boolean messageReceived(String[] parts)
    {
        if (parts.length == 2 && parts[0].equals("google-analytics-initialize")) {
            initialize(parts[1]);
            return true;
        } else
        if (parts.length == 2 && parts[0].equals("analytics-send-screen-view")) {
            sendScreenView(parts[1]);
            return true;
        } else
        if (parts.length == 5 && parts[0].equals("analytics-send-event")) {
            sendEvent(parts[1], parts[2], parts[3], Long.parseLong(parts[4]));
            return true;
        } else
        // if (parts.length == 6 && parts[0].equals("analytics-send-event-purchase")) {
        //     sendEventPurchase(parts[1], parts[2], parts[3], Long.parseLong(parts[4]), parts[5]);
        //     return true;
        // } else
        if (parts.length == 5 && parts[0].equals("analytics-send-timing")) {
            sendTiming(parts[1], parts[2], parts[3], Long.parseLong(parts[4]));
            return true;
        } else {
            return false;
        }
    }
}
