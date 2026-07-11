#ifndef DISPLAY_EYES_H
#define DISPLAY_EYES_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "../../../include/RobotStates.h"

class DisplayEyes {
public:
    DisplayEyes();
    void draw(U8G2& display, OperatingState state, int frame);

private:
    // Current animated values
    float lx_cur, ly_cur, lw_cur, lh_cur;
    float rx_cur, ry_cur, rw_cur, rh_cur;

    // Target values
    float lx_tar, ly_tar, lw_tar, lh_tar;
    float rx_tar, ry_tar, rw_tar, rh_tar;

    // Default reference dimensions (scaled slightly from user's code to fit text at bottom)
    const float ref_w = 30.0f;
    const float ref_h = 30.0f;
    const float ref_space = 10.0f;
    const float ref_radius = 8.0f;

    // Animation modifiers
    bool show_happy;
    bool show_x;
    bool show_shock;
    bool show_sad;
    bool show_angry;

    // Organic movement tracking
    int next_move_frame;
    int next_blink_frame;

    void updateInterpolation();
    
    // Target setters
    void setTargetCenter();
    void setTargetSleep();
    void setTargetHappy();
    void setTargetSad();
    void setTargetSurprise();
    void setTargetAngry();
    void setTargetOrganicIdle(int frame, bool isPatrol);

    void drawLeftEye(U8G2& display, int x, int y, int w, int h, int radius, bool filled = true);
    void drawRightEye(U8G2& display, int x, int y, int w, int h, int radius, bool filled = true);
};

#endif // DISPLAY_EYES_H
