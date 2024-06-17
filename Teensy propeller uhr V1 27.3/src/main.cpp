//1 = 19
//2 = 18
//3 = 14
//4 = 15
//5 = 40
//6 = 41
//7 = 17
//8 = 16




#include <Arduino.h>
#define LEDPIN1 19
#define LEDPIN2 18
#define LEDPIN3 14
#define LEDPIN4 15
#define LEDPIN5 40
#define LEDPIN6 41
#define LEDPIN7 17
#define LEDPIN8 16
#include <math.h>

#define LEDPIN 18
#define T1H 600
#define T1L 650 - 30
#define T0H 250 - 70
#define T0L 1000

#define PixelCount 9
#define Splits 8

int Counter = 0;

elapsedMicros TimeStamp = 0;

uint8_t ShiftOffsets[] = {
  8,
  16,
  0
};

uint8_t PinBits[] = {
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23};

String AnsiColors[] = {
    "\e[0;30m",
    "\e[0;31m",
    "\e[0;32m",
    "\e[0;33m",
    "\e[0;34m",
    "\e[0;35m",
    "\e[0;36m",
    "\e[0;37m"};

enum TerminalColors
{
  Black = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Purple = 5,
  Cyan = 6,
  White = 7
};

enum Colors
{
  RED,
  GREEN,
  BLUE,
};

bool UselessMachine = 0;

/*
17
16
18
19

*/

void PrintColorTerminal(String Text, int Color, bool nl = 0)
{
  Serial.print(AnsiColors[Color]);
  Serial.print(Text);
  if (nl)
  {
    Serial.println(AnsiColors[TerminalColors::White]);
  }
}

uint32_t BackedBuffer[PixelCount * 3 * 3 * 8 * 3];
uint32_t Pixelbuffer[PixelCount * Splits]; /* = {
     0b111111110000000000000000,
     0b000000001111111100000000,
     0b000000000000000011111111,
 };*/

uint32_t OnMask = 0;

uint32_t getColorPacket(int r, int g, int b)
{
  //Serial.println((r << 16) || (g << 8) || b, BIN);
  return (r << 16) | (g << 8) | b;
}

void createTestImage()
{
  int pc = PixelCount * Splits;
  for (float i = 0; i < pc; i++)
  {

    double n = (float)Counter / (float)pc;
    int rot = 0;
    int gruen = 0;// Counter;
    int blau = n*(pc-i);
    Pixelbuffer[int(i)] = getColorPacket(int(rot), int(gruen), int(blau));
    if (i == 30){
    PrintColorTerminal("Color: ", TerminalColors::White,0);
    PrintColorTerminal("  n=" + String(n), TerminalColors::Purple,0);
    PrintColorTerminal("  i=" + String(i), TerminalColors::Purple,0);
    PrintColorTerminal("  R=" + String(rot), TerminalColors::Red,0);
    PrintColorTerminal("  G=" + String(gruen), TerminalColors::Green,0);
    PrintColorTerminal("  B=" + String(blau), TerminalColors::Blue,1);
    }
    
  }
}

void BakePixelOutput()
{
  for (int l = 0; l < PixelCount * 3 * 3 * 8 * 3; l++){
BackedBuffer[l] =0;
  }



  uint32_t BufferIndex = 0;
  for (int i = 0; i < PixelCount; i++)
  {
    for (int l = 0; l < 3; l++)
    {
      for (int o = 0; o < 8; o++)
      {
        BackedBuffer[BufferIndex] = OnMask;
        BufferIndex++;
        if (i < PixelCount){
          BackedBuffer[BufferIndex] = 0;
        }
        for (int k = 0; k < Splits;k++){
          BackedBuffer[BufferIndex] |= (bitRead((Pixelbuffer[i + PixelCount * k] & (0xFF << ShiftOffsets[l])) >> ShiftOffsets[l], 7 - o) << PinBits[k]);
        }
        BufferIndex++;
        BackedBuffer[BufferIndex] = 0;
        BufferIndex++;
        //}
      }
    }

    if (BufferIndex >= PixelCount * 3 * 8 * 3)
    {
      BufferIndex = 0;
    }
  }
}

void writePixel()
{
  for (int i = 0; i < 24; i++)
  {
    uint32_t cyclesStart = ARM_DWT_CYCCNT;

    digitalWrite(18, HIGH); // 24
    while (ARM_DWT_CYCCNT - cyclesStart < T1H)
      Serial.println();
    

    digitalWrite(18, LOW);
    while (ARM_DWT_CYCCNT - cyclesStart < T1L + T1H)
      Serial.println();
  }
}

String DigitFill(int number, int digits){
  int currentDigits = log10(number);
  String ret = "";
  for (int l = 0; l < digits - currentDigits;l++){
    ret += " ";
  }
  ret += String(number);
  return ret;
}

void printBakeBuffer()
{
  for (int i = 0; i < PixelCount * 3 * 3 * 8; i++)
  {
    PrintColorTerminal("#" + DigitFill(i,3) + ":   ",TerminalColors::Purple,0);
    Serial.println(BackedBuffer[i], BIN);
  }
}

void flushPixels()
{
  for (uint32_t n = 0; n < PixelCount * 3 * 3 * 8; n)
  {
    uint32_t cyclesStart = ARM_DWT_CYCCNT;
    GPIO6_DR = GPIO6_DR | BackedBuffer[n]; // setzen
    GPIO6_DR = GPIO6_DR & ~(OnMask & ~BackedBuffer[n]);
    n++;
    delayNanoseconds(T0H);
    GPIO6_DR = GPIO6_DR | BackedBuffer[n]; // setzen
    GPIO6_DR = GPIO6_DR & ~(OnMask & ~BackedBuffer[n]);
    n++;
    delayNanoseconds(T1H - T0H + 8);
    GPIO6_DR = GPIO6_DR | BackedBuffer[n]; // setzen
    GPIO6_DR = GPIO6_DR & ~(OnMask & ~BackedBuffer[n]);
    n++;
    delayNanoseconds(T1L);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(LEDPIN1, OUTPUT);
  pinMode(LEDPIN2, OUTPUT);
  pinMode(LEDPIN3, OUTPUT);
  pinMode(LEDPIN4, OUTPUT);
  pinMode(LEDPIN5, OUTPUT);
  pinMode(LEDPIN6, OUTPUT);
  pinMode(LEDPIN7, OUTPUT);
  pinMode(LEDPIN8, OUTPUT);

  for (int i = 0; i < Splits; i++)
  {
    OnMask |= 0x1 << PinBits[i];
  }

  pinMode(24, OUTPUT);
  delay(5000);
  TimeStamp = 0;
  createTestImage();
  BakePixelOutput();
  Serial.println("\e[0;34mRendering took    " + String(TimeStamp));
  PrintColorTerminal("Dies Ist ein Roter Text", TerminalColors::Red, 1);
  // printBakeBuffer();
}

void loop()
{
  // Serial.println("Ich seh'n Stääärnenhimmel");
  /*
  digitalWrite(24, LOW);
  digitalWrite(18, LOW); // reset
  delayNanoseconds(50000);
  for (int i = 0; i < 3; i++)
  {
    writePixel();
  }
  digitalWrite(24, HIGH);
  delay(100);
  */
  digitalWrite(LEDPIN1, LOW); // reset
  TimeStamp = 0;
  createTestImage();
  PrintColorTerminal("  Imagerendering took: " + String(TimeStamp), TerminalColors::Green,0);
  TimeStamp = 0;
  BakePixelOutput();
  PrintColorTerminal("  Baking took: " + String(TimeStamp), TerminalColors::Blue,0);
  TimeStamp = 0;
  //printBakeBuffer();
  Counter++;
  if (Counter > 254){
    Counter = 0;
  }
  PrintColorTerminal("  Counting took: " + String(TimeStamp), TerminalColors::Yellow,0);
  TimeStamp = 0;

  flushPixels();
  PrintColorTerminal("  Flushing took: " + String(TimeStamp), TerminalColors::Cyan,1);
  TimeStamp = 0;
  //delay(100);
}
