#ifndef DINOGAME_H
#define DINOGAME_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C


class DinoGame {
public:
  // Use the global display instance (defined elsewhere) via reference injection
  DinoGame(Adafruit_SSD1306 &disp) : display(disp) {}

  bool gameRunning = false;

  bool begin() {
    // assume display already initialized by main face_setup
    display.clearDisplay();
    //introMessage();
    delay(2000); // Intro message shown
    resetGame();
    return true;
  }

  void update() {
    if (!gameRunning) return;

    if (gameOverFlag) {
      gameRunning = false;
      return;
    }

    runGameLoop();
  }

  void resetGame() {
    obstacle_x = 127;
    obstacle1_x = 195;
    dino_y = DINO_INIT_Y;
    jump = 0;
    score = 0;
    gameOverFlag = false;
    gameRunning = true;
    display.clearDisplay();
    display.display();
  }

private:
  Adafruit_SSD1306 &display;

  static const int16_t DINO_WIDTH = 25, DINO_HEIGHT = 26;
  static const int16_t DINO_INIT_X = 10, DINO_INIT_Y = 29;
  static const int16_t BASE_LINE_X = 0, BASE_LINE_Y = 54, BASE_LINE_X1 = 127, BASE_LINE_Y1 = 54;
  static const int16_t OBSTACLE_HEIGHT = 8;
  static const int16_t OBSTACLE_Y = BASE_LINE_Y - OBSTACLE_HEIGHT;
  static const int16_t JUMP_PIXEL = 22;

  int16_t dino_y = DINO_INIT_Y;
  int16_t obstacle_x = 127;
  int16_t obstacle1_x = 195;
  int jump = 0;
  int score = 0;
  bool gameOverFlag = false;

  static const unsigned char PROGMEM dino1[];

  // void introMessage() {
  //   display.setTextSize(2);
  //   display.setTextColor(SSD1306_WHITE);
  //   display.setCursor(10, 5);
  //   display.println("Dino Game");

  //   display.setTextSize(1);
  //   display.setCursor(5, 45);
  //   display.println("Starting...");

  //   display.display();
  // }

  void moveDino(int16_t y) {
    display.drawBitmap(DINO_INIT_X, y, dino1, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
  }

  void moveObstacle(int16_t x) {
    display.drawLine(x, OBSTACLE_Y, x, OBSTACLE_Y + OBSTACLE_HEIGHT, SSD1306_WHITE);
    display.drawLine(x + 1, OBSTACLE_Y, x + 1, OBSTACLE_Y + OBSTACLE_HEIGHT, SSD1306_WHITE);
  }

  // void displayScore(int score) {
  //   display.setTextSize(1);
  //   display.setCursor(64, 10);
  //   display.print("Score: ");
  //   display.print(score);
  // }

  void gameOver() {
    // display.clearDisplay();

    // display.setTextSize(2);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(10, 5);
    // display.println("Game Over");

    // display.setTextSize(1);
    // display.setCursor(10, 30);
    // display.print("Score: ");
    // display.print(score);

    // display.setCursor(1, 45);
    // display.println("Restarting...");

    // display.display();
    // delay(3000);
  }

  void runGameLoop() {
    display.clearDisplay();

    // Auto-jump logic
    if (jump == 0) {
      if ((obstacle_x > DINO_INIT_X) && (obstacle_x < (DINO_INIT_X + DINO_WIDTH + 5))) {
        jump = 1;
      } else if ((obstacle1_x > DINO_INIT_X) && (obstacle1_x < (DINO_INIT_X + DINO_WIDTH + 5))) {
        jump = 1;
      }
    }

    // Jump logic
    if (jump == 1) {
      dino_y--;
      if (dino_y == (DINO_INIT_Y - JUMP_PIXEL)) {
        jump = 2;
        score++;
      }
    } else if (jump == 2) {
      dino_y++;
      if (dino_y == DINO_INIT_Y) {
        jump = 0;
      }
    }

    // Collision detection
    if ((obstacle_x <= (DINO_INIT_X + DINO_WIDTH) && obstacle_x >= (DINO_INIT_X + (DINO_WIDTH / 2))) && (dino_y >= (DINO_INIT_Y - 3))) {
      gameOverFlag = true;
    }
    if ((obstacle1_x <= (DINO_INIT_X + DINO_WIDTH) && obstacle1_x >= (DINO_INIT_X + (DINO_WIDTH / 2))) && (dino_y >= (DINO_INIT_Y - 3))) {
      gameOverFlag = true;
    }

    //displayScore(score);
    moveObstacle(obstacle_x);
    moveObstacle(obstacle1_x);
    moveDino(dino_y);
    display.drawLine(BASE_LINE_X, BASE_LINE_Y, BASE_LINE_X1, BASE_LINE_Y1, SSD1306_WHITE);

    obstacle_x--;
    obstacle1_x--;

    if (obstacle_x <= 0) obstacle_x = 127;
    if (obstacle1_x <= 0) obstacle1_x = 195;

    display.display();

    if (gameOverFlag) {
      gameOver();
    }
  }

};

const unsigned char PROGMEM DinoGame::dino1[] = {
  0x00,0x00,0x00,0x00,0x00,0x07,0xfe,0x00,0x00,0x06,0xff,0x00,0x00,0x0e,0xff,0x00,
  0x00,0x0f,0xff,0x00,0x00,0x0f,0xff,0x00,0x00,0x0f,0xff,0x00,0x00,0x0f,0xc0,0x00,
  0x00,0x0f,0xfc,0x00,0x40,0x0f,0xc0,0x00,0x40,0x1f,0x80,0x00,0x40,0x7f,0x80,0x00,
  0x60,0xff,0xe0,0x00,0x71,0xff,0xa0,0x00,0x7f,0xff,0x80,0x00,0x7f,0xff,0x80,0x00,
  0x7f,0xff,0x80,0x00,0x3f,0xff,0x00,0x00,0x1f,0xff,0x00,0x00,0x0f,0xfe,0x00,0x00,
  0x03,0xfc,0x00,0x00,0x01,0xdc,0x00,0x00,0x01,0x8c,0x00,0x00,0x01,0x8c,0x00,0x00,
  0x01,0x0c,0x00,0x00,0x01,0x8e,0x00,0x00
};

#endif
