// BreakoutGame.h
#ifndef BREAKOUTGAME_H
#define BREAKOUTGAME_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1

#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 3
#define BALL_SIZE 3
#define BRICK_ROWS 4
#define BRICK_COLS 8
#define BRICK_WIDTH (SCREEN_WIDTH / BRICK_COLS - 2)
#define BRICK_HEIGHT 6
#define PADDLE_Y (SCREEN_HEIGHT - PADDLE_HEIGHT - 2)

class BreakoutGame {
public:
  Adafruit_SSD1306 &display;
  int paddleX;
  int ballX, ballY;
  float ballSpeedX, ballSpeedY;
  bool bricks[BRICK_ROWS][BRICK_COLS];
  int score;
  bool gameRunning;

  BreakoutGame(Adafruit_SSD1306 &disp) : display(disp) {}

  void begin() {
    // assume display already initialized by face_setup()
    display.clearDisplay();
    display.display();
    delay(2000);
    initGame();
  }

  void update() {
    if (gameRunning) {
      updateGame();
      drawGame();
      delay(20); // adjust game speed
    } else {
      gameOver();
      delay(3000);
      initGame();
    }
  }

private:
  void initGame() {
    paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    ballX = SCREEN_WIDTH / 2;
    ballY = SCREEN_HEIGHT / 2;
    ballSpeedX = 1;
    ballSpeedY = -1;

    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        bricks[row][col] = true;
      }
    }
    score = 0;
    gameRunning = true;
  }

  void updateGame() {
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE) {
      ballSpeedX = -ballSpeedX;
    }
    if (ballY <= 0) {
      ballSpeedY = -ballSpeedY;
    }

    if (ballY + BALL_SIZE >= PADDLE_Y && 
        ballY + BALL_SIZE <= PADDLE_Y + PADDLE_HEIGHT &&
        ballX + BALL_SIZE >= paddleX &&
        ballX <= paddleX + PADDLE_WIDTH) {
      int relativeIntersect = (paddleX + PADDLE_WIDTH / 2) - ballX;
      float normalized = relativeIntersect / (PADDLE_WIDTH / 2.0);
      ballSpeedX = -normalized * 2;
      ballSpeedY = -abs(ballSpeedY);
      ballY = PADDLE_Y - BALL_SIZE;
    }

    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        if (bricks[row][col]) {
          int brickX = col * (BRICK_WIDTH + 2) + 1;
          int brickY = row * (BRICK_HEIGHT + 2) + 1;
          if (ballX + BALL_SIZE >= brickX && ballX <= brickX + BRICK_WIDTH &&
              ballY + BALL_SIZE >= brickY && ballY <= brickY + BRICK_HEIGHT) {
            bricks[row][col] = false;
            score += 10;
            int ballCenterX = ballX + BALL_SIZE / 2;
            int ballCenterY = ballY + BALL_SIZE / 2;
            int brickCenterX = brickX + BRICK_WIDTH / 2;
            int brickCenterY = brickY + BRICK_HEIGHT / 2;
            int dx = ballCenterX - brickCenterX;
            int dy = ballCenterY - brickCenterY;
            if (abs(dx) * BRICK_HEIGHT > abs(dy) * BRICK_WIDTH) {
              ballSpeedX = -ballSpeedX;
            } else {
              ballSpeedY = -ballSpeedY;
            }
          }
        }
      }
    }

    int targetX = ballX - PADDLE_WIDTH / 2;
    if (ballSpeedY > 0) {
      if (paddleX + PADDLE_WIDTH / 2 < targetX) {
        paddleX += 2;
      } else if (paddleX + PADDLE_WIDTH / 2 > targetX) {
        paddleX -= 2;
      }
    } else {
      if (paddleX + PADDLE_WIDTH / 2 < SCREEN_WIDTH / 2) {
        paddleX += 1;
      } else if (paddleX + PADDLE_WIDTH / 2 > SCREEN_WIDTH / 2) {
        paddleX -= 1;
      }
    }
    if (paddleX < 0) paddleX = 0;
    if (paddleX > SCREEN_WIDTH - PADDLE_WIDTH) paddleX = SCREEN_WIDTH - PADDLE_WIDTH;

    if (ballY >= SCREEN_HEIGHT) {
      gameRunning = false;
    }

    bool allBricksDestroyed = true;
    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        if (bricks[row][col]) {
          allBricksDestroyed = false;
          break;
        }
      }
      if (!allBricksDestroyed) break;
    }

    if (allBricksDestroyed) {
      gameRunning = false;
    }
  }

  void drawGame() {
    display.clearDisplay();
    display.fillRect(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);
    display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, SSD1306_WHITE);

    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        if (bricks[row][col]) {
          int brickX = col * (BRICK_WIDTH + 2) + 1;
          int brickY = row * (BRICK_HEIGHT + 2) + 1;
          display.fillRect(brickX, brickY, BRICK_WIDTH, BRICK_HEIGHT, SSD1306_WHITE);
        }
      }
    }

    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(0, 0);
    // display.print("Score: ");
    // display.print(score);

    display.display();
  }

  void gameOver() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 20);

    bool allBricksDestroyed = true;
    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        if (bricks[row][col]) {
          allBricksDestroyed = false;
          break;
        }
      }
      if (!allBricksDestroyed) break;
    }

    if (allBricksDestroyed) {
  //    display.print("YOU WIN!");
    } else {
  //    display.print("GAME OVER");
    }
  //  display.setCursor(20, 40);
  //  display.print("Score: ");
   // display.print(score);

    display.display();
  }
};

#endif // BREAKOUTGAME_H
