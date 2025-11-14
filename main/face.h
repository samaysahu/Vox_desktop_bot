
#ifndef FACE_H
#define FACE_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Create display object
extern Adafruit_SSD1306 display;

// Eye position and size variables
int ref_eye_height = 40, ref_eye_width = 40, ref_space_between_eye = 10, ref_corner_radius = 10;
int left_eye_height, right_eye_height, left_eye_width, right_eye_width;
int left_eye_x, left_eye_y, right_eye_x, right_eye_y;

enum EyeState {
  IDLE,
  MOVE_RIGHT_BLINK_DOWN,
  MOVE_RIGHT_BLINK_UP,
  WAIT_1,
  MOVE_LEFT_BLINK_DOWN,
  MOVE_LEFT_BLINK_UP,
  WAIT_2,
  CENTER_BLINK_DOWN,
  CENTER_BLINK_UP,
  WAIT_3
};

EyeState eyeState = IDLE;
unsigned long lastEyeMoveTime = 0;
int eyeMoveStep = 0;
int currentDirection = 1;

inline void draw_eyes() {
  display.clearDisplay();
  int x = int(left_eye_x - left_eye_width / 2), y = int(left_eye_y - left_eye_height / 2);
  display.fillRoundRect(x, y, left_eye_width, left_eye_height, ref_corner_radius, SSD1306_WHITE);
  x = int(right_eye_x - right_eye_width / 2), y = int(right_eye_y - right_eye_height / 2);
  display.fillRoundRect(x, y, right_eye_width, right_eye_height, ref_corner_radius, SSD1306_WHITE);
  display.display();
}

inline void center_eyes() {
  left_eye_height = right_eye_height = ref_eye_height;
  left_eye_width = right_eye_width = ref_eye_width;
  left_eye_x = SCREEN_WIDTH / 2 - ref_eye_width / 2 - ref_space_between_eye / 2;
  left_eye_y = SCREEN_HEIGHT / 2;
  right_eye_x = SCREEN_WIDTH / 2 + ref_eye_width / 2 + ref_space_between_eye / 2;
  right_eye_y = SCREEN_HEIGHT / 2;
}

inline void face_setup() {
  center_eyes();
}

inline void face_loop() {
  unsigned long now = millis();
  int direction_oversize = 1;
  int direction_movement_amplitude = 2;
  int blink_amplitude = 5;

  switch (eyeState) {
    case IDLE:
      eyeState = MOVE_RIGHT_BLINK_DOWN;
      eyeMoveStep = 0;
      lastEyeMoveTime = now;
      currentDirection = 1;
      break;

    case MOVE_RIGHT_BLINK_DOWN:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        left_eye_x += direction_movement_amplitude * currentDirection;
        right_eye_x += direction_movement_amplitude * currentDirection;
        right_eye_height -= blink_amplitude;
        left_eye_height -= blink_amplitude;
        if (currentDirection > 0) {
          right_eye_height += direction_oversize;
          right_eye_width += direction_oversize;
        } else {
          left_eye_height += direction_oversize;
          left_eye_width += direction_oversize;
        }
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = MOVE_RIGHT_BLINK_UP;
          eyeMoveStep = 0;
        }
      }
      break;

    case MOVE_RIGHT_BLINK_UP:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        left_eye_x += direction_movement_amplitude * currentDirection;
        right_eye_x += direction_movement_amplitude * currentDirection;
        right_eye_height += blink_amplitude;
        left_eye_height += blink_amplitude;
        if (currentDirection > 0) {
          right_eye_height += direction_oversize;
          right_eye_width += direction_oversize;
        } else {
          left_eye_height += direction_oversize;
          left_eye_width += direction_oversize;
        }
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = WAIT_1;
          lastEyeMoveTime = now;
        }
      }
      break;

    case WAIT_1:
      if (now - lastEyeMoveTime > 500) {
        eyeState = MOVE_LEFT_BLINK_DOWN;
        eyeMoveStep = 0;
        lastEyeMoveTime = now;
      }
      break;

    case MOVE_LEFT_BLINK_DOWN:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        left_eye_x -= direction_movement_amplitude * currentDirection;
        right_eye_x -= direction_movement_amplitude * currentDirection;
        right_eye_height -= blink_amplitude;
        left_eye_height -= blink_amplitude;
        if (currentDirection > 0) {
          right_eye_height -= direction_oversize;
          right_eye_width -= direction_oversize;
        } else {
          left_eye_height -= direction_oversize;
          left_eye_width -= direction_oversize;
        }
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = MOVE_LEFT_BLINK_UP;
          eyeMoveStep = 0;
        }
      }
      break;

    case MOVE_LEFT_BLINK_UP:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        left_eye_x -= direction_movement_amplitude * currentDirection;
        right_eye_x -= direction_movement_amplitude * currentDirection;
        right_eye_height += blink_amplitude;
        left_eye_height += blink_amplitude;
        if (currentDirection > 0) {
          right_eye_height -= direction_oversize;
          right_eye_width -= direction_oversize;
        } else {
          left_eye_height -= direction_oversize;
          left_eye_width -= direction_oversize;
        }
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = WAIT_2;
          lastEyeMoveTime = now;
        }
      }
      break;

    case WAIT_2:
      if (now - lastEyeMoveTime > 700) {
        if (currentDirection == 1) {
          currentDirection = -1;
          eyeState = MOVE_RIGHT_BLINK_DOWN;
        } else {
          eyeState = CENTER_BLINK_DOWN;
        }
        eyeMoveStep = 0;
        lastEyeMoveTime = now;
      }
      break;

    case CENTER_BLINK_DOWN:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        center_eyes();
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = CENTER_BLINK_UP;
          eyeMoveStep = 0;
        }
      }
      break;

    case CENTER_BLINK_UP:
      if (now - lastEyeMoveTime > 20) {
        lastEyeMoveTime = now;
        center_eyes();
        draw_eyes();
        eyeMoveStep++;
        if (eyeMoveStep >= 3) {
          eyeState = WAIT_3;
          lastEyeMoveTime = now;
        }
      }
      break;

    case WAIT_3:
      if (now - lastEyeMoveTime > 500) {
        eyeState = IDLE;
      }
      break;
  }
}

#endif
