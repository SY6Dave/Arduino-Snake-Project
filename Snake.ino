//
//Snake Arduino Project coded by David Mortiboy in February 2015.
//Current student of Computer Science at the University of Hull
//Contact: www.davidmortiboy.com
//GITHUB: SY6Dave
//
#include <TrueRandom.h>

//assign the pins on the arduino with the corresponding row/columns on the display
const int row[8] = {3,8,9,6,A2,10,A1,12};
const int col[8] = {7,A0,13,4,11,5,A3,A4};
const int buttonPin = A5;
const int button2Pin = 2;
int buttonState = 0;
int lastButtonState = 0;
int button2State = 0;
int lastButton2State = 0;
//2-dimensional array of pixels:
int pixels[8][8];    

//length of tail and number of moves
int tailCount = 0;
int moveCount = 0;

//to create instances of blobs/head
struct vector2d
{
  int x;
  int y;
  int dir; //0 for right, 1 for down, 2 for left, 3 for up
};

vector2d blob;
vector2d head;

//don't need to store any more than 64 moves because the tail can never grow larger than this
vector2d moves[64];

//for accurate "frames" per second
unsigned long currentmillis, elapsed;

boolean blobPlaced = false;

void setup()
{
  //set up all the row/col pins as output to the display, and two buttons as inputs
  for(int i = 0; i < 8; i++)
  {
    pinMode(row[i],OUTPUT);
    pinMode(col[i],OUTPUT);
  }
  
  pinMode(buttonPin,INPUT);
  pinMode(button2Pin,INPUT);
  
  Serial.begin(9600); //debugging
}

void loop()
{
  //check if there's currently a blob on the screen, if not, place one down
  if(!blobPlaced) PlaceBlob();

  //are either of the buttons being pressed?
  buttonState = digitalRead(buttonPin);
  button2State = digitalRead(button2Pin);
  
  //if half a second has passed, move (this game runs at 2 fps)
  if(TwoPerSecond())
  {
    Move();
  }
  
  //checks that the button is being pressed once and only once
  if(buttonState==1 && lastButtonState==0)
  {
    AntiClockwise();
  }

  if(button2State==1 && lastButton2State==0)
    {
      Clockwise();
    }
  
  //get rid of everything, we will draw the new positions
  clearPixels();
  
  //if the snake goes off the screen, they need to die, otherwise, draw them
  if(!OffScreen())
  {
    drawPixel(head.x,head.y);
  }
  else
  {
    die();
  }
  
  //when a blob is collected, increase tail count and prepare to place a new one on next iteration
  if(CollisionWithBlob())
  {
    tailCount++;
    blobPlaced = false;
  }
  
  if(CollisionWithSelf())
  {
    die();
  }
  
  drawBlob();
  drawTail();
  
  //refresh the screen 5 times because otherwise pixel isn't very bright
  int t = 0;
  while(t < 5)
  {
    refreshScreen();
    t++;
  }
  
  //store previous button states so we know when the button is pressed only once
  lastButtonState = buttonState;
  lastButton2State = button2State;
}

//this method simply sets every single pixel  to LOW to turn off
void clearPixels()
{
  for(int x = 0; x < 8; x++)
  {
    for(int y = 0; y < 8; y++)
    {
      pixels[x][y]=LOW;
    }
  }
}

//this method sets the specified co-ords to HIGH to turn on
void drawPixel(int x, int y)
{
  pixels[y][x]=HIGH;
}

//similar to previous, except always draws the blobs
void drawBlob()
{
  pixels[blob.y][blob.x]=HIGH;
}

//this method loops backwards through the saved moves, the number of times that the tail
//length should be, and sets those pixels to high to make our tail visible
void drawTail()
{
  int i = tailCount;
  int j = moveCount;
  while(i > 0)
  {
    pixels[moves[j].y][moves[j].x]=HIGH;
    j--;
    i--;
  }
}

//this method adapted from the arduino tutorial on RowColScanning
void refreshScreen() {
  // iterate over the rows (anodes):
  for (int thisRow = 0; thisRow < 8; thisRow++) {
    // take the row pin (anode) high:
    digitalWrite(row[thisRow], HIGH);
    // iterate over the cols (cathodes):
    for (int thisCol = 0; thisCol < 8; thisCol++) {
      // get the state of the current pixel;
      int thisPixel = !pixels[thisRow][thisCol];
      // when the row is HIGH and the col is LOW,
      // the LED where they meet turns on:
      digitalWrite(col[thisCol], thisPixel);
      // turn the pixel off:
      if (thisPixel == LOW) {
        digitalWrite(col[thisCol], HIGH);
      }
    }
    // take the row pin low to turn off the whole row:
    digitalWrite(row[thisRow], LOW);
  }
}

//when we die, light up the whole screen, wait a few seconds, then reset all values
void die()
{
  for(int i = 0; i < 8; i++)
  {
    for(int j = 0; j < 8; j++)
    {
      drawPixel(j,i);
    }
  }

  int t = 0;
  
  while(t < 1000)
  {
    refreshScreen();
    t++;
  }
  
  head.x = 0;
  head.y = 0;
  head.dir = 0;
  blobPlaced=false;
  
  tailCount=0;
  moveCount=0;
  
  //to reset the timer
  currentmillis = millis();
  elapsed = currentmillis;
}

//test method for timings
boolean OneSecond()
{
  currentmillis = millis();
  
  if(currentmillis > (elapsed+1000))
  {
    elapsed = currentmillis;
    return true;
  }
  return false;
}

//constantly checks if 500ms has passed, returns true if correct
boolean TwoPerSecond()
{
  currentmillis = millis();
  
  if(currentmillis > (elapsed+500))
  {
    elapsed = currentmillis;

    return true;
  }
  return false;
}

//these two methods change the direction and wraps around if out of bounds
void Clockwise()
{
  if(head.dir==3)
  {
    head.dir = 0;
  }
  else
  {
    head.dir++;
  }
}

void AntiClockwise()
{
  if(head.dir==0)
  {
    head.dir = 3;
  }
  else
  {
    head.dir--;
  }
}

void Move()
{
  //first, make sure we don't go out of bounds of the array
  if(moveCount==63) WrapAround();
  //there has been a new move, so up the counter
  moveCount++;
  //store the x and y position of the player in the array at the correct index
  moves[moveCount].x = head.x;
  moves[moveCount].y = head.y;
  //handy switch statement tells us which way the snake should go
  switch(head.dir)
  {
    case 0:
      head.x++;
      break;
    case 1:
      head.y++;
      break;
    case 2:
      head.x--;
      break;
    case 3:
      head.y--;
      break;
  }
}

//just checks the boundaries of the matrix
boolean OffScreen()
{
  if(head.x > 7) return true;
  if(head.y > 7) return true;
  if(head.x < 0) return true;
  if(head.y < 0) return true;
  
  return false;
}

//this method utilizes the TrueRandom class to get me a new random number every time
//the arduino loads up
void PlaceBlob()
{
  int randNumberX, randNumberY;
  //continue searching for a new x and y position so long as the positions it's finding are
  //colliding with either the head or tail
  do
  {
    randNumberX = TrueRandom.random(8);
    blob.x = randNumberX;

     randNumberY = TrueRandom.random(8);
     blob.y = randNumberY;
  }while(((randNumberY==head.y) && (randNumberX==head.x)) || (!IsPlaceable(randNumberX, randNumberY)));
 
 //makes sure we don't place another on the next loop iteration
 blobPlaced = true;
}

//this uses the same algorithm as drawTail() to loop through all tail positions and
//see if the two parameters are directly colliding with any of the tail blobs. If there's no
//collision then return true
boolean IsPlaceable(int x, int y)
{
  int i = tailCount;
  int j = moveCount;
  while(i > 0)
  {
    if((y==moves[j].y) && (x==moves[j].x)) return false;
    j--;
    i--;
  }
  
  return true;
}

//checks if the x and y positions of the head are equal to the x and y of the blob
boolean CollisionWithBlob()
{
  if((head.x == blob.x) && (head.y == blob.y)) return true;
  return false;
}

//again, loops through the tail 'entities' in the moves array to check if the head
//position is colliding with any of the tail positions
boolean CollisionWithSelf()
{
  int i = tailCount;
  int j = moveCount;
  while(i > 0)
  {
    if((head.y==moves[j-1].y) && (head.x==moves[j-1].x)) return true;
    j--;
    i--;
  }
  
  return false;
}

//this ensures we never go out of bounds of the moves array without losing the tail
void WrapAround()
{
  //first, reset moveCount so we can start overwriting the array
  moveCount = 0;
  //for each tail item, increase the move counter and place the earliest tail required into that
  //index of the array
  int i = tailCount;
  while(i > 0)
  {
    moveCount++;
    moves[moveCount]=moves[64-i];
    i--;
  }
}
