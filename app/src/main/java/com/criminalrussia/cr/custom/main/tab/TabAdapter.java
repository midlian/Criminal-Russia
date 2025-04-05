package criminal.russia.custom.main.tab;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.TextView;

import criminal.russia.R;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

public class TabAdapter extends RecyclerView.Adapter implements Filterable {

    private List<PlayerData> mPlayerData;
    private List<PlayerData> mPlayerDataCopy;

    public TabAdapter(List<PlayerData> playerData) {
        this.mPlayerData = playerData;
        this.mPlayerDataCopy = playerData;
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.b_tab_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        this.onBindViewHolder((ViewHolder) holder, position);
    }

    public void onBindViewHolder(@NonNull final ViewHolder holder, int position) {
        PlayerData data = mPlayerData.get(position);

        holder.id.setText(String.valueOf(data.getId()));
        holder.name.setText(data.getName());
        holder.level.setText(String.valueOf(data.getLevel()));
        holder.ping.setText(String.valueOf(data.getPing()));
    }

    @Override
    public int getItemCount() {
        return this.mPlayerData.size();
    }

    @Override
    public Filter getFilter() {
        return new Filter() {
            public Filter.FilterResults performFiltering(CharSequence charSequence) {
                String input = charSequence.toString();
                List<PlayerData> newPlayersDataList = new ArrayList<>();

                if (input.isEmpty())
                    newPlayersDataList = mPlayerDataCopy;
                else {
                    for (PlayerData playersData : mPlayerDataCopy) {
                        if (playersData.getName().toLowerCase().contains(input.toLowerCase()))
                            newPlayersDataList.add(playersData);
                    }
                }

                Filter.FilterResults filterResults = new Filter.FilterResults();
                filterResults.values = newPlayersDataList;
                return filterResults;
            }

            public void publishResults(CharSequence charSequence, Filter.FilterResults filterResults) {
                mPlayerData = (List) filterResults.values;
                notifyDataSetChanged();
            }
        };
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        public TextView id;
        public TextView name;
        public TextView level;
        public TextView ping;

        private final View mView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);

            this.mView = itemView;

            this.id = itemView.findViewById(R.id.player_id);
            this.name = itemView.findViewById(R.id.player_name);
            this.level = itemView.findViewById(R.id.player_level);
            this.ping = itemView.findViewById(R.id.player_ping);
        }

        public View getView() {
            return mView;
        }
    }
}
