package criminal.russia.custom.main.quests;

import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Formatter;
import java.util.Objects;

public class QuestsAdapter extends RecyclerView.Adapter {

    private final ArrayList<JSONObject> mQuestsList;
    private final ArrayList<Integer> mQuestsStatsList;

    private final Drawable mTargetDrawable;
    private final Drawable mCompleteDrawable;

    private final Drawable mCompleteBtnBgDrawable;
    private final Drawable mStartBtnBgDrawable;

    public QuestsAdapter(ArrayList<JSONObject> quests, ArrayList<Integer> questsStats) {
        this.mQuestsList = quests;
        this.mQuestsStatsList = questsStats;

        this.mTargetDrawable = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.ic_target);
        this.mCompleteDrawable = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.ic_complete);

        this.mCompleteBtnBgDrawable = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.quest_item_complete_btn_bg);
        this.mStartBtnBgDrawable = Utils.getRes(NvEventQueueActivity.getInstance(), R.drawable.quest_item_start_btn_bg);
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.brp_quest_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        this.onBindViewHolder((ViewHolder) holder, position);
    }

    public void onBindViewHolder(@NonNull final ViewHolder holder, int position) {
        JSONObject quest = this.mQuestsList.get(position);

        try {
            int id = quest.getInt("id");

            int currentProgress = this.mQuestsStatsList.get(position);
            int maxProgress = quest.getInt("max_progress");

            boolean isCompleted = currentProgress >= maxProgress;

            String title = quest.getString("name");
            String description = quest.getString("description");
            String task = quest.getString("task");
            String reward = new Formatter().format("Награда\n%dEXP %d₽", quest.getInt("reward_exp"), quest.getInt("reward_money")).toString();

            holder.mIcon.setImageDrawable(isCompleted ? this.mCompleteDrawable : this.mTargetDrawable);

            holder.mTitle.setText(title);
            holder.mProgressTV.setText(new Formatter().format("%d/%d", currentProgress, maxProgress).toString());
            holder.mReward.setText(reward);

            if (isCompleted) {
                holder.mStartBtn.setBackground(this.mCompleteBtnBgDrawable);
                holder.mStartBtn.setText("Выполнен");

                holder.mStartBtn.setOnClickListener(view -> {});
            } else {
                holder.mStartBtn.setBackground(this.mStartBtnBgDrawable);
                holder.mStartBtn.setText("Начать");
                //ПЕРЕПИСАТЬ!
                holder.mStartBtn.setOnClickListener(view ->
                        NvEventQueueActivity.getInstance().mQuestsInfo.show(id, title, description, task, reward));
            }

            holder.mProgress.setMax(maxProgress);
            holder.mProgress.setProgress(currentProgress);
        } catch (JSONException e) {
            Log.e("Quests", Objects.requireNonNull(e.getMessage()));
        }
    }

    @Override
    public int getItemCount() {
        return this.mQuestsList.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        public ImageView mIcon;

        public TextView mTitle;
        public TextView mProgressTV;
        public TextView mReward;

        public TextView mStartBtn;

        public ProgressBar mProgress;

        private final View mView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);

            this.mView = itemView;

            this.mIcon = itemView.findViewById(R.id.brp_quests_icon);

            this.mTitle = itemView.findViewById(R.id.brp_quests_quest_item_name);
            this.mProgressTV = itemView.findViewById(R.id.brp_quests_quest_item_progress_tv);
            this.mReward = itemView.findViewById(R.id.brp_quests_quest_item_reward);

            this.mStartBtn = itemView.findViewById(R.id.brp_quests_quest_item_start);

            this.mProgress = itemView.findViewById(R.id.brp_quests_quest_item_progress_pb);
        }

        public View getView() {
            return mView;
        }
    }
}
