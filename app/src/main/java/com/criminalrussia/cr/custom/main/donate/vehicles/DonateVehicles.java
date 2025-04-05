package criminal.russia.custom.main.donate.vehicles;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;

import criminal.russia.R;
import criminal.russia.custom.util.CustomRecyclerView;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

public class DonateVehicles extends Fragment {

    private DonateVehiclesAdapter mVehicleAdapter;
    private DonateVehicleTypesAdapter mVehicleTypesAdapter;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.brp_donate_vehicles, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        this.mVehicleAdapter = new DonateVehiclesAdapter();
        this.mVehicleTypesAdapter = new DonateVehicleTypesAdapter();

        CustomRecyclerView recyclerView = view.findViewById(R.id.brp_donate_vehicles_rv);
        recyclerView.setAdapter(this.mVehicleTypesAdapter);

        DividerItemDecoration itemDecorator = new DividerItemDecoration(NvEventQueueActivity.getInstance(), LinearLayoutManager.HORIZONTAL);
        itemDecorator.setDrawable(Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.divider));

        recyclerView.addItemDecoration(itemDecorator);
    }
}
