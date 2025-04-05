package criminal.russia.custom.main.donate;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;

import criminal.russia.R;
import criminal.russia.custom.main.donate.vehicles.DonateVehicles;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;

public class DonatePagesAdapter extends FragmentPagerAdapter {

    @StringRes
    private static final int[] TAB_TITLES = new int[]{ R.string.donate_stock, R.string.donate_services, R.string.donate_transport, R.string.donate_skins, R.string.donate_boxex, R.string.donate_vip, R.string.donate_cloths };
    private static final int[] TAB_IMAGES = new int[]{ R.drawable.ic_donate_percent, R.drawable.ic_donate_star, R.drawable.ic_donate_wheel, R.drawable.ic_donate_skin, R.drawable.ic_donate_box, R.drawable.ic_donate_vip, R.drawable.ic_donate_cloths };

    private final ArrayList<Fragment> mFragments;
    private final ArrayList<View> mViews;

    private final Context mContext;

    public DonatePagesAdapter(Context context, FragmentManager fm) {
        super(fm);
        this.mContext = context;

        this.mFragments = new ArrayList<>();
        this.mViews = new ArrayList<>();

        for (int i = 0; i < this.getCount(); i++)
            this.mFragments.add(new DonateVehicles());

        for (int i = 0; i < this.getCount(); i++)
            this.mViews.add(createView(i));
    }

    @NotNull
    @Override
    public Fragment getItem(int position) {
        return this.mFragments.get(position);
    }

    @Nullable
    @Override
    public CharSequence getPageTitle(int position) {
        return mContext.getResources().getString(TAB_TITLES[position]);
    }

    public View createView(int position) {
        View view = LayoutInflater.from(mContext).inflate(R.layout.brp_donate_page_item, null);

        TextView tv = view.findViewById(R.id.brp_donate_page_item_title);
        tv.setText(TAB_TITLES[position]);

        ImageView iv = view.findViewById(R.id.brp_donate_page_item_img);
        iv.setImageDrawable(Utils.getRes(NvEventQueueActivity.getInstance(), TAB_IMAGES[position]));

        return view;
    }

    public void getTabView(int position) {
        TextView tv = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_title);
        tv.setText(TAB_TITLES[position]);

        ImageView iv = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_img);
        iv.setImageDrawable(Utils.getRes(NvEventQueueActivity.getInstance(), TAB_IMAGES[position]));

        ConstraintLayout cl = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_main);
        cl.setBackground(Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.donate_page_item_bg));
    }

    public void getSelectedTabView(int position) {
        TextView tv = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_title);
        tv.setText(TAB_TITLES[position]);

        ImageView iv = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_img);
        iv.setImageDrawable(Utils.changeDrawableColor(TAB_IMAGES[position], Color.WHITE));

        ConstraintLayout cl = this.mViews.get(position).findViewById(R.id.brp_donate_page_item_main);
        cl.setBackground(Utils.changeDrawableColor(R.drawable.donate_page_item_bg, Color.parseColor("#337BE7"))); // R.drawable.donate_page_item_bg
    }

    public View getViewById(int position) {
        return this.mViews.get(position);
    }

    public int getTitle(int position) {
        return TAB_TITLES[position];
    }

    @Override
    public int getCount() {
        return 7;
    }
}
