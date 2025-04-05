package criminal.russia.custom.main.dialogs;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.RecyclerView;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;

import java.util.ArrayList;

public class DialogAdapter extends RecyclerView.Adapter {

    private OnClickListener mOnClickListener;
    private OnDoubleClickListener mOnDoubleClickListener;

    private final ArrayList<String> mFieldTexts;
    private final ArrayList<TextView> mFieldHeaders;
    private final ArrayList<ArrayList<TextView>> mFields;

    private int mCurrentSelectedPosition = 0;
    private View mCurrentSelectedView;

    public interface OnClickListener {
        void onClick(int i, String str);
    }

    public interface OnDoubleClickListener {
        void onDoubleClick();
    }

    public DialogAdapter(ArrayList<String> fields, ArrayList<TextView> fieldHeaders) {
        this.mFieldTexts = fields;
        this.mFieldHeaders = fieldHeaders;

        this.mFields = new ArrayList<>();
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.brp_dialog_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        this.onBindViewHolder((ViewHolder) holder, position);
    }

    public void onBindViewHolder(@NonNull final ViewHolder holder, int position) {
        String[] headers = this.mFieldTexts.get(position).split("\t");

        ArrayList<TextView> fields = new ArrayList<>();

        for (int i = 0; i < headers.length; i++) {
            TextView field = holder.mFields.get(i);

            field.setText(Utils.transfromColors(headers[i].replace("\\t", "")));
            field.setVisibility(View.VISIBLE);

            fields.add(field);
        }

        this.mFields.add(fields);

        if (this.mCurrentSelectedPosition == position) {
            this.mCurrentSelectedView = holder.mFieldBg;
            this.mCurrentSelectedView.setVisibility(View.VISIBLE);

            this.mOnClickListener.onClick(position, holder.mFields.get(0).getText().toString());
        } else holder.mFieldBg.setVisibility(View.GONE);

        holder.getView().setOnClickListener(view -> {
            if (this.mCurrentSelectedPosition != position) {
                if (this.mCurrentSelectedView != null)
                    this.mCurrentSelectedView.setVisibility(View.GONE);

                this.mCurrentSelectedPosition = position;
                this.mCurrentSelectedView = holder.mFieldBg;

                holder.mFieldBg.setVisibility(View.VISIBLE);

                this.mOnClickListener.onClick(position, holder.mFields.get(0).getText().toString());
            } else if (this.mOnDoubleClickListener != null)
                this.mOnDoubleClickListener.onDoubleClick();
        });
    }

    public void updateSizes() {
        int[] max = new int[4];

        for (int i = 0; i < this.mFields.size(); i++) {
            for (int j = 0; j < this.mFields.get(i).size(); j++) {
                int width = this.mFields.get(i).get(j).getWidth();

                if (max[j] < width)
                    max[j] = width;
            }
        }

        for (int i = 0; i < max.length; i++) {
            int headerWidth = this.mFieldHeaders.get(i).getWidth();

            Log.i("DIALOG", max[i] + "\t" + this.mFieldHeaders.get(i).getText() + " " + headerWidth);

            if (max[i] < headerWidth)
                max[i] = headerWidth;
        }

        for (int i = 0; i < this.mFields.size(); i++) {
            for (int j = 0; j < this.mFields.get(i).size(); j++)
                this.mFields.get(i).get(j).setWidth(max[j]);
        }

        for (int i = 0; i < this.mFieldHeaders.size(); i++)
            this.mFieldHeaders.get(i).setWidth(max[i]);
    }

    public void setOnClickListener(OnClickListener onClickListener) {
        this.mOnClickListener = onClickListener;
    }

    public void setOnDoubleClickListener(OnDoubleClickListener onDoubleClickListener) {
        this.mOnDoubleClickListener = onDoubleClickListener;
    }

    public ArrayList<ArrayList<TextView>> getFields() {
        return mFields;
    }

    @Override
    public int getItemCount() {
        return this.mFieldTexts.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        public ArrayList<TextView> mFields;
        public ImageView mFieldBg;

        private final View mView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);

            this.mView = itemView;

            this.mFieldBg = itemView.findViewById(R.id.brp_dialog_item_bg);

            ConstraintLayout field = itemView.findViewById(R.id.brp_dialog_item_main);
            this.mFields = new ArrayList<>();

            for (int i = 1; i < field.getChildCount(); i++)
                mFields.add((TextView) field.getChildAt(i));
        }

        public View getView() {
            return mView;
        }
    }
}
