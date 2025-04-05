package com.criminalrussia.launcher.adapter;

import android.annotation.SuppressLint;

import android.view.LayoutInflater;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.*;

import androidx.recyclerview.widget.RecyclerView;
import androidx.annotation.NonNull;

import criminal.russia.R;
import com.criminalrussia.launcher.model.Servers;
import java.util.ArrayList;
public class ServersAdapter extends RecyclerView.Adapter<ServersAdapter.ServersViewHolder> {
	Context context;
	
	ArrayList<Servers> serverList;
	
	public ServersAdapter(Context context, ArrayList<Servers> serverList){
		 this.context = context;
		 this.serverList = serverList;
	}
	
	@NonNull
	@Override
    public ServersViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(context).inflate(R.layout.servers_layout_new, parent, false);
		return new ServersViewHolder(v); 
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onBindViewHolder(@NonNull ServersViewHolder holder, int position) {
        Servers servers = serverList.get(position);
        if(servers.getX2()) {
            holder.x2.setVisibility(View.VISIBLE);
            holder.a.setVisibility(View.VISIBLE);
        } else {
            holder.x2.setVisibility(View.INVISIBLE);
            holder.a.setVisibility(View.INVISIBLE);
        }
        holder.serverText.setText(servers.getName());
        holder.onlineText.setText(Integer.toString(servers.getOnline()));
        holder.maxOnlineText.setText("/" + servers.getMaxOnline());
    }

    @Override
    public int getItemCount() {
        return serverList.size();
    }

    public static class ServersViewHolder extends RecyclerView.ViewHolder {

        public TextView onlineText;
        public TextView serverText;
        public TextView maxOnlineText;
        public ImageView x2;
        public ImageView a;
	    
        public ServersViewHolder(View itemView) {
            super(itemView);
            
		    x2 = itemView.findViewById(R.id.brp_launcher_server_double);
            a = itemView.findViewById(R.id.brp_launcher_server_shipping);
            serverText = itemView.findViewById(R.id.brp_launcher_server_title);
            onlineText = itemView.findViewById(R.id.brp_launcher_server_online);
            maxOnlineText = itemView.findViewById(R.id.textView5);
        }
    }
	
}