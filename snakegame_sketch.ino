#include <LedControl.h>

#include <SnakeGame.h>

#define REFRESH_DELAY 300

#define MUX_DELAY 5

#define ES_OFF PORTB &= B11111011
#define ES_ON PORTB |= B00000100
#define ZS_OFF PORTB &= B11111110
#define ZS_ON PORTB |= B00000001
#define HS_OFF PORTB &= B11111101
#define HS_ON PORTB |= B00000010
#define TS_OFF PORTC &= B11111110;
#define TS_ON PORTC |= B00000001;

#define T1_PIN A5
#define T2_PIN A4
#define T3_PIN A3
#define T4_PIN A2

#define DIN_PORT 11
#define CS_PORT 12
#define CLK_PORT 13
#define MAXMATRIX_USED 1

byte numbers[10] = {    //PORTD .... 7 = G 6 = F 5 = E 4 = D 3 = C 2 = B 1 = A 0 = BUZZER
  B01111110,  //0
  B00001100,  //1
  B10110110,  //2
  B10011110,  //3
  B11001100,  //4
  B11011010,  //5
  B11111010,  //6
  B01001110,  //7    
  B11111110,  //8
  B11011110   //9
};

LedControl maxMatrix = LedControl(DIN_PORT, CLK_PORT, CS_PORT, MAXMATRIX_USED);

SnakeGame snakeg;
unsigned int score = 0;
byte lives = 3;
Vector2D movingDirection;

unsigned long int lastRefresh = millis();

void reset(){
  snakeg.reset();
  lastRefresh = millis();
  movingDirection.set(0,0);
  lives = 3;
  score = 0;
  maxMatrix.shutdown(0, false);
  maxMatrix.setIntensity(0, 8);
  maxMatrix.clearDisplay(0);
}

void setup() {
  DDRD = B11111111;
  DDRB |= B00000111;
  DDRC |= B00000001;
  reset();
  delay(1000);
  playIntroOnMatrix(maxMatrix);
}

void loop() {
  movingDirection = getMovingDirection();
  multiplex4DigitNumber(numbers, score + (lives * 1000));

  if(lives <= 0){
    lives = 3;
    score = 0;
  }
  if(millis() - lastRefresh >= REFRESH_DELAY){
    maxMatrix.clearDisplay(0);
    lastRefresh = millis();
    if (snakeg.snake.eatsFood()) {
      score += 10;
      snakeg.snake.tail.addPart();
      snakeg.spawnFood();
    }
    if (snakeg.snake.hitsBorder() || snakeg.snake.hitsItself()) {
      snakeg.reset();
      lives--;
    }
    
    snakeg.setObjectPositions();
    snakeg.snake.move(movingDirection.x, movingDirection.y);
    displayPointsOnMatrix(maxMatrix, snakeg.objectPositions);
  }
}

void displayNumberOnSevenSeg(byte numbers[], byte number){
  PORTD &= B00000001;
  PORTD |= numbers[number];
}

void displayDigit(byte numbers[], byte digit, byte number){
  turnDigitOn(digit);
  displayNumberOnSevenSeg(numbers, number);
}

void turnDigitOn(byte digit){
  HS_OFF;
  ES_OFF;
  ZS_OFF;
  switch(digit){
    case 0:
      ES_ON;
      ZS_OFF;
      HS_OFF;
      TS_OFF;
      break;
    case 1:
      ES_OFF;
      ZS_ON;
      HS_OFF;
      TS_OFF;
      break;
    case 2:
      ES_OFF;
      ZS_OFF;
      HS_ON;
      TS_OFF;
      break;
    case 3:
      ES_OFF;
      ZS_OFF;
      HS_OFF;
      TS_ON;
      break;
    default:
      break;
  }
}

void multiplex4DigitNumber(byte numbers[], int number){
    byte numberOfDigit[4];
    numberOfDigit[3] = (number / 1000);   //MSB
    numberOfDigit[2] = (number / 100) - numberOfDigit[3] * 10;
    numberOfDigit[1] = (number / 10) - numberOfDigit[3] * 100 - numberOfDigit[2] * 10;
    numberOfDigit[0] = number - (numberOfDigit[3] * 1000) - (numberOfDigit[2] * 100) - (numberOfDigit[1] * 10);   //LSB
    for(int i = 0; i < 4; i++){
      displayDigit(numbers, i, numberOfDigit[i]);
      delay(MUX_DELAY);
    }
}

Vector2D getMovingDirection(){
  if(digitalRead(T1_PIN))
    return Vector2D(-1, 0);
  if(digitalRead(T2_PIN))
    return Vector2D(0, -1);
  if(digitalRead(T3_PIN))
    return Vector2D(0, 1);
  if(digitalRead(T4_PIN))
    return Vector2D(1, 0);
}

void displayPointsOnMatrix(LedControl displayMatrix, Vector2D points[]){
  for(byte i = 0; !points[i].equals(Vector2D('N','D')); i++){
    displayMatrix.setLed(0, points[i].x, points[i].y, 1);
  }
}

void playIntroOnMatrix(LedControl displayMatrix){
  for(byte k = 1; k <= 2; k++){
    for(byte i = 0; i < 8; i++){
      for(byte j = 0; j < 8; j++){
          displayMatrix.setLed(0, i, j, k % 2);
          delay(12);
      }
    }
  }
}
