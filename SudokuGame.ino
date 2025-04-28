#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
#include <LCDWIKI_TOUCH.h> //touch screen library

#include "Logo.h"

/**************************************************************************************/

#define BLACK     0x0000
#define BLUE      0x001F
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define PINK      0xFB78
#define GRAY      0b1100011000011000
#define LIGHTGRAY 0b1110011100011100
#define DARKGRAY  0b1000010000010000

/**************************************************************************************/

#define ButtonFaceColor GRAY
#define ButtonLightColor LIGHTGRAY
#define ButtonDarkColor DARKGRAY

#define WindowTitleBackColor 0b0001000010000010
#define WindowTitleTextColor GREEN
#define WindowBackColor 0b0010000100000100
#define WindowFrameColor 0b1111101111100100
#define WindowFrameBorderColor 0b1111110111100100
#define WindowShadowColor 0b0111101111100010

/**************************************************************************************/

// Grids positions
#define GameBoardPositionX 0
#define GameBoardPositionY 50
#define GameBoardTextSize 2

#define GameNumbersPositionX 20
#define GameNumbersPositionY 395
#define GameNumbersTextSize 2

// Colors
#define GameTimeColor WHITE

// Easy
#define GameDifficulty_Easy 1
#define BlankedCells_Easy_Min 27
#define BlankedCells_Easy_Max 35

#define GameBoard_Easy_GridColor GREEN
#define GameBoard_Easy_FixedTextColor GREEN
#define GameBoard_Easy_BlankedTextColor YELLOW
#define GameBoard_Easy_BackColor 0b0010000100000100
#define GameBoard_Easy_SelectedBackColor 0b0111101111100000

#define GameNumbers_Easy_GridColor GREEN
#define GameNumbers_Easy_TextColor YELLOW
#define GameNumbers_Easy_BackColor 0b0010000100000100
#define GameNumbers_Easy_SelectedBackColor 0b0111101111100000

// Normal
#define GameDifficulty_Normal 2
#define BlankedCells_Normal_Min 37
#define BlankedCells_Normal_Max 45

#define GameBoard_Normal_GridColor YELLOW
#define GameBoard_Normal_FixedTextColor YELLOW
#define GameBoard_Normal_BlankedTextColor CYAN
#define GameBoard_Normal_BackColor 0b0010000100000100
#define GameBoard_Normal_SelectedBackColor 0b0111101111100000

#define GameNumbers_Normal_GridColor YELLOW
#define GameNumbers_Normal_TextColor CYAN
#define GameNumbers_Normal_BackColor 0b0010000100000100
#define GameNumbers_Normal_SelectedBackColor 0b0111101111100000

// Hard
#define GameDifficulty_Hard 3
#define BlankedCells_Hard_Min 47
#define BlankedCells_Hard_Max 55

#define GameBoard_Hard_GridColor RED
#define GameBoard_Hard_FixedTextColor RED
#define GameBoard_Hard_BlankedTextColor PINK
#define GameBoard_Hard_BackColor 0b0010000100000100
#define GameBoard_Hard_SelectedBackColor 0b0111101111100000

#define GameNumbers_Hard_GridColor RED
#define GameNumbers_Hard_TextColor PINK
#define GameNumbers_Hard_BackColor 0b0010000100000100
#define GameNumbers_Hard_SelectedBackColor 0b0111101111100000

// ----

uint16_t GameBoardGridColor;
uint16_t GameBoardFixedTextColor;
uint16_t GameBoardBlankedTextColor;
uint16_t GameBoardBackColor;
uint16_t GameBoardSelectedBackColor;

uint16_t GameNumbersGridColor;
uint16_t GameNumbersTextColor;
uint16_t GameNumbersBackColor;
uint16_t GameNumbersSelectedBackColor;

/**************************************************************************************/

LCDWIKI_KBV tft(ILI9488, 40, 38, 39, 44, 41); //model, cs, cd, wr, rd, reset
LCDWIKI_TOUCH tp(53, 52, 50, 51, 44); //tcs, tclk, tdout, tdin, tirq

#define CharSetWidth 6
#define CharSetHeight 8


/* TouchPad ***************************************************************************/

// Touch IDs
#define TidGameBoard 100  // GameBoard Touch IDs: 100 + y * 10 + x
#define TidNumbers 200    // Numbers Touch IDs:   200 + number (Clear = 10)

struct TouchFieldItem
{
    uint16_t id, x1, y1, x2, y2;
};

TouchFieldItem TouchFields[200];
uint16_t StartOfTouchFields;
uint16_t TouchFieldsCount;

// Store touchable range TouchID
void Set_TouchObject(uint16_t TouchID, int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint16_t Index = StartOfTouchFields + TouchFieldsCount;

  TouchFields[Index].id = TouchID; 
  TouchFields[Index].x1 = x;
  TouchFields[Index].y1 = y;
  TouchFields[Index].x2 = x + w - 1;
  TouchFields[Index].y2 = y + h - 1;

  TouchFieldsCount++;
}

// Read touchpad and find TouchID by touched point
uint16_t Get_TouchObject()
{
  const unsigned long TouchDelayTime = 100;  
  static unsigned long TouchDelayTimer = 1;

  uint16_t TouchID = 0;

  tp.TP_Scan(0);

  if (tp.TP_Get_State() & TP_PRES_DOWN) 
  {
    if (TouchDelayTimer != 0 && millis() > TouchDelayTimer + TouchDelayTime)
    {
      TouchDelayTimer = 0;

      // Find touched item ID
      uint16_t tpx = tp.x;
      uint16_t tpy = tft.Get_Height() - tp.y;

      uint16_t Count = 0;

      while (Count < TouchFieldsCount)
      {
        uint16_t Index = StartOfTouchFields + Count;

        if (tpx >= TouchFields[Index].x1 &&
            tpy >= TouchFields[Index].y1 &&
            tpx <= TouchFields[Index].x2 &&
            tpy <= TouchFields[Index].y2)
        {
          TouchID = TouchFields[Index].id;
          Count = TouchFieldsCount; // Stop search
        }

        Count++;
      }
    }
  }
  else
    if (TouchDelayTimer == 0) TouchDelayTimer = millis();

  return TouchID;
}


/* Button *****************************************************************************/

// Draw button and store TouchID
void Draw_Button(uint16_t TouchID, int16_t x, int16_t y, int16_t w, int16_t h, char *Text, uint8_t TextSize, uint16_t TextColor)
{
  Set_TouchObject(TouchID, x, y, w, h);

  tft.Set_Draw_color(ButtonFaceColor);
  tft.Fill_Rectangle(x + 2, y + 2, x + w - 3, y + h - 3);

  tft.Set_Draw_color(ButtonLightColor);
  tft.Fill_Rectangle(x, y, x + 1, y + h - 3);
  tft.Fill_Rectangle(x + 2, y, x + w - 1, y + 1);

  tft.Set_Draw_color(ButtonDarkColor);
  tft.Fill_Rectangle(x, y + h - 2, x + w - 3, y + h - 1);
  tft.Fill_Rectangle(x + w - 2, y + 2, x + w - 1, y + h - 1);

  tft.Set_Text_colour(TextColor);   
  tft.Set_Text_Size(TextSize);
  tft.Set_Text_Mode(1);
  tft.Print_String(Text, x + (w - strlen(Text) * CharSetWidth * TextSize) / 2, y + (h - CharSetHeight * TextSize) / 2);
}


/* Window *****************************************************************************/

// Draw window
void Draw_Window(int16_t x, int16_t y, int16_t w, int16_t h, int16_t TitleSize, char *TitleText, uint8_t TitleTextSize)
{
  #define BorderWidth 5
  #define CornerRadius 8

  // Title back color
  if (TitleSize > 0)
  {
    tft.Set_Draw_color(WindowTitleBackColor);
    tft.Fill_Rectangle(x + 5, y + 5, x + w - 6, y + TitleSize + 4);

    // Window back color
    tft.Set_Draw_color(WindowBackColor);
    tft.Fill_Rectangle(x + 5, y + TitleSize + 10, x + w - 6, y + h - 6);
  }
  else
  {
    // Window back color
    tft.Set_Draw_color(WindowBackColor);
    tft.Fill_Rectangle(x + 5, y + 5, x + w - 6, y + h - 6);
  }

  // Frame
  tft.Set_Draw_color(WindowFrameColor);

  if (TitleSize > 0) 
    tft.Fill_Rectangle(x + 4, y + TitleSize + 5, x + w - 5, y + TitleSize + 9);

  tft.Draw_Round_Rectangle(x + 1, y + 1, x + w - 2, y + h - 2, CornerRadius - 1);
  tft.Draw_Round_Rectangle(x + 2, y + 2, x + w - 3, y + h - 3, CornerRadius - 2);
  tft.Draw_Round_Rectangle(x + 3, y + 3, x + w - 4, y + h - 4, CornerRadius - 3);

  // Frame border
  tft.Set_Draw_color(WindowFrameBorderColor);
  tft.Draw_Round_Rectangle(x, y, x + w - 1, y + h - 1, CornerRadius);

  if (TitleSize > 0)
  {
    tft.Draw_Round_Rectangle(x + 4, y + 4, x + w - 5, y + TitleSize + 5, CornerRadius - 5);
    tft.Draw_Round_Rectangle(x + 4, y + TitleSize + 9, x + w - 5, y + h - 5, CornerRadius - 5);
  }
  else
  {
    tft.Draw_Round_Rectangle(x + 4, y + 4, x + w - 5, y + h - 5, CornerRadius - 4);
  }

  // Shadow
  tft.Set_Draw_color(WindowShadowColor);

  for (unsigned int i = 0; i <= 3; i++)
  {
 	  tft.Draw_Fast_HLine(x + i + 8, y + h + i, w - 2 * (CornerRadius + i - 1)); 
    tft.Draw_Fast_VLine(x + w + i, y + i + 8, h - 2 * (CornerRadius + i - 1));
 	  tft.Draw_Circle_Helper(x + w - (CornerRadius + 1), y + h - (CornerRadius + 1), CornerRadius + i + 1, 4);
  }

  if (TitleSize > 0)
  {
    tft.Set_Text_colour(WindowTitleTextColor);   
    tft.Set_Text_Size(TitleTextSize);
    tft.Set_Text_Mode(1);
    tft.Print_String(TitleText, x + (w - strlen(TitleText) * CharSetWidth * TitleTextSize) / 2, y + BorderWidth + (TitleSize - CharSetHeight * TitleTextSize) / 2);
  }
}

// Draw window without title
void Draw_Window(int16_t x, int16_t y, int16_t w, int16_t h)
{
  Draw_Window(x, y, w, h, 0, "", 0);
}

/* Game *******************************************************************************/

void Game_DrawLogo(uint16_t x, uint16_t y)
{
  #define LogoSize_w 240
  #define LogoSize_h  40

  // Draw logo
  tft.Set_Addr_Window(x, y, x + LogoSize_w - 1, y + LogoSize_h - 1); 
  tft.Push_Any_Color(Logo, LogoSize_w * LogoSize_h, 1, 1);
}


#define GameBoardGridSize 32

// Game board
struct GameBoardItem
{
  uint8_t Value;
  bool Blank;
};

struct GameBoardTempItem
{
  uint8_t RemainValues[9];
  uint8_t NoOfRemainValues;
};

GameBoardItem GameBoard[9][9];

// Grid fields coords (upper-left)
const unsigned int GridFields_X_Coords[9] = {5, 39, 73, 110, 144, 178, 215, 249, 283};
const unsigned int GridFields_Y_Coords[9] = {5, 39, 73, 110, 144, 178, 215, 249, 283};

uint8_t Game_SelectedNumber;  // 0: nothing selected, 1 - 9, 10: clear

// Generate Game Board Numbers
void Game_GenerateBoard()
{
  GameBoardTempItem GameBoardTemp[9][9];
  bool TableOk;

  randomSeed(analogRead(0));

  do
  {
    TableOk = true; 

    // Initialize tables
    for (int8_t x = 0; x < 9; x++)
      for (int8_t y = 0; y < 9; y++)
      {
        GameBoard[x][y].Value = 0;
        GameBoard[x][y].Blank = false;

        for (int8_t v = 0; v < 9; v++)
          GameBoardTemp[x][y].RemainValues[v] = (int8_t)(v + 1);

        GameBoardTemp[x][y].NoOfRemainValues = 9;
      }

    int f = 0;

    while (f++ < 81)
    {
      // Find first field with minimum number of remain values
      int8_t x = 10, y = 10, MinRemain = 10;
      bool FoundField = false;

      for (int8_t xm = 0; xm < 9; xm++)
        for (int8_t ym = 0; ym < 9; ym++)
        {
          if ((GameBoard[xm][ym].Value == 0) &&
              (GameBoardTemp[xm][ym].NoOfRemainValues > 0) &&
              (GameBoardTemp[xm][ym].NoOfRemainValues < MinRemain))
          {
            MinRemain = GameBoardTemp[xm][ym].NoOfRemainValues;
            x = xm;
            y = ym;

            FoundField = true;
          }
        }

      if (FoundField)
      {
        int8_t ValueIndex = (int8_t)random(1, GameBoardTemp[x][y].NoOfRemainValues + 1);
        int v = 0;

        while (ValueIndex > 0)
        {
          if (GameBoardTemp[x][y].RemainValues[v] != 0) ValueIndex--;
          v++;
        }

        int8_t Value = GameBoardTemp[x][y].RemainValues[v - 1];

        // Store field value
        GameBoard[x][y].Value = Value;
        GameBoardTemp[x][y].NoOfRemainValues = 0;

        //  Clear possible values
        for (int8_t cv = 0; cv < 9; cv++)
        {
          if (GameBoardTemp[x][cv].RemainValues[Value - 1] != 0)
          {
            GameBoardTemp[x][cv].RemainValues[Value - 1] = 0;
            GameBoardTemp[x][cv].NoOfRemainValues--;
          }

          if (GameBoardTemp[cv][y].RemainValues[Value - 1] != 0)
          {
            GameBoardTemp[cv][y].RemainValues[Value - 1] = 0;
            GameBoardTemp[cv][y].NoOfRemainValues--;
          }
        }

        int8_t x30 = (byte)((x / 3) * 3);
        int8_t y30 = (byte)((y / 3) * 3);

        for (int8_t x3 = 0; x3 < 3; x3++)
          for (int8_t y3 = 0; y3 < 3; y3++)
            if (GameBoardTemp[x30 + x3][y30 + y3].RemainValues[Value - 1] != 0)
            {
              GameBoardTemp[x30 + x3][y30 + y3].RemainValues[Value - 1] = 0;
              GameBoardTemp[x30 + x3][y30 + y3].NoOfRemainValues--;
            }
      }
      else
          TableOk = false;
    }
  } while (!TableOk);
}

void Game_GenerateBlankFields(int8_t NoOFBlankFields)
{
  int8_t BlankedCount = 0;

  randomSeed(analogRead(0));

  for (int8_t bf = 0; bf < 9; bf++)
  {
    int8_t bx = (int8_t)random(9);
    int8_t by = (int8_t)random(9);

    if (!GameBoard[bx][bf].Blank)
    {
      GameBoard[bx][bf].Value = 0;
      GameBoard[bx][bf].Blank = true;

      BlankedCount++;
    }

    if (!GameBoard[bf][by].Blank)
    {
      GameBoard[bf][by].Value = 0;
      GameBoard[bf][by].Blank = true;

      BlankedCount++;
    }
  }

  for (int8_t bfx = 0; bfx < 9; bfx += 3)
    for (int8_t bfy = 0; bfy < 9; bfy += 3)
    {
      int8_t bx = (int8_t)random(3);
      int8_t by = (int8_t)random(3);

      if (!GameBoard[bfx + bx][bfy + by].Blank)
      {
        GameBoard[bfx + bx][bfy + by].Value = 0;
        GameBoard[bfx + bx][bfy + by].Blank = true;

        BlankedCount++;
      }
    }

  while (BlankedCount < NoOFBlankFields)
  {
    int8_t bx = (int8_t)random(9);
    int8_t by = (int8_t)random(9);

    if (!GameBoard[bx][by].Blank)
    {
      GameBoard[bx][by].Value = 0;
      GameBoard[bx][by].Blank = true;

      BlankedCount++;
    }
  }
}

bool Game_CheckBoard()
{
  bool TableOk = true;
  int8_t FieldValue;

  for (int8_t x = 0; x < 9; x++)
    for (int8_t y = 0; y < 9; y++)
    {
      FieldValue = GameBoard[x][y].Value;

      // Rmpty field
      if (TableOk)
        TableOk = (FieldValue != 0);

      // Vertival and horizontal chaeck
      if (TableOk)
      {
        for (int8_t v = 0; v < 9; v++)
        {
          if ((v != y) && (GameBoard[x][v].Value == FieldValue))
            TableOk = false;

          if ((v != x) && (GameBoard[v][y].Value == FieldValue))
            TableOk = false;
        }
      }

      // 3 x 3
      if (TableOk)
      {
        int8_t x30 = (x / 3) * 3;
        int8_t y30 = (y / 3) * 3;

        for (int8_t x3 = 0; x3 < 3; x3++)
          for (int8_t y3 = 0; y3 < 3; y3++)
            if (((x30 + x3 != x) || (y30 + y3 != y)) && (GameBoard[x30 + x3][y30 + y3].Value == FieldValue))
                TableOk = false;
      }
    }

  return TableOk;
}

// Draw game grid item with number and fill background
void Game_FillGameGridItem(uint8_t x, uint8_t y)
{
  // Background
  if (GameBoard[x][y].Value == Game_SelectedNumber && Game_SelectedNumber != 0) tft.Set_Draw_color(GameBoardSelectedBackColor);
  else tft.Set_Draw_color(GameBoardBackColor);
  
  tft.Fill_Rectangle(GridFields_X_Coords[x] + GameBoardPositionX, GridFields_Y_Coords[y] + GameBoardPositionY, GridFields_X_Coords[x] + GameBoardPositionX + GameBoardGridSize - 1, GridFields_Y_Coords[y] + GameBoardPositionY + GameBoardGridSize - 1);

  // Draw number
  if (GameBoard[x][y].Value != 0)
  {
    tft.Set_Text_Size(GameBoardTextSize);
    tft.Set_Text_Mode(1);

    if (GameBoard[x][y].Blank) tft.Set_Text_colour(GameBoardBlankedTextColor);
    else tft.Set_Text_colour(GameBoardFixedTextColor);

    tft.Print_String(String(GameBoard[x][y].Value), 
                    GridFields_X_Coords[x] + GameBoardPositionX + ((GameBoardGridSize - CharSetWidth * GameBoardTextSize) / 2) + 1, 
                    GridFields_Y_Coords[y] + GameBoardPositionY + ((GameBoardGridSize - CharSetHeight * GameBoardTextSize) / 2) + 1);
  }
}

// Fill game grid with numbers
void Game_FillGameGrid()
{
  for (int8_t x = 0; x < 9; x++)
    for (int8_t y = 0; y < 9; y++)
    {
      Game_FillGameGridItem(x, y);
      if (GameBoard[x][y].Blank) Set_TouchObject(TidGameBoard + y * 10 + x, GridFields_X_Coords[x] + GameBoardPositionX, GridFields_Y_Coords[y] + GameBoardPositionY, GameBoardGridSize, GameBoardGridSize); 
    }
}

// Draw game board
void Game_DrawBoard()
{
  uint16_t gx, gy;

  // Grid
  tft.Set_Draw_color(GameBoardGridColor);

  for (uint16_t y = 0; y <= 8; y++)
    for (uint16_t x = 0; x <= 8; x++)
    {
      gx = GridFields_X_Coords[x] + GameBoardPositionX;
      gy = GridFields_Y_Coords[y] + GameBoardPositionY;
    
      tft.Draw_Rectangle(gx - 1, gy - 1, gx + GameBoardGridSize, gy + GameBoardGridSize); 
    }

  tft.Fill_Rectangle(GridFields_X_Coords[0] + GameBoardPositionX - 1, GridFields_Y_Coords[0] + GameBoardPositionY - 5, GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize, GridFields_Y_Coords[0] + GameBoardPositionY - 2);
  tft.Fill_Rectangle(GridFields_X_Coords[0] + GameBoardPositionX - 1, GridFields_Y_Coords[3] + GameBoardPositionY - 4, GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize, GridFields_Y_Coords[3] + GameBoardPositionY - 2);
  tft.Fill_Rectangle(GridFields_X_Coords[0] + GameBoardPositionX- 1, GridFields_Y_Coords[6] + GameBoardPositionY - 4, GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize, GridFields_Y_Coords[6] + GameBoardPositionY - 2);
  tft.Fill_Rectangle(GridFields_X_Coords[0] + GameBoardPositionX - 1, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 1, GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 4);

  tft.Fill_Rectangle(GridFields_X_Coords[0] + GameBoardPositionX - 5, GridFields_Y_Coords[0] + GameBoardPositionY - 5, GridFields_X_Coords[0] + GameBoardPositionX - 2, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 4);
  tft.Fill_Rectangle(GridFields_X_Coords[3] + GameBoardPositionX - 4, GridFields_Y_Coords[0] + GameBoardPositionY - 1, GridFields_X_Coords[3] + GameBoardPositionX - 2, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 4);
  tft.Fill_Rectangle(GridFields_X_Coords[6] + GameBoardPositionX - 4, GridFields_Y_Coords[0] + GameBoardPositionY - 1, GridFields_X_Coords[6] + GameBoardPositionX - 2, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 4);
  tft.Fill_Rectangle(GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize + 1, GridFields_Y_Coords[0] + GameBoardPositionY - 5, GridFields_X_Coords[8] + GameBoardPositionX + GameBoardGridSize + 4, GridFields_Y_Coords[8] + GameBoardPositionY + GameBoardGridSize + 4);
}

// Set numbers grid item with number and fill background
void Game_FillNumbersGridItem(uint8_t n)
{
  uint16_t x, y;

  x = GameNumbersPositionX + ((n - 1) % 5) * (GameBoardGridSize + 2);
  y = GameNumbersPositionY + (uint16_t)((n - 1) / 5) * (GameBoardGridSize + 2);

  // Background
  if (n == Game_SelectedNumber) tft.Set_Draw_color(GameNumbersSelectedBackColor);
  else tft.Set_Draw_color(GameNumbersBackColor);
  tft.Fill_Rectangle(x, y, x + GameBoardGridSize - 1, y + GameBoardGridSize - 1);

  // Draw number
  tft.Set_Text_colour(GameNumbersTextColor);   
  tft.Set_Text_Size(GameNumbersTextSize);
  tft.Set_Text_Mode(1);

  if (n == 10) // Clear?
    tft.Print_String("C", x + (GameBoardGridSize - 6 * GameNumbersTextSize) / 2, y + (GameBoardGridSize - 8 * GameNumbersTextSize) / 2);
  else
    tft.Print_String(String(n), x + (GameBoardGridSize - 6 * GameNumbersTextSize) / 2, y + (GameBoardGridSize - 8 * GameNumbersTextSize) / 2);
}

// Draw grid with selectable numbers
void Game_DrawNumbersGrid()
{
  uint16_t gx, gy1, gy2;

  for (uint8_t x = 0; x <= 4; x++)
  {
    gx = GameNumbersPositionX + x * (GameBoardGridSize + 2);
    gy1 = GameNumbersPositionY;
    gy2 = GameNumbersPositionY + GameBoardGridSize + 2;

    // Draw numbers frame  
    tft.Set_Draw_color(GameNumbersGridColor);
    tft.Draw_Rectangle(gx - 1, gy1 - 1, gx + GameBoardGridSize, gy1 + GameBoardGridSize); 
    tft.Draw_Rectangle(gx - 1, gy2 - 1, gx + GameBoardGridSize, gy2 + GameBoardGridSize); 

    // Set touchable area
    Set_TouchObject(TidNumbers + x + 1, gx + 1, gy1 + 1, GameBoardGridSize, GameBoardGridSize); 
    Set_TouchObject(TidNumbers + x + 6, gx + 1, gy2 + 1, GameBoardGridSize, GameBoardGridSize); 

    // Draw numbers
    Game_FillNumbersGridItem(x + 1);
    Game_FillNumbersGridItem(x + 6);
  }

  // Draw frame
  tft.Set_Draw_color(GameNumbersGridColor);
  tft.Draw_Rectangle(GameNumbersPositionX - 2, GameNumbersPositionY - 2, GameNumbersPositionX + 5 * (GameBoardGridSize + 2) - 1, GameNumbersPositionY + 2 * (GameBoardGridSize + 2) - 1); 
  tft.Draw_Rectangle(GameNumbersPositionX - 3, GameNumbersPositionY - 3, GameNumbersPositionX + 5 * (GameBoardGridSize + 2), GameNumbersPositionY + 2 * (GameBoardGridSize + 2)); 
}


/**************************************************************************************/

void Game_DisplayTime(uint16_t x, uint16_t y, long Game_Time, uint16_t TimeColor, uint16_t TimeBackColor)
{
  tft.Set_Text_colour(TimeColor);   
  tft.Set_Text_Back_colour(TimeBackColor);
  tft.Set_Text_Size(2);
  tft.Set_Text_Mode(0);

  char Time[6] = " 0:00\0";

  int TimeMin = Game_Time / 60;
  int TimeSec = Game_Time % 60;

  Time[0] = char(48 + TimeMin / 10);
  Time[1] = char(48 + TimeMin % 10);
  Time[3] = char(48 + TimeSec / 10);
  Time[4] = char(48 + TimeSec % 10);

  tft.Print_String(Time, x, y);
}


/* Main menu **************************************************************************/

uint8_t Game_SelectDifficulty()
{
  uint16_t TouchID;

  // Save TouchID table
  uint16_t Temp_StartOfTouchFields = StartOfTouchFields;
  uint16_t Temp_TouchFieldsCount = TouchFieldsCount;

  StartOfTouchFields += TouchFieldsCount;
  TouchFieldsCount = 0;

  // Clear display
  tft.Fill_Screen(0);  

  // Draw logo
  Game_DrawLogo(40, 40); 

  // Draw frame and buttons
  Draw_Window(60, 120, 200, 320);
  Draw_Button(GameDifficulty_Easy, 97, 158, 126, 60, "Easy", 2, GameBoard_Easy_GridColor);
  Draw_Button(GameDifficulty_Normal, 97, 250, 126, 60, "Normal", 2, GameBoard_Normal_GridColor);
  Draw_Button(GameDifficulty_Hard, 97, 342, 126, 60, "Hard", 2, GameBoard_Hard_GridColor);

  // Wait for click
  do
  {
    TouchID = Get_TouchObject();
  } while (TouchID == 0);

  // Restore TouchID table
  StartOfTouchFields = Temp_StartOfTouchFields;
  TouchFieldsCount = Temp_TouchFieldsCount;

  // Clear window
  tft.Fill_Rect(60, 120, 205, 325, BLACK);

  return TouchID;
}


/* Game menu **************************************************************************/

#define GameMenu_Continue 1
#define GameMenu_Restart 2
#define GameMenu_Abort 10


uint8_t Game_Menu()
{
  uint16_t TouchID;

  // Save TouchID table
  uint16_t Temp_StartOfTouchFields = StartOfTouchFields;
  uint16_t Temp_TouchFieldsCount = TouchFieldsCount;

  StartOfTouchFields += TouchFieldsCount;
  TouchFieldsCount = 0;

  // Draw frame and buttons
  Draw_Window(60, 120, 200, 320);
  Draw_Button(GameMenu_Continue, 97, 158, 126, 60, "Continue", 2, BLACK);
  Draw_Button(GameMenu_Restart, 97, 250, 126, 60, "Restart", 2, BLACK);
  Draw_Button(GameMenu_Abort, 97, 342, 126, 60, "Abort", 2, BLACK);

  // Wait for click
  do
  {
    TouchID = Get_TouchObject();
  } while (TouchID == 0);

  // Restore TouchID table
  StartOfTouchFields = Temp_StartOfTouchFields;
  TouchFieldsCount = Temp_TouchFieldsCount;

  // Clear window
  tft.Fill_Rect(60, 120, 205, 325, BLACK);

  return TouchID;
}


/* Game finished **********************************************************************/

#define GameFinished_Ok 1

uint8_t Game_Finished(long Game_Time)
{
  uint16_t TouchID;

  // Save TouchID table
  uint16_t Temp_StartOfTouchFields = StartOfTouchFields;
  uint16_t Temp_TouchFieldsCount = TouchFieldsCount;

  StartOfTouchFields += TouchFieldsCount;
  TouchFieldsCount = 0;

  // Draw frame and buttons
  Draw_Window(60, 120, 200, 200);

  tft.Set_Text_colour(WHITE);   
  tft.Set_Text_Back_colour(WindowBackColor);
  tft.Set_Text_Size(2);
  tft.Set_Text_Mode(0);

  tft.Print_String("Finished!", 106, 148);
  tft.Print_String("Your time:", 100, 182);
  Game_DisplayTime(130, 206, Game_Time, WHITE, WindowBackColor);

  Draw_Button(1, 97, 245, 126, 50, "Ok", 2, BLACK);

  // Wait for click
  do
  {
    TouchID = Get_TouchObject();
  } while (TouchID == 0);

  // Restore TouchID table
  StartOfTouchFields = Temp_StartOfTouchFields;
  TouchFieldsCount = Temp_TouchFieldsCount;

  // Clear window
  tft.Fill_Rect(60, 120, 205, 205, BLACK);

  return TouchID;
}


/* Game play **************************************************************************/

void Game_Play(uint8_t Game_Difficulty)
{
  #define MenuID 1000

  uint16_t TouchID;
  bool Game_Exit = false;
  bool Game_Repaint;

  // Game timer
  long Game_StartOfTime;

  // Select difficulty
  uint8_t Game_BlankedCells = 0;

  randomSeed(analogRead(0));

  // Set game colors and blanked cells by difficulty
  switch(Game_Difficulty)
  {
    case GameDifficulty_Easy:
      Game_BlankedCells = random(BlankedCells_Normal_Min, BlankedCells_Normal_Max + 1);

      GameBoardGridColor = GameBoard_Easy_GridColor;
      GameBoardFixedTextColor = GameBoard_Easy_FixedTextColor;
      GameBoardBlankedTextColor = GameBoard_Easy_BlankedTextColor;
      GameBoardBackColor = GameBoard_Easy_BackColor;
      GameBoardSelectedBackColor = GameBoard_Easy_SelectedBackColor;

      GameNumbersGridColor = GameNumbers_Easy_GridColor;
      GameNumbersTextColor = GameNumbers_Easy_TextColor;
      GameNumbersBackColor = GameNumbers_Easy_BackColor;
      GameNumbersSelectedBackColor = GameNumbers_Easy_SelectedBackColor;
      break;

    case GameDifficulty_Normal:
      Game_BlankedCells = random(BlankedCells_Normal_Min, BlankedCells_Normal_Max + 1);

      GameBoardGridColor = GameBoard_Normal_GridColor;
      GameBoardFixedTextColor = GameBoard_Normal_FixedTextColor;
      GameBoardBlankedTextColor = GameBoard_Normal_BlankedTextColor;
      GameBoardBackColor = GameBoard_Normal_BackColor;
      GameBoardSelectedBackColor = GameBoard_Normal_SelectedBackColor;

      GameNumbersGridColor = GameNumbers_Normal_GridColor;
      GameNumbersTextColor = GameNumbers_Normal_TextColor;
      GameNumbersBackColor = GameNumbers_Normal_BackColor;
      GameNumbersSelectedBackColor = GameNumbers_Normal_SelectedBackColor;
      break;

    case GameDifficulty_Hard:
      Game_BlankedCells = random(BlankedCells_Hard_Min, BlankedCells_Hard_Max + 1);

      GameBoardGridColor = GameBoard_Hard_GridColor;
      GameBoardFixedTextColor = GameBoard_Hard_FixedTextColor;
      GameBoardBlankedTextColor = GameBoard_Hard_BlankedTextColor;
      GameBoardBackColor = GameBoard_Hard_BackColor;
      GameBoardSelectedBackColor = GameBoard_Hard_SelectedBackColor;

      GameNumbersGridColor = GameNumbers_Hard_GridColor;
      GameNumbersTextColor = GameNumbers_Hard_TextColor;
      GameNumbersBackColor = GameNumbers_Hard_BackColor;
      GameNumbersSelectedBackColor = GameNumbers_Hard_SelectedBackColor;
      break;

  }

  // Game init
  Game_SelectedNumber = 0;

  Game_GenerateBoard();
  Game_GenerateBlankFields(Game_BlankedCells);

  Game_StartOfTime = millis();

  // Save TouchID table
  uint16_t Temp_StartOfTouchFields = StartOfTouchFields;
  uint16_t Temp_TouchFieldsCount = TouchFieldsCount;

  // Clear display
  tft.Fill_Screen(0);  

  // Loop for repaint game board
  do
  {
    Game_Repaint = false;

    // Draw logo
    Game_DrawLogo(0, 0); 

    // Draw game time
    tft.Set_Text_colour(GameTimeColor);   
    tft.Set_Text_Size(2);
    tft.Set_Text_Mode(1);
    tft.Print_String("Time", 260, 0);

    // Paint game board
    Game_DrawBoard();
    Game_FillGameGrid();

    Game_DrawNumbersGrid();

    Draw_Button(MenuID, 
                5 * GameBoardGridSize + 14 + 2 * GameNumbersPositionX + 3,
                GameNumbersPositionY - 3, 
                320 - 5 * GameBoardGridSize - 14 - 3 * GameNumbersPositionX, 
                GameBoardGridSize * 2 + 8,
                "Menu", 2, BLACK);

    // Game main loop
    do
    {
      // Display elpsed game time
      static long Game_PrevTime = -1;
      long Game_Time = (millis() - Game_StartOfTime) >> 10;

      if (Game_Time != Game_PrevTime)
      {
        Game_PrevTime = Game_Time;
        Game_DisplayTime(255, 18, Game_Time, GameTimeColor, BLACK);
      }

      // Get touch id
      TouchID = Get_TouchObject();

      // Menu button touched?
      if (TouchID == MenuID)
      {
        // Stop Game time while menu open
        unsigned long MenuStartTime = millis();

        switch (Game_Menu())
        {
          case GameMenu_Continue:
            Game_StartOfTime += millis() - MenuStartTime;
            break;

          case GameMenu_Restart:
            for (int8_t x = 0; x < 9; x++)
              for (int8_t y = 0; y < 9; y++)
                if (GameBoard[x][y].Blank) GameBoard[x][y].Value = 0;
          
            Game_SelectedNumber = 0;
            Game_StartOfTime = millis();
            break;

          case GameMenu_Abort:
            Game_Exit = true;
            break;
        }
        
        Game_Repaint = true;
      }

      // Game numbers touched?
      if (TouchID >= TidNumbers && TouchID <= TidNumbers + 10)
      {
        uint8_t Game_PrevSelectedNumber = Game_SelectedNumber;

        if (Game_SelectedNumber == TouchID - TidNumbers) Game_SelectedNumber = 0;
        else Game_SelectedNumber = TouchID - TidNumbers;

        if (Game_PrevSelectedNumber != 0) Game_FillNumbersGridItem(Game_PrevSelectedNumber);
        if (Game_SelectedNumber != 0) Game_FillNumbersGridItem(Game_SelectedNumber);

        for (int8_t x = 0; x < 9; x++)
          for (int8_t y = 0; y < 9; y++)
            if ((GameBoard[x][y].Value == Game_SelectedNumber && Game_SelectedNumber != 0) || 
                (GameBoard[x][y].Value == Game_PrevSelectedNumber && Game_PrevSelectedNumber != 0))
              Game_FillGameGridItem(x, y);
      }

      // Game board touched?
      if (TouchID >= TidGameBoard && TouchID <= TidGameBoard + 88)
      {
        bool BoardOk = false;

        uint8_t x = (TouchID - TidGameBoard) % 10;
        uint8_t y = (TouchID - TidGameBoard) / 10;

        if (Game_SelectedNumber == 10) // Clear
        {
          GameBoard[x][y].Value = 0;
        }
        else
        {
          GameBoard[x][y].Value = Game_SelectedNumber;

          // Check finished
          BoardOk = Game_CheckBoard();
        }

        Game_FillGameGridItem(x, y);

        // Finished?
        if (BoardOk)
        {
          Game_Finished(Game_Time);

          Game_Repaint = true;
          Game_Exit = true;
        }
      }
    } while(!Game_Repaint);

    StartOfTouchFields = Temp_StartOfTouchFields;
    TouchFieldsCount = Temp_TouchFieldsCount;

  } while(!Game_Exit);
}

/* Setup ******************************************************************************/

void setup(void)
{
  Serial.begin(9600);

  // Init LCD and TouchPad
  tft.Init_LCD();
  tft.Set_Rotation(2);  
  tp.TP_Init(tft.Get_Rotation(), tft.Get_Display_Width(), tft.Get_Display_Height()); 

  // Clear display
  tft.Fill_Screen(0);  

  // Init touchable range store
  StartOfTouchFields = 0;
  TouchFieldsCount = 0;
}


/* Main loop **************************************************************************/

void loop(void)
{
  uint8_t Game_Difficulty;

  // Select difficulty
  Game_Difficulty = Game_SelectDifficulty();

  // Play Game
  Game_Play(Game_Difficulty);
}
