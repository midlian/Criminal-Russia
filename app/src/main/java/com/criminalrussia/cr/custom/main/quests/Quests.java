package criminal.russia.custom.main.quests;

import android.app.Activity;
import android.util.Log;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.LinearLayoutManager;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;

import criminal.russia.R;
import criminal.russia.custom.util.CustomRecyclerView;
import criminal.russia.custom.util.Utils;

import com.nvidia.devtech.NvEventQueueActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Objects;

public class Quests {

    private final ConstraintLayout mMainLayout;

    private final CustomRecyclerView mCustomRecyclerView;

    RequestQueue mRequestQueue;

    private final ArrayList<JSONObject> mQuestsList;
    private final ArrayList<Integer> mQuestsStatsList;

    public Quests(Activity activity) {
        this.mMainLayout = activity.findViewById(R.id.brp_quests_main);

        this.mCustomRecyclerView = activity.findViewById(R.id.brp_quests_items);

        TextView exitBtn = activity.findViewById(R.id.brp_quests_close_btn);
        exitBtn.setOnClickListener(view -> Utils.HideLayout(this.mMainLayout, true));

        mRequestQueue = Volley.newRequestQueue(activity);

        this.mQuestsList = new ArrayList<>();
        this.mQuestsStatsList = new ArrayList<>();

        for (int i = 0; i < 100; i++)
            this.mQuestsStatsList.add(0);

        Utils.HideLayout(this.mMainLayout, false);
    }

    public void show() {
        this.mQuestsList.clear();

        final JsonObjectRequest request = new JsonObjectRequest(Request.Method.GET, "http://api.b-rp.ru:3000/api/quests", null, response -> {
            try {
                JSONArray jsonArray = response.getJSONArray("quests");

                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonObject = jsonArray.getJSONObject(i);

                    this.mQuestsList.add(jsonObject);
                }

                QuestsAdapter adapter = new QuestsAdapter(this.mQuestsList, this.mQuestsStatsList);

                this.mCustomRecyclerView.setLayoutManager(new LinearLayoutManager(NvEventQueueActivity.getInstance()));
                this.mCustomRecyclerView.setAdapter(adapter);

                Utils.ShowLayout(mMainLayout, true);
            } catch (JSONException e) {
                Log.e("Quests 1", Objects.requireNonNull(e.getMessage()));
            }
        }, error -> Log.e("Quests 2", Objects.requireNonNull(error.getMessage())));

        mRequestQueue.add(request);
    }

    public void setQuestInfo(int id, int progress) {
        this.mQuestsStatsList.set(id, progress);
    }

    public ConstraintLayout getMainLayout() {
        return mMainLayout;
    }
}
