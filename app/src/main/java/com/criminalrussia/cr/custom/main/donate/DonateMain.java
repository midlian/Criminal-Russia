package criminal.russia.custom.main.donate;

import android.app.Activity;
import android.view.animation.AnimationUtils;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.viewpager.widget.ViewPager;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.google.android.material.tabs.TabLayout;
import com.nvidia.devtech.NvEventQueueActivity;

public class DonateMain {

    private final ConstraintLayout mInputLayout;

    private final DonatePagesAdapter mAdapter;

    private final TextView mTitle;

    private int mPreviousPageId = 0;
    private TabLayout.Tab mPreviousTab;

    public DonateMain(Activity activity) {
        this.mInputLayout = activity.findViewById(R.id.brp_donate_main);

        this.mAdapter = new DonatePagesAdapter(NvEventQueueActivity.getInstance().getApplicationContext(), NvEventQueueActivity.getInstance().getSupportFragmentManager());

        this.mTitle = activity.findViewById(R.id.brp_donate_title);

        ViewPager viewPager = activity.findViewById(R.id.brp_donate_vp);
        viewPager.setAdapter(mAdapter);

        TabLayout tabs = activity.findViewById(R.id.brp_donate_tabs);
        tabs.setupWithViewPager(viewPager);

        for (int i = 0; i < tabs.getTabCount(); i++) {
            TabLayout.Tab tab = tabs.getTabAt(i);

            if (tab == null)
                continue;

            int finalI = i;

            tab.view.setOnClickListener(v -> {
                v.setAnimation(AnimationUtils.loadAnimation(NvEventQueueActivity.getInstance(), R.anim.bounce));

                changeTab(finalI, tab);
            });
            tab.setCustomView(mAdapter.getViewById(i));

            if (i == 0)
                changeTab(i, tab);
        }

        Utils.HideLayout(this.mInputLayout, false);
    }

    private void changeTab(int tabId, TabLayout.Tab tab) {
        this.mTitle.setText(this.mAdapter.getTitle(tabId));

        this.mAdapter.getTabView(mPreviousPageId);
        this.mAdapter.getSelectedTabView(tabId);

        tab.setCustomView(mAdapter.getViewById(tabId));

        if (this.mPreviousTab != null)
            this.mPreviousTab.setCustomView(mAdapter.getViewById(mPreviousPageId));

        this.mPreviousPageId = tabId;
        this.mPreviousTab = tab;
    }
}
