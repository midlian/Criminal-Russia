package criminal.russia.custom.main.donate.vehicles;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.StringRes;
import androidx.recyclerview.widget.RecyclerView;

import criminal.russia.R;

public class DonateVehicleTypesAdapter extends RecyclerView.Adapter {

    @StringRes
    private static final int[] TAB_TITLES = new int[]{ R.string.vehicle_type_economy, R.string.vehicle_type_economy, R.string.vehicle_type_economy, R.string.vehicle_type_economy };
    private static final int[] TAB_PRICES = new int[]{ 25, 1000, 6300, 15000 };
    private static final int[] TAB_IMAGES = new int[]{ R.drawable.donate_test_img, R.drawable.donate_middle_veh, R.drawable.donate_elite_veh, R.drawable.donate_exclusive_veh };

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new DonateVehicleTypesAdapter.ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.brp_donate_vehicles_type_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        this.onBindViewHolder((DonateVehicleTypesAdapter.ViewHolder) holder, position);
    }

    public void onBindViewHolder(@NonNull final DonateVehicleTypesAdapter.ViewHolder holder, int position) {
        holder.mTitle.setText(TAB_TITLES[position]);
        holder.mPrice.setText(String.valueOf(TAB_PRICES[position]));
        holder.mBackground.setImageResource(TAB_IMAGES[position]);
    }

    @Override
    public int getItemCount() {
        return 4;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView mTitle;

        public TextView mPrice;

        public ImageView mBackground;

        public View mView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);

            this.mView = itemView;

            this.mTitle = itemView.findViewById(R.id.brp_donate_vehicles_type_title);
            this.mPrice = itemView.findViewById(R.id.brp_donate_vehicles_type_price);
            this.mBackground = itemView.findViewById(R.id.brp_donate_vehicles_type_img);
        }
    }
}
