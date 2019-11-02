/**********************************
Frogger
Version 1.0
Copyright (C) 2019 James Stevick
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

// Relevant to all games
int FRAME_RATE = 10;               // Frames per second
boolean pad, pad2, pad3;             //Button press buffer used to stop pause repeating
boolean oldpad, oldpad2, oldpad3;
char initials[3];                    // Initials used in high score
char text_buffer[16];           //General string buffer
boolean menu = true;            // Is menu displayed
boolean initialDraw = false;    // Is initial state of game drawn
boolean drawRound = false;       // Is round and score displayed

// Frogger Y-axis movement
byte fyPos = 0;
byte fyposArr[7] = {60, 49, 40, 31, 22, 13, 4};
byte fx = 124;
byte fy = fyposArr[fyPos];

// Frogger Logs
int log1a[5] = {0, 35, 70, 105, 140}; // 23 / 35
int log2a[4] = {-50, 0, 50, 100};  // 23 / 50
int log3a[3] = {-70, 0, 70}; // 50 / 70
int log4a[4] = {-5, 30, 65, 100}; // 15 /  35
int log5a[4] = {-55, 0, 55, 110}; // 35 / 55
int logLengths[5] =  {23,23,50,15,35}; // lengths of logs for each row

// Frog homes
bool frogHomes[5] = {false, false, false, false, false};
byte frogHomePos[5] = {9, 36, 63, 90, 118};

// Gameplay helper variables
int score = 0;                  // Game score
byte froggerRound = 1;          // Round, determines speed
byte lives = 5;                 // Frogger lives
byte frogCount = 0;             // Frogs in homes
boolean released = false;       // Is game in motion
boolean paused = false;         // Is game paused
boolean froggerFell = false;    // Did Frogger fall
boolean delayFrogJump = false;  // Avoid double jumps
boolean froggerHome = false;    // Did frogger just land in a home

// Setup Game
void setup()
{
  arduboy.begin(); 
  beep.begin();
  arduboy.setFrameRate(FRAME_RATE); // Sets the frame rate
  arduboy.initRandomSeed();  // Initiates random generator
  Serial.begin(9600);
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

  // Initialize game
  if (!initialDraw)
  {
    if (!drawRound)
    {
      delay(50);
      roundScreen();
      drawRound = true;

    }
    newGame();
    initialDraw = true;
  }

  // Has Frogger fallen
  if (!froggerFell){

    // Did frogger land in home
    if(froggerHome)
    {
      if (frogCount == 5)
      {
        playToneTimed(523, 300);
        playToneTimed(1046, 300);
        drawRound = false;
        FRAME_RATE += 2;
        frogCount = 0;
        froggerRound +=1;
        for (byte index = 0; index < 5; index++)
        {
          frogHomes[index] = false;
        }
      }
      else
      {
        playToneTimed(523, 100);
        playToneTimed(1046, 100);       
      }
      froggerHome = false;
      froggerFell = false;
      initialDraw = false;
      return;
    }

    //Pause game if FIRE pressed
    pad = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if(pad == true && oldpad == false && released)
    {
      oldpad2 = false; // Use pad 2 in paused
      pause();
    }
    oldpad = pad;

    drawFrogger(); // Draw Frogger
  }
  // Frog Fell
  else{

    lives --;
    playToneTimed(175, 300);

    // Game over
    if (lives == 0)
    {

      playToneTimed(100, 300);
      drawGameOver();

      // Set game over variables
      menu = true;
      lives = 5;
      FRAME_RATE = 10;
      frogCount = 0;
      score = 0;
      froggerRound = 1;
      for (byte frogs = 0; frogs < 5; frogs ++)
      {
        frogHomes[frogs] = false;
      }

      // High score mode
      if (score > 0)
      {
        enterHighScore(EE_FILE);
      }

    } 

    // Whether Frogger fell or game over
    delay(200); // Show Frogger in failed move spot 
    released = false;
    initialDraw = false;
    froggerFell = false;
  }

  arduboy.display();
}

// Screen shows round and score
void roundScreen(){
  arduboy.clear(); // Clear Screen
  arduboy.drawRect(0, 0, 128, 64, 1); // Draw border
  arduboy.drawRect(0, 1, 128, 62, 1);
  arduboy.setCursor(22, 18);
  arduboy.setTextSize(2);
  arduboy.print("ROUND ");
  arduboy.print(froggerRound);
  arduboy.setCursor(34, 40);
  arduboy.setTextSize(1);
  arduboy.print("Score: ");
  arduboy.print(score);
  arduboy.display();
  delay(1800);
}

// Start a new game of Frogger
void newGame(){
  fyPos = 0; // Reset Frogger location
  fx = 124;
  fy = fyposArr[fyPos];
  arduboy.setFrameRate(FRAME_RATE);  // Sets the frame rate
  arduboy.clear();   // Clear display
  drawHomeFrogs();   // Draw frogs in their homes
  drawBackground();  // Draw background of Frogger
  arduboy.display(); // Display  
  // FOR DEBUGGING
  Serial.print("lives: ");
  Serial.println(lives); 
  Serial.print("score: ");
  Serial.println(score);
  Serial.print("frame rate: ");
  Serial.println(FRAME_RATE);
  Serial.print("frogs in home: ");
  Serial.println(frogCount);
  Serial.print("round: ");
  Serial.print(froggerRound);
  Serial.println(" ");
}

// Draws the background of Froggeer
void drawBackground(){
  arduboy.fillRect(0,0,4,8,1);
  arduboy.fillRect(15,0,16,8,1);
  arduboy.fillRect(42,0,16,8,1);
  arduboy.fillRect(69,0,16,8,1);
  arduboy.fillRect(96,0,17,8,1);
  arduboy.fillRect(124,0,4,8,1);
  arduboy.fillRect(0,55,128,1,1);
}

// Draw Frogs in their homes
void drawHomeFrogs(){
  for (byte index = 0; index < 5; index++)
  {
    if(frogHomes[index])
    {
      arduboy.fillRect(frogHomePos[index]-3, fyposArr[6], 7, 2, 1);
      arduboy.fillRect(frogHomePos[index]-1, fyposArr[6]-2, 3, 4, 1);
      arduboy.fillRect(frogHomePos[index]-2, fyposArr[6]+2, 2, 1, 1);
      arduboy.fillRect(frogHomePos[index]+1, fyposArr[6]+2, 2, 1, 1);
      arduboy.fillRect(frogHomePos[index]-2, fyposArr[6]-3, 2, 1, 1);
      arduboy.fillRect(frogHomePos[index]+1, fyposArr[6]-3, 2, 1, 1);
      arduboy.drawPixel(frogHomePos[index]-1, fyposArr[6]+3, 1);
      arduboy.drawPixel(frogHomePos[index]+1, fyposArr[6]+3, 1);
    }
  }
}

// Check if Frogger made final Jump
boolean froggerJump(){
  for (byte ix = 0; ix < 5; ix++)
  {
    if ((fx >= frogHomePos[ix]-5) && (fx <= frogHomePos[ix]+5))
    {
      fx = frogHomePos[ix];
      frogHomes[ix] = true;
      score += 10*froggerRound;
      frogCount ++;
      froggerFell = false;
      froggerHome = true;
      return true;
    }
  }
  return false;
}


void moveFrogger()
{
  if(released)
  {

    // Set Frogger status booleans
    froggerFell = true;
    bool hitWall = false;

    // Move Frogger based on control buttons
    if (!delayFrogJump){
      if (arduboy.pressed(UP_BUTTON)){
        // Check if Frogger reached goal safely
        fyPos += 1;
        fy = fyposArr[fyPos];
        delayFrogJump = true;
        if (fyPos >= 6) {
          bool success = froggerJump();
          if(success){return;}
        } 
        else
        {
          playToneTimed(523, 50);
        }
      } else if (arduboy.pressed(RIGHT_BUTTON)){
        fx += 5;
        if (fx > 124){
          fx = 124;
        }
      } else if (arduboy.pressed(DOWN_BUTTON)){
        if (fyPos > 0){
          fyPos -= 1;
          delayFrogJump = true;
        }
        playToneTimed(523, 50);
        fy = fyposArr[fyPos];
      } else if (arduboy.pressed(LEFT_BUTTON)){
        fx -= 5;
        if (fx < 3){
          fx = 3;
        }
      } 
    } else {
      delayFrogJump = false;
    }

    // Where did Frogger Jump
    // If on a log move Frogger with log
    // If not on a log and not on the land, Frogger fell in water
    // If hit a wall, check again if fell in water after moved by wall
    for (int i = 0; i<2; i++){

      froggerFell = true;

      for (byte index = 0; index < 6 ; index ++)
      {
        if (fy == fyposArr[index])
        {
          // Frog is on initial land
          if (index == 0){
            froggerFell = false; 
            break;
          } 
          // Frog is on first row of logs
          else if (index == 1) {
            for (byte index2 = 0 ; index2 < 5; index2++){
              if( ((int)fx >= log1a[index2]) && ((int)fx <= (log1a[index2]+logLengths[index-1]) )) {
                if (!hitWall){
                  fx -= 2;
                }
                froggerFell = false;
                break;
              }
            }
          }
          // Frog is on second row of logs
          else if (index == 2) {
            for (byte index2 = 0 ; index2 < 4; index2++){
              if( (int)fx >= log2a[index2] && (int)fx <= log2a[index2]+logLengths[index-1] ) {
                if (!hitWall){
                  fx += 1;
                }
                froggerFell = false;
                break;
              }
            } 
          }
          // Frog is on third row of logs
          else if (index == 3) {
            for (byte index2 = 0 ; index2 < 3; index2++){
              if( (int)fx >= log3a[index2] && (int)fx <= log3a[index2]+logLengths[index-1] ) {
                if (!hitWall){
                  fx += 3;
                }
                froggerFell = false;
                break;
              }
            }            
          }
          // Frog is on fourth row of logs
          else if (index == 4) {
            for (byte index2 = 0 ; index2 < 4; index2++){
              if( (int)fx >= log4a[index2] && (int)fx <= log4a[index2]+logLengths[index-1] ) {
                if (!hitWall){
                  fx -= 2;
                }
                froggerFell = false;
                break;
              }
            }             
          }
          // Frog is on fifth row of logs
          else if (index == 5) {
            for (byte index2 = 0 ; index2 < 4; index2++){
              if( (int)fx >= log5a[index2] && (int)fx <= log5a[index2]+logLengths[index-1] ) {
                if (!hitWall){
                  fx += 2;
                }
                froggerFell = false;
                break;
              }
            }            
          }
          break; // Break loop that checks which row Frogger is on
        }
      }

      // Did frogger hit a wall
      // If so, check again if he fell in the water
      // This allows wall to drag Frogger into water
      if (fx < 3){
        fx = 3;
        hitWall = true;
      } else if (fx > 124){
        fx = 124;
        hitWall = true;
      } else {
        break;
      }
    }
  }
  else  // Frogger not yet released
  {
    // Release logs if any button pressed
    pad3 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) || arduboy.pressed(UP_BUTTON) || arduboy.pressed(RIGHT_BUTTON) || arduboy.pressed(DOWN_BUTTON) || arduboy.pressed(LEFT_BUTTON);
    if (pad3 == true && oldpad3 == false)
    {
      released = true;
      playToneTimed(523, 75);
    }
    oldpad3 = pad3;
  }
}

void drawLogs()
{
  arduboy.drawRect(log1a[0], fyposArr[1]-3, logLengths[0], 7, 0);
  arduboy.drawRect(log1a[1], fyposArr[1]-3, logLengths[0], 7, 0);
  arduboy.drawRect(log1a[2], fyposArr[1]-3, logLengths[0], 7, 0);
  arduboy.drawRect(log1a[3], fyposArr[1]-3, logLengths[0], 7, 0);
  arduboy.drawRect(log1a[4], fyposArr[1]-3, logLengths[0], 7, 0);

  arduboy.drawRect(log2a[0], fyposArr[2]-3, logLengths[1], 7, 0);
  arduboy.drawRect(log2a[1], fyposArr[2]-3, logLengths[1], 7, 0);
  arduboy.drawRect(log2a[2], fyposArr[2]-3, logLengths[1], 7, 0);
  arduboy.drawRect(log2a[3], fyposArr[2]-3, logLengths[1], 7, 0);

  arduboy.drawRect(log3a[0], fyposArr[3]-3, logLengths[2], 7, 0);
  arduboy.drawRect(log3a[1], fyposArr[3]-3, logLengths[2], 7, 0);
  arduboy.drawRect(log3a[2], fyposArr[3]-3, logLengths[2], 7, 0);

  arduboy.drawRect(log4a[0], fyposArr[4]-3, logLengths[3], 7, 0);
  arduboy.drawRect(log4a[1], fyposArr[4]-3, logLengths[3], 7, 0);
  arduboy.drawRect(log4a[2], fyposArr[4]-3, logLengths[3], 7, 0);
  arduboy.drawRect(log4a[3], fyposArr[4]-3, logLengths[3], 7, 0);

  arduboy.drawRect(log5a[0], fyposArr[5]-3, logLengths[4], 7, 0);
  arduboy.drawRect(log5a[1], fyposArr[5]-3, logLengths[4], 7, 0);
  arduboy.drawRect(log5a[2], fyposArr[5]-3, logLengths[4], 7, 0);
  arduboy.drawRect(log5a[3], fyposArr[5]-3, logLengths[4], 7, 0);

  moveLogs();

  arduboy.drawRect(log5a[0], fyposArr[5]-3, logLengths[4], 7, 1);
  arduboy.drawRect(log5a[1], fyposArr[5]-3, logLengths[4], 7, 1);
  arduboy.drawRect(log5a[2], fyposArr[5]-3, logLengths[4], 7, 1);
  arduboy.drawRect(log5a[3], fyposArr[5]-3, logLengths[4], 7, 1);

  arduboy.drawRect(log4a[0], fyposArr[4]-3, logLengths[3], 7, 1);
  arduboy.drawRect(log4a[1], fyposArr[4]-3, logLengths[3], 7, 1);
  arduboy.drawRect(log4a[2], fyposArr[4]-3, logLengths[3], 7, 1);
  arduboy.drawRect(log4a[3], fyposArr[4]-3, logLengths[3], 7, 1);

  arduboy.drawRect(log3a[0], fyposArr[3]-3, logLengths[2], 7, 1);
  arduboy.drawRect(log3a[1], fyposArr[3]-3, logLengths[2], 7, 1);
  arduboy.drawRect(log3a[2], fyposArr[3]-3, logLengths[2], 7, 1);

  arduboy.drawRect(log2a[0], fyposArr[2]-3, logLengths[1], 7, 1);
  arduboy.drawRect(log2a[1], fyposArr[2]-3, logLengths[1], 7, 1);
  arduboy.drawRect(log2a[2], fyposArr[2]-3, logLengths[1], 7, 1);
  arduboy.drawRect(log2a[3], fyposArr[2]-3, logLengths[1], 7, 1);

  arduboy.drawRect(log1a[0], fyposArr[1]-3, logLengths[0], 7, 1);
  arduboy.drawRect(log1a[1], fyposArr[1]-3, logLengths[0], 7, 1);
  arduboy.drawRect(log1a[2], fyposArr[1]-3, logLengths[0], 7, 1);
  arduboy.drawRect(log1a[3], fyposArr[1]-3, logLengths[0], 7, 1);
  arduboy.drawRect(log1a[4], fyposArr[1]-3, logLengths[0], 7, 1);
}

void moveLogs(){
  if(released)
  {
    for (int index = 0; index < 5; index++){
      log1a[index] -- ;
      log1a[index] -- ;
      if (log1a[index] < -35){
        log1a[index] = 140;
      }
    }
    for (int index = 0; index < 4; index++){
      log2a[index] ++ ;
      if (log2a[index] > 150){
        log2a[index] = -50;
      }
    }
    for (int index = 0; index < 3; index++){
      log3a[index] ++ ;
      log3a[index] ++ ;
      log3a[index] ++ ;
      if (log3a[index] > 140){
        log3a[index] = -70;
      }
    }
    for (int index = 0; index < 4; index++){
      log4a[index] -- ;
      log4a[index] -- ;
      if (log4a[index] < -35){
        log4a[index] = 140;
      }
    }
    for (int index = 0; index < 4; index++){
      log5a[index] ++ ;
      log5a[index] ++ ;
      if (log5a[index] > 165){
        log5a[index] = -55;
      }
    }
  }
}

void drawFrogger()
{
  // Remove Frogger
  arduboy.fillRect(fx-3, fy-3, 7, 7, 0);
  // Draw logs and move Frogger
  drawLogs();
  moveFrogger();
  // Draw Frogger
  arduboy.fillRect(fx-3, fy, 7, 2, 1);
  arduboy.fillRect(fx-1, fy-2, 3, 4, 1);
  arduboy.fillRect(fx-2, fy+2, 2, 1, 1);
  arduboy.fillRect(fx+1, fy+2, 2, 1, 1);
  arduboy.fillRect(fx-2, fy-3, 2, 1, 1);
  arduboy.fillRect(fx+1, fy-3, 2, 1, 1);
  arduboy.drawPixel(fx-1, fy+3, 1);
  arduboy.drawPixel(fx+1, fy+3, 1);
}

void drawGameOver()
{
  arduboy.clear(); // Clear Screen
  arduboy.drawRect(0, 0, 128, 64, 1); // Draw border
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
  arduboy.fillRect(42,25,50,28,0);
  arduboy.setCursor(52, 35);
  arduboy.print("PAUSE");
  arduboy.display();
  while (paused)
  {
    arduboy.delayShort(150);
    //Unpause if FIRE is pressed
    pad2 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if (pad2 == true && oldpad2 == false && released)
    {
        arduboy.fillRect(42,25,50,28,0);
        drawBackground();

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
  arduboy.setCursor(22,12);
  arduboy.setTextSize(2);
  arduboy.print("FROGGER");
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
