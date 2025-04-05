package criminal.russia.custom.main.antibot;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import criminal.russia.R;
import criminal.russia.custom.util.Utils;

import java.util.Random;

public class Captcha extends View {

    private Paint mTextPaint;

    private String mCode;

    public Captcha(Context context) {
        super(context);

        init(context);
    }

    public Captcha(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);

        init(context);
    }

    public Captcha(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        init(context);
    }

    public Captcha(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);

        init(context);
    }

    private void init(Context context) {
        this.mTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        this.mTextPaint.setColor(0xffc2c2c2);
        this.mTextPaint.setTextSize(context.getResources().getDimensionPixelSize(R.dimen._20sdp));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        this.mCode = Utils.randomString(6);

        int posX = (int) ((canvas.getWidth() - mTextPaint.getTextSize() * 6) / 2);
        int posY = (int) ((canvas.getHeight() + mTextPaint.getTextSize()) / 2);

        for (int i = 0; i < this.mCode.length(); i++) {
            Random random = new Random();

            canvas.drawText(String.valueOf(this.mCode.charAt(i)), posX, posY, mTextPaint);
            posX += mTextPaint.getTextSize() + random.nextInt(15);
            posY += random.nextInt(15) + random.nextInt(25) * -1;
        }
    }

    public String getCode() {
        return this.mCode;
    }
}
