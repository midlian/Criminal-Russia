package criminal.russia.custom.cef;

import android.annotation.SuppressLint;
import android.view.MotionEvent;
import android.view.View;

import com.nvidia.devtech.NvEventQueueActivity;

public class CefTouchListener implements View.OnTouchListener {

    private boolean touchState = true;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        if(touchState) {
            int x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
            int numEvents = event.getPointerCount();

            for (int i = 0; i < numEvents; i++) {
                int pointerId = event.getPointerId(i);

                if (pointerId == 0)
                {
                    x1 = (int)event.getX(i);
                    y1 = (int)event.getY(i);
                }
                else if (pointerId == 1)
                {
                    x2 = (int)event.getX(i);
                    y2 = (int)event.getY(i);
                }
                else if (pointerId == 2)
                {
                    x3 = (int)event.getX(i);
                    y3 = (int)event.getY(i);
                }
            }

            int pointerId = event.getPointerId(event.getActionIndex());
            int action = event.getActionMasked();

            NvEventQueueActivity.getInstance().customMultiTouchEvent(action, pointerId, x1, y1, x2, y2, x3, y3);
        }

        return false;
    }
}
