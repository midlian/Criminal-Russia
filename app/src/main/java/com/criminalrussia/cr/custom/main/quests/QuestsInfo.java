package criminal.russia.custom.main.quests;

import android.app.Activity;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;
import com.nvidia.devtech.NvEventQueueActivity;


public class QuestsInfo {

    private final ConstraintLayout mMainLayout;

    private final ImageView mImage;

    private final TextView mTitle;
    private final TextView mDescription;
    private final TextView mTask;
    private final TextView mReward;

    private int mCurrentQuestId = -1;

    public QuestsInfo(Activity activity) {
        this.mMainLayout = activity.findViewById(R.id.brp_quests_info_main);

        TextView exitBtn = activity.findViewById(R.id.brp_quests_info_close_btn);
        exitBtn.setOnClickListener(view -> {

            //ПЕРЕПИСАТЬ!
            NvEventQueueActivity.getInstance().mQuests.show();

            Utils.HideLayout(this.mMainLayout, true);
        });

        this.mImage = activity.findViewById(R.id.brp_quests_info_img);

        this.mTitle = activity.findViewById(R.id.brp_quests_info_title);
        this.mDescription = activity.findViewById(R.id.brp_quests_info_description);
        this.mTask = activity.findViewById(R.id.brp_quests_info_task);
        this.mReward = activity.findViewById(R.id.brp_quests_info_reward);

        TextView startBtn = activity.findViewById(R.id.brp_quests_info_start);
        startBtn.setOnClickListener(view -> {
            Utils.HideLayout(this.mMainLayout, true);

            NvEventQueueActivity.getInstance().applyQuest(this.mCurrentQuestId);
        });

        Utils.HideLayout(this.mMainLayout, false);
    }

    public void show(int id, String title, String description, String task, String reward) {
        this.mImage.setImageResource(R.drawable.dialog_bg);

        this.mCurrentQuestId = id;

        this.mTitle.setText(title);
        this.mDescription.setText(description);
        this.mTask.setText(task);
        this.mReward.setText(reward);

        new Utils.LoadImage(this.mImage).execute("https://brilliantrp-samp-cdn.itcloud.io/new_launcher2/quests/" + id + ".png");

        Utils.ShowLayout(this.mMainLayout, true);
        //ПЕРЕПИСАТЬ!
        Utils.HideLayout(NvEventQueueActivity.getInstance().mQuests.getMainLayout(), true);
    }

}
