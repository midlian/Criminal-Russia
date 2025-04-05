package criminal.russia.custom.animations.text;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.text.Layout;
import android.util.AttributeSet;
import android.view.animation.AccelerateDecelerateInterpolator;

import com.hanks.htextview.base.CharacterDiffResult;
import com.hanks.htextview.base.CharacterUtils;
import com.hanks.htextview.base.DefaultAnimatorListener;
import com.hanks.htextview.base.HText;
import com.hanks.htextview.base.HTextView;

import java.util.ArrayList;
import java.util.List;

public class EvaporateText extends HText {

    float mCharTime = 300;
    int mMostCount = 20;

    private int mTextHeight;

    private final List<CharacterDiffResult> mDifferentList = new ArrayList<>();
    private long mDuration;

    private ValueAnimator mAnimator;

    @Override
    public void init(final HTextView hTextView, AttributeSet attrs, int defStyle) {
        super.init(hTextView, attrs, defStyle);

        mAnimator = new ValueAnimator();
        mAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        mAnimator.addListener(new DefaultAnimatorListener() {
            @Override
            public void onAnimationEnd(Animator animation) {
                if (animationListener != null) {
                    animationListener.onAnimationEnd(mHTextView);
                }
            }
        });

        mAnimator.addUpdateListener(animation -> {
            progress = (float) animation.getAnimatedValue();
            mHTextView.invalidate();
        });

        mDuration = (long) (mCharTime + mCharTime / mMostCount * (mText.length() <= 0 ? 1 : mText.length() - 1));
    }

    @Override
    public void animateText(final CharSequence text) {
        mHTextView.post(() -> {
            Layout layout = mHTextView.getLayout();
            if (layout != null)
                oldStartX = layout.getLineLeft(0);

            super.animateText(text);
        });
    }

    @Override
    protected void initVariables() {
    }

    @Override
    protected void animateStart(CharSequence text) {
        mDuration = (long) (mCharTime + mCharTime / mMostCount * (mText.length() <= 0 ? 1 : mText.length() - 1));

        mAnimator.cancel();
        mAnimator.setFloatValues(0, 1);
        mAnimator.setDuration(mDuration);
        mAnimator.start();
    }

    @Override
    protected void animatePrepare(CharSequence text) {
        mDifferentList.clear();
        mDifferentList.addAll(CharacterUtils.diff(mOldText, mText));

        Rect bounds = new Rect();
        mPaint.getTextBounds(mText.toString(), 0, mText.length(), bounds);
        mTextHeight = bounds.height();
    }

    @Override
    protected void drawFrame(Canvas canvas) {
        float startX = mHTextView.getLayout().getLineLeft(0);
        float startY = mHTextView.getBaseline();

        float offset = startX;
        float oldOffset = oldStartX;

        int maxLength = Math.max(mText.length(), mOldText.length());

        for (int i = 0; i < maxLength; i++) {
            if (i < mOldText.length()) {
                float pp = progress * mDuration / (mCharTime + mCharTime / mMostCount * (mText.length() - 1));

                mOldPaint.setTextSize(mTextSize);

                int move = CharacterUtils.needMove(i, mDifferentList);
                if (move != -1) {
                    mOldPaint.setAlpha(255);

                    float p = pp * 2f;
                    float distX = CharacterUtils.getOffset(i, move, p > 1 ? 1 : p, startX, oldStartX, gapList, oldGapList);

                    canvas.drawText(mOldText.charAt(i) + "", 0, 1, distX, startY, mOldPaint);
                } else {
                    mOldPaint.setAlpha((int) ((1 - pp) * 255));

                    float y = startY - pp * mTextHeight;
                    float width = mOldPaint.measureText(mOldText.charAt(i) + "");

                    canvas.drawText(mOldText.charAt(i) + "", 0, 1, oldOffset + (oldGapList.get(i) - width) / 2, y, mOldPaint);
                }

                oldOffset += oldGapList.get(i);
            }

            if (i < mText.length()) {

                if (!CharacterUtils.stayHere(i, mDifferentList)) {

                    int alpha = (int) (255f / mCharTime * (progress * mDuration - mCharTime * i / mMostCount));
                    alpha = Math.min(alpha, 255);
                    alpha = Math.max(alpha, 0);

                    mPaint.setAlpha(alpha);
                    mPaint.setTextSize(mTextSize);

                    float pp = progress * mDuration / (mCharTime + mCharTime / mMostCount * (mText.length() - 1));
                    float y = mTextHeight + startY - pp * mTextHeight;

                    float width = mPaint.measureText(mText.charAt(i) + "");
                    canvas.drawText(mText.charAt(i) + "", 0, 1, offset + (gapList.get(i) - width) / 2, y, mPaint);
                }

                offset += gapList.get(i);
            }
        }
    }

}
