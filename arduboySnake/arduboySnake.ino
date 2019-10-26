 /**********************************
 SNAKE
 Version 1.1
 Copyright (C) 2019 James STevick
 All rights reserved.
 **********************************/

#include <Arduboy2.h>

// High Scores saved in EEPROM
#define EE_FILE 2

 /**********************************
Initiate Variables
***********************************/
Arduboy2 arduboy;
BeepPin1 beep;

const unsigned int FRAME_RATE = 15;  // Frames per second
int dx = 1;       // Initial movement of snake
byte snakex[500]; // Snakehead starting position
byte snakey[500]; // Snakehead starting position
int dirSnake = 1; // 0-Up, 1-Right, 2-Down, 3-Left
int xa;           // Apple starting position
int ya;           // Apple starting position
boolean released = false;       // Is game in motion
boolean paused = false;         // Is game paused
boolean crashed = false;
boolean snakeGrow = false;       // Did snake just eat an apple
unsigned int score = 0;         // Game score
unsigned int snakeLength = 10;  // Length of snake
boolean menu = true;            // Is menu displayed
boolean initialDraw = false;    // Is initial state of game drawn
char initials[3];               // Initials used in high score
char text_buffer[16];           //General string buffer
boolean pad, pad2, pad3;        //Button press buffer used to stop pause repeating
boolean oldpad, oldpad2, oldpad3;

//Ball Bounds used in collision detection
// byte leftBorder;
// byte rightBorder;
// byte topBorder;
// byte bottomBorder;

void setup()
{
  arduboy.begin(); 
  beep.begin();
  arduboy.setFrameRate(FRAME_RATE); // Sets the frame rate
  arduboy.initRandomSeed();  // Initiates random generator
}

void loop()
{
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return;

  // Handle the timing and stopping of tones
  beep.timer();

  //Title screen loop switches from title screen
  //and high scores until FIRE is pressed
  while (menu)
  {
    menu = titleScreen();
    if (menu)
    {
      menu = displayHighScores(EE_FILE);
    }
  }

  //Initialize game
  if (!initialDraw)
  {
    score = 0;
    snakeLength = 10; 
    dirSnake = 1;
    xa = 2+random(1,41)*3;
    ya = 3+random(1,19)*3;
    
    newGame();
    
    crashed = false;
    initialDraw=true;
  }

  // If the snake has not crashed
  if (!crashed)
  {
    //Pause game if FIRE pressed
    pad = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if(pad == true && oldpad == false && released)
    {
      oldpad2 = false; // Use pad 2 in paused
      pause();
    }
    oldpad = pad;

    if(released){
      drawApple();
    }
    drawSnake();
  }
  else
  {
    drawGameOver();
    if (score > 0)
    {
      enterHighScore(EE_FILE);
    }

    arduboy.clear();
    released = false;
    initialDraw = false;
    menu = true;
  }

  arduboy.display();
}

void moveApple()
{
  if (snakeGrow){
    boolean appleMoved = false;
    while(!appleMoved){
      appleMoved = true;
      xa = 2+random(1,41)*3;
      ya = 3+random(1,19)*3;
      for (byte snakeBodyX = 0; snakeBodyX < snakeLength; snakeBodyX++){
        if (abs(xa - snakex[snakeBodyX]) <=2 ){
          for (byte snakeBodyY = 0; snakeBodyY < snakeLength; snakeBodyY++){
            if (abs(ya - snakey[snakeBodyY]) <=2 ){
              appleMoved = false;
              break;
              break;
            }
          }
        }
      }
    }
    snakeGrow = false;
  }
}

void newGame(){
  arduboy.clear();  // Clear display
  drawSnake();   // Draw snake
  arduboy.drawRect(0, 0, 128, 64, 1); // Draw border
  arduboy.drawRect(0, 1, 128, 62, 1);
  arduboy.display(); // Diplay
}

void moveSnake()
{
  // Initial Snake position
  if (!initialDraw){
    snakex[0] = 47;
    snakey[0] = 30;
    snakex[1] = 44;
    snakey[1] = 30;
    snakex[2] = 41;
    snakey[2] = 30;
    snakex[3] = 38;
    snakey[3] = 30;
    snakex[4] = 35;
    snakey[4] = 30;
    snakex[5] = 32;
    snakey[5] = 30;
    snakex[6] = 29;
    snakey[6] = 30;
    snakex[7] = 26;
    snakey[7] = 30;
    snakex[8] = 23;
    snakey[8] = 30;
    snakex[9] = 20;
    snakey[9] = 30;
  }

  if(released)
  {
    // Did Snake crash into the wall
    if (snakex[0] <= 1 || snakex[0] >= 126 || snakey[0] <= 2 || snakey[0] >= 61)
    {
      crashed = true;
      playToneTimed(175, 500);
      return;
    }

    // Did snake crash into itself
   for (byte snakeBodyX = 0; snakeBodyX < snakeLength; snakeBodyX++){
     for (byte snakeBodyX2 = 0; snakeBodyX2 < snakeLength; snakeBodyX2++){
        if (snakeBodyX != snakeBodyX2){
          if (snakex[snakeBodyX]==snakex[snakeBodyX2]){
            if (snakey[snakeBodyX]==snakey[snakeBodyX2]){
              crashed = true;
              playToneTimed(175, 500);
              return;
            }
          }
        }
      }
    }
      
    // Did Snake eat an apple
    if (abs(xa - snakex[0])<=2 && abs(ya - snakey[0])<=2)
    {
      snakeGrow = true;
      score ++ ;
      playTone(523, 250);
    }

    // Extend snake
    if (snakeGrow){
      snakex[snakeLength] = snakex[snakeLength - 1];
      snakey[snakeLength] = snakey[snakeLength - 1];
    }

    // Move the snake body
    for (byte snakeBody = snakeLength - 1; snakeBody > 0 ; snakeBody--){
      snakex[snakeBody] = snakex[snakeBody - 1];
      snakey[snakeBody] = snakey[snakeBody - 1];
    }

    // Move the snake head
    boolean moveAhead = true;
    if (arduboy.pressed(UP_BUTTON)){
      if (dirSnake != 2){
        snakey[0] -= 3;
        dirSnake = 0;
        moveAhead = false;
      }
    } else if (arduboy.pressed(RIGHT_BUTTON)){
      if (dirSnake != 3){
        snakex[0] += 3;
        dirSnake = 1;
        moveAhead = false;
      }
    } else if (arduboy.pressed(DOWN_BUTTON)){
      if (dirSnake != 0){
        snakey[0] += 3;
        dirSnake = 2;
        moveAhead = false;
      }
    } else if (arduboy.pressed(LEFT_BUTTON)){
      if (dirSnake != 1){
        snakex[0] -= 3;
        dirSnake = 3;
        moveAhead = false;
      }
    } 
    if (moveAhead){
      if (dirSnake==0){
        snakey[0] -= 3;
      } else if (dirSnake==1){
        snakex[0] += 3;
      } else if (dirSnake==2){
        snakey[0] += 3;
      } else if (dirSnake==3){
        snakex[0] -= 3;
      }
    }

    if(snakeGrow){
        snakeLength ++ ; 
    }

  }
  else
  {
    //Release snake if any button pressed
    pad3 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) || arduboy.pressed(UP_BUTTON) || arduboy.pressed(RIGHT_BUTTON) || arduboy.pressed(DOWN_BUTTON) || arduboy.pressed(LEFT_BUTTON);
    if (pad3 == true && oldpad3 == false)
    {
      released = true;
    }
    oldpad3 = pad3;
  }
}

void drawApple()
{
  arduboy.fillRect(xa-1, ya-1, 3, 3, 0);
  moveApple();
  arduboy.fillRect(xa-1, ya-1, 3, 3, 1);
}

void drawSnake()
{
  for (byte snakeBody = 0; snakeBody < snakeLength; snakeBody++){
    arduboy.fillRect(snakex[snakeBody]-1, snakey[snakeBody]-1, 3, 3, 0);
  }
  moveSnake();
  for (byte snakeBody = 0; snakeBody < snakeLength; snakeBody++){
    arduboy.fillRect(snakex[snakeBody]-1, snakey[snakeBody]-1, 3, 3, 1);
  }
}

void drawGameOver()
{
  arduboy.fillRect(xa-1, ya-1, 3, 3, 0);
  for (byte snakeBody = 0; snakeBody < snakeLength; snakeBody++){
    arduboy.fillRect(snakex[snakeBody]-1, snakey[snakeBody]-1, 3, 3, 0);
  }
  arduboy.drawRect(0, 0, 128, 64, 1);
  arduboy.drawRect(0, 1, 128, 62, 1);
  arduboy.setCursor(37, 22);
  arduboy.print("Game Over");
  arduboy.setCursor(31, 36);
  arduboy.print("Score: ");
  arduboy.print(score);
  arduboy.display();
  arduboy.delayShort(3000);
}

void pause()
{
  paused = true;
  //Draw pause to the screen
  arduboy.setCursor(52, 45);
  arduboy.print("PAUSE");
  arduboy.display();
  while (paused)
  {
    arduboy.delayShort(150);
    //Unpause if FIRE is pressed
    pad2 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if (pad2 == true && oldpad2 == false && released)
    {
        arduboy.fillRect(52, 45, 30, 11, 0);

        paused=false;
    }
    oldpad2 = pad2;
  }
}

//Used to delay images while reading button input
boolean pollFireButton(int n)
{
  for(int i = 0; i < n; i++)
  {
    arduboy.delayShort(15);
    pad = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if(pad == true && oldpad == false)
    {
      oldpad3 = true; //Forces pad loop 3 to run once
      return true;
    }
    oldpad = pad;
  }
  return false;
}

//Function by nootropic design to display highscores
boolean displayHighScores(byte file)
{
  byte y = 8;
  byte x = 24;
  // Each block of EEPROM has 7 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file * 7 * 5 + EEPROM_STORAGE_SPACE_START;
  byte hi, lo;
  arduboy.clear();
  arduboy.setCursor(32, 0);
  arduboy.print("HIGH SCORES");
  arduboy.display();

  for(int i = 0; i < 7; i++)
  {
    sprintf(text_buffer, "%2d", i+1);
    arduboy.setCursor(x,y+(i*8));
    arduboy.print(text_buffer);
    arduboy.display();
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);

    if ((hi == 0xFF) && (lo == 0xFF))
    {
      score = 0;
    }
    else
    {
      score = (hi << 8) | lo;
    }

    initials[0] = (char)EEPROM.read(address + (5*i) + 2);
    initials[1] = (char)EEPROM.read(address + (5*i) + 3);
    initials[2] = (char)EEPROM.read(address + (5*i) + 4);

    if (score > 0)
    {
      sprintf(text_buffer, "%c%c%c %u", initials[0], initials[1], initials[2], score);
      arduboy.setCursor(x + 24, y + (i*8));
      arduboy.print(text_buffer);
      arduboy.display();
    }
  }
  if (pollFireButton(300))
  {
    return false;
  }
  return true;
  arduboy.display();
}

boolean titleScreen()
{
  //Clears the screen
  arduboy.clear();
  arduboy.setCursor(34,12);
  arduboy.setTextSize(2);
  arduboy.print("SNAKE");
  arduboy.setTextSize(1);
  arduboy.display();
  if (pollFireButton(25))
  {
    return false;
  }
  arduboy.setCursor(16, 32);
  arduboy.print("BY JAMES STEVICK");
  arduboy.display();

  //Flash "Press FIRE" 5 times
  for(byte i = 0; i < 5; i++)
  {
    //Draws "Press FIRE"
    arduboy.setCursor(1, 55);
    arduboy.print("PRESS A OR B TO START");
    arduboy.display();

    if (pollFireButton(75))
    {
      return false;
    }
  }

  return true;
}

//Function by nootropic design to add high scores
void enterInitials()
{
  byte index = 0;

  arduboy.clear();

  initials[0] = ' ';
  initials[1] = ' ';
  initials[2] = ' ';

  while (true)
  {
    arduboy.display();
    arduboy.clear();

    arduboy.setCursor(16,0);
    arduboy.print("HIGH SCORE");
    sprintf(text_buffer, "%u", score);
    arduboy.setCursor(88, 0);
    arduboy.print(text_buffer);
    arduboy.setCursor(56, 20);
    arduboy.print(initials[0]);
    arduboy.setCursor(64, 20);
    arduboy.print(initials[1]);
    arduboy.setCursor(72, 20);
    arduboy.print(initials[2]);
    for(byte i = 0; i < 3; i++)
    {
      arduboy.drawLine(56 + (i*8), 27, 56 + (i*8) + 6, 27, 1);
    }
    arduboy.drawLine(56, 28, 88, 28, 0);
    arduboy.drawLine(56 + (index*8), 28, 56 + (index*8) + 6, 28, 1);
    arduboy.delayShort(70);

    if (arduboy.pressed(LEFT_BUTTON) || arduboy.pressed(B_BUTTON))
    {
      if (index > 0)
      {
        index--;
        playToneTimed(1046, 80);
      }
    }

    if (arduboy.pressed(RIGHT_BUTTON))
    {
      if (index < 2)
      {
        index++;
        playToneTimed(1046, 80);
      }
    }

    if (arduboy.pressed(UP_BUTTON))
    {
      initials[index]++;
      playToneTimed(523, 80);
      // A-Z 0-9 :-? !-/ ' '
      if (initials[index] == '0')
      {
        initials[index] = ' ';
      }
      if (initials[index] == '!')
      {
        initials[index] = 'A';
      }
      if (initials[index] == '[')
      {
        initials[index] = '0';
      }
      if (initials[index] == '@')
      {
        initials[index] = '!';
      }
    }

    if (arduboy.pressed(DOWN_BUTTON))
    {
      initials[index]--;
      playToneTimed(523, 80);
      if (initials[index] == ' ') {
        initials[index] = '?';
      }
      if (initials[index] == '/') {
        initials[index] = 'Z';
      }
      if (initials[index] == 31) {
        initials[index] = '/';
      }
      if (initials[index] == '@') {
        initials[index] = ' ';
      }
    }

    if (arduboy.pressed(A_BUTTON))
    {
      playToneTimed(1046, 80);
      if (index < 2)
      {
        index++;
      } else {
        return;
      }
    }
  }

}

void enterHighScore(byte file)
{
  // Each block of EEPROM has 7 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file * 7 * 5 + EEPROM_STORAGE_SPACE_START;
  byte hi, lo;
  char tmpInitials[3];
  unsigned int tmpScore = 0;

  // High score processing
  for(byte i = 0; i < 7; i++)
  {
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);
    if ((hi == 0xFF) && (lo == 0xFF))
    {
      // The values are uninitialized, so treat this entry
      // as a score of 0.
      tmpScore = 0;
    } else
    {
      tmpScore = (hi << 8) | lo;
    }
    if (score > tmpScore)
    {
      enterInitials();
      for(byte j = i; j < 7; j++)
      {
        hi = EEPROM.read(address + (5*j));
        lo = EEPROM.read(address + (5*j) + 1);

        if ((hi == 0xFF) && (lo == 0xFF))
        {
        tmpScore = 0;
        }
        else
        {
          tmpScore = (hi << 8) | lo;
        }

        tmpInitials[0] = (char)EEPROM.read(address + (5*j) + 2);
        tmpInitials[1] = (char)EEPROM.read(address + (5*j) + 3);
        tmpInitials[2] = (char)EEPROM.read(address + (5*j) + 4);

        // write score and initials to current slot
        EEPROM.update(address + (5*j), ((score >> 8) & 0xFF));
        EEPROM.update(address + (5*j) + 1, (score & 0xFF));
        EEPROM.update(address + (5*j) + 2, initials[0]);
        EEPROM.update(address + (5*j) + 3, initials[1]);
        EEPROM.update(address + (5*j) + 4, initials[2]);

        // tmpScore and tmpInitials now hold what we want to
        //write in the next slot.
        score = tmpScore;
        initials[0] = tmpInitials[0];
        initials[1] = tmpInitials[1];
        initials[2] = tmpInitials[2];
      }

      score = 0;
      initials[0] = ' ';
      initials[1] = ' ';
      initials[2] = ' ';

      return;
    }
  }
}

// Play a tone at the specified frequency for the specified duration.
void playTone(unsigned int frequency, unsigned int duration)
{
  beep.tone(beep.freq(frequency), duration / (1000 / FRAME_RATE));
}

// Play a tone at the specified frequency for the specified duration using
// a delay to time the tone.
// Used when beep.timer() isn't being called.
void playToneTimed(unsigned int frequency, unsigned int duration)
{
  beep.tone(beep.freq(frequency));
  arduboy.delayShort(duration);
  beep.noTone();
}
