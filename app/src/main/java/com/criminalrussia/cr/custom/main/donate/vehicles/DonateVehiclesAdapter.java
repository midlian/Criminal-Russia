package criminal.russia.custom.main.donate.vehicles;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import criminal.russia.R;

public class DonateVehiclesAdapter extends RecyclerView.Adapter {
    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new DonateVehiclesAdapter.ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.brp_donate_product_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        this.onBindViewHolder((DonateVehiclesAdapter.ViewHolder) holder, position);
    }

    public void onBindViewHolder(@NonNull final DonateVehiclesAdapter.ViewHolder holder, int position) {

    }

    @Override
    public int getItemCount() {
        return 10;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView mTitle;

        public TextView mNew;
        public TextView mEvent;
        public TextView mDiscount;

        public LinearLayout mBuyBtn;

        public TextView mPrice;
        public TextView mOldPrice;

        public ImageView mBackground;

        public View mView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);

            this.mView = itemView;

            this.mTitle = itemView.findViewById(R.id.brp_donate_product_title);

            this.mNew = itemView.findViewById(R.id.brp_donate_product_new);
            this.mEvent = itemView.findViewById(R.id.brp_donate_product_event);
            this.mDiscount = itemView.findViewById(R.id.brp_donate_product_discount);

            this.mBuyBtn = itemView.findViewById(R.id.brp_donate_product_buy_btn);

            this.mPrice = itemView.findViewById(R.id.brp_donate_product_price);
            this.mOldPrice = itemView.findViewById(R.id.brp_donate_product_old_price);

            this.mBackground = itemView.findViewById(R.id.brp_donate_product_img);
        }
    }
}
