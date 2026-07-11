#include "DisplayEyes.h"

DisplayEyes::DisplayEyes() {
    // Initial conditions
    lx_cur = lx_tar = 64.0f - ref_w/2.0f - ref_space/2.0f;
    ly_cur = ly_tar = 25.0f; // Start at center
    lw_cur = lw_tar = ref_w;
    lh_cur = lh_tar = 2.0f; // Start fully closed for smooth booting

    rx_cur = rx_tar = 64.0f + ref_w/2.0f + ref_space/2.0f;
    ry_cur = ry_tar = 25.0f; // Start at center
    rw_cur = rw_tar = ref_w;
    rh_cur = rh_tar = 2.0f; // Start fully closed
}

void DisplayEyes::setTargetCenter() {
    lx_tar = 64.0f - ref_w/2.0f - ref_space/2.0f;
    ly_tar = 25.0f;
    lw_tar = ref_w;
    lh_tar = ref_h;

    rx_tar = 64.0f + ref_w/2.0f + ref_space/2.0f;
    ry_tar = 25.0f;
    rw_tar = ref_w;
    rh_tar = ref_h;
}

void DisplayEyes::setTargetSleep() {
    setTargetCenter();
    lh_tar = 2.0f;
    rh_tar = 2.0f;
}

void DisplayEyes::setTargetHappy() {
    setTargetCenter();
    show_happy = true;
}

void DisplayEyes::setTargetSad() {
    setTargetCenter();
    show_sad = true;
    lh_tar = 22.0f; // slightly squished
    rh_tar = 22.0f;
}

void DisplayEyes::setTargetSurprise() {
    setTargetCenter();
    lw_tar = 20.0f; // tall and thin
    rw_tar = 20.0f;
    lh_tar = 42.0f;
    rh_tar = 42.0f;
    ly_tar -= 4.0f; // move up slightly
    ry_tar -= 4.0f;
}

void DisplayEyes::setTargetAngry() {
    setTargetCenter();
    show_angry = true;
    lx_tar += 3.0f; // move closer together
    rx_tar -= 3.0f;
}


void DisplayEyes::updateInterpolation() {
    float ease = 0.35f; // 35% towards target per frame
    lx_cur += (lx_tar - lx_cur) * ease;
    ly_cur += (ly_tar - ly_cur) * ease;
    lw_cur += (lw_tar - lw_cur) * ease;
    lh_cur += (lh_tar - lh_cur) * ease;

    rx_cur += (rx_tar - rx_cur) * ease;
    ry_cur += (ry_tar - ry_cur) * ease;
    rw_cur += (rw_tar - rw_cur) * ease;
    rh_cur += (rh_tar - rh_cur) * ease;
}

void DisplayEyes::drawLeftEye(U8G2& display, int x, int y, int w, int h, int radius, bool filled) {
    // The user's coordinates are center-based. U8G2 uses top-left.
    int tlx = x - w/2;
    int tly = y - h/2;
    
    // Prevent rendering glitches by capping radius
    int effective_radius = radius;
    if (effective_radius > w/2) effective_radius = w/2;
    if (effective_radius > h/2) effective_radius = h/2;

    if (filled) {
        display.drawRBox(tlx, tly, w, h, effective_radius);
    } else {
        display.drawRFrame(tlx, tly, w, h, effective_radius);
    }
}

void DisplayEyes::drawRightEye(U8G2& display, int x, int y, int w, int h, int radius, bool filled) {
    drawLeftEye(display, x, y, w, h, radius, filled);
}

void DisplayEyes::setTargetOrganicIdle(int frame, bool isPatrol) {
    // Handle random movements
    if (frame >= next_move_frame) {
        // Pick next move frame (faster if patrolling)
        int min_wait = isPatrol ? 8 : 15;
        int max_wait = isPatrol ? 25 : 50;
        next_move_frame = frame + random(min_wait, max_wait);
        
        // Reset base target to center
        lx_tar = 64.0f - ref_w/2.0f - ref_space/2.0f;
        ly_tar = 25.0f;
        lw_tar = ref_w;
        lh_tar = ref_h;

        rx_tar = 64.0f + ref_w/2.0f + ref_space/2.0f;
        ry_tar = 25.0f;
        rw_tar = ref_w;
        rh_tar = ref_h;

        // 75% chance to look somewhere, 25% chance to just look center
        if (random(0, 100) < 75) {
            float dir_x = random(-10, 11) / 10.0f; // -1.0 to 1.0
            float dir_y = random(-10, 11) / 10.0f;

            // 30% chance to look extreme up/down/left/right
            if (random(0, 100) < 30) {
                if (random(0, 2) == 0) { 
                    dir_x = (dir_x > 0) ? 1.0f : -1.0f; 
                    dir_y = 0.0f; 
                } else { 
                    dir_y = (dir_y > 0) ? 1.0f : -1.0f; 
                    dir_x = 0.0f; 
                }
            }

            float move_amp_x = 12.0f;
            float move_amp_y = 8.0f;

            lx_tar += move_amp_x * dir_x;
            rx_tar += move_amp_x * dir_x;
            ly_tar += move_amp_y * dir_y;
            ry_tar += move_amp_y * dir_y;
        }
    }

    // Handle random blinking
    if (frame >= next_blink_frame) {
        // Quick blink
        lh_tar = 2.0f;
        rh_tar = 2.0f;
        
        // If we've held the blink for 3 frames, open eyes and pick next blink
        if (frame >= next_blink_frame + 3) {
            lh_tar = ref_h;
            rh_tar = ref_h;
            
            // Next blink in 30 to 80 frames
            next_blink_frame = frame + random(30, 80);
            
            // 20% chance of double blink
            if (random(0, 100) < 20) {
                next_blink_frame = frame + 5; // blink again very soon
            }
        }
    }
}

void DisplayEyes::draw(U8G2& display, OperatingState state, int frame) {
    // Reset modifiers
    show_happy = false;
    show_x = false;
    show_shock = false;
    show_sad = false;
    show_angry = false;

    // 1. Determine targets based on state
    switch (state) {
        case OP_BOOTING:
            // Wakeup: Eyes start closed and open up
            setTargetCenter();
            if (frame < 5) {
                lh_tar = rh_tar = 2.0f; // Stay closed for first few frames
            }
            break;
            
        case OP_IDLE_READY:
        case OP_FULLY_CHARGED:
        case OP_MANUAL_CONTROL:
            setTargetOrganicIdle(frame, false);
            break;

        case OP_AUTONOMOUS_PATROL:
            setTargetOrganicIdle(frame, true);
            break;

        case OP_CHARGING:
        case OP_SHUTDOWN:
        case OP_CRITICAL_BATTERY:
            setTargetSleep();
            break;

        case OP_LOW_BATTERY:
            setTargetSad();
            break;

        case OP_UNKNOWN_PERSON:
            setTargetSurprise();
            break;

        case OP_INTRUDER_DETECTED:
            setTargetAngry();
            // Blink angrily
            if (frame % 20 < 2) {
                lh_tar = rh_tar = 2.0f;
            }
            break;

        case OP_SPEAKING:
            setTargetHappy();
            // Blinking while speaking
            if (frame % 15 < 3) {
                lh_tar = 2.0f;
                rh_tar = 2.0f;
            }
            break;

        case OP_CAMERA_RECORDING:
        case OP_OBSTACLE_WARNING:
        case OP_OBSTACLE_CRITICAL:
        case OP_EMERGENCY_STOP:
            setTargetCenter();
            show_shock = true;
            lw_tar = ref_w + 6.0f;
            rw_tar = ref_w + 6.0f;
            lh_tar = ref_h + 6.0f;
            rh_tar = ref_h + 6.0f;
            break;

        case OP_SENSOR_ERROR:
        case OP_MOTOR_ERROR:
            setTargetCenter();
            show_x = true;
            break;
            
        default:
            setTargetOrganicIdle(frame, false);
            break;
    }

    // 2. Interpolate
    updateInterpolation();

    // 3. Draw
    int lx = (int)lx_cur;
    int ly = (int)ly_cur;
    int lw = (int)lw_cur;
    int lh = (int)lh_cur;

    int rx = (int)rx_cur;
    int ry = (int)ry_cur;
    int rw = (int)rw_cur;
    int rh = (int)rh_cur;

    int r = (int)ref_radius;

    if (show_x) {
        // Draw X for error state
        display.drawLine(lx - 12, ly - 12, lx + 12, ly + 12);
        display.drawLine(lx + 12, ly - 12, lx - 12, ly + 12);
        display.drawLine(rx - 12, ry - 12, rx + 12, ry + 12);
        display.drawLine(rx + 12, ry - 12, rx - 12, ry + 12);
    } 
    else if (show_shock) {
        // Empty huge frame with tiny pupils
        drawLeftEye(display, lx, ly, lw, lh, r, false);
        drawRightEye(display, rx, ry, rw, rh, r, false);
        display.drawDisc(lx, ly, 2);
        display.drawDisc(rx, ry, 2);
    }
    else {
        // Standard filled eyes
        drawLeftEye(display, lx, ly, lw, lh, r, true);
        drawRightEye(display, rx, ry, rw, rh, r, true);

        // Overlay masks
        if (show_happy) {
            display.setDrawColor(0); // Draw black
            int offset = lh / 4;
            display.drawTriangle(lx - lw/2 - 1, ly + offset, lx + lw/2 + 1, ly + 5 + offset, lx - lw/2 - 1, ly + lh + offset);
            display.drawTriangle(rx + rw/2 + 1, ry + offset, rx - rw/2 - 1, ry + 5 + offset, rx + rw/2 + 1, ry + rh + offset);
            display.setDrawColor(1);
        }
        else if (show_sad) {
            display.setDrawColor(0);
            display.drawTriangle(lx - lw/2 - 2, ly - lh/2 - 2, lx + lw/2 + 2, ly - 2, lx + lw/2 + 2, ly - lh/2 - 2);
            display.drawTriangle(rx + rw/2 + 2, ry - rh/2 - 2, rx - rw/2 - 2, ry - 2, rx - rw/2 - 2, ry - rh/2 - 2);
            display.setDrawColor(1);
        }
        else if (show_angry) {
            display.setDrawColor(0);
            display.drawTriangle(lx + lw/2 + 2, ly - lh/2 - 2, lx - lw/2 - 2, ly + 2, lx - lw/2 - 2, ly - lh/2 - 2);
            display.drawTriangle(rx - rw/2 - 2, ry - rh/2 - 2, rx + rw/2 + 2, ry + 2, rx + rw/2 + 2, ry - rh/2 - 2);
            display.setDrawColor(1);
        }
    }
}
