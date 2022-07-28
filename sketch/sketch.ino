// IMPORTANT: LCDWIKI_KBV LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

//This program is a demo of how to show a bmp picture from SD card.

//Set the pins to the correct ones for your development shield or breakout board.
//the 16bit mode only use in Mega.you must modify the mode in the file of lcd_mode.h
//when using the BREAKOUT BOARD only and using these 16 data lines to the LCD,
//pin usage as follow:
//             CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15 
//Arduino Mega 40  38  39  /   41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29

//Remember to set the pins to suit your display module!

/**********************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

#include <SD.h>
#include <SPI.h>
#include <limits.h>
#include <string.h>
#include "src/LCDWIKI_GUI.h" //Core graphics library
#include "src/LCDWIKI_KBV.h" //Hardware-specific library
#include "src/lvgl/lvgl.h"
#include "src/loved.h"

//the definiens of 16bit mode as follow:
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV my_lcd(ILI9486,40,38,39,1,41); //model,cs,cd,wr,rd,reset

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define PIXEL_NUMBER  (my_lcd.Get_Display_Width()/4)
#define FILE_NUMBER 10
#define FILE_NAME_SIZE_MAX 20

uint32_t bmp_offset = 0;
uint16_t s_width = my_lcd.Get_Display_Width();  
uint16_t s_heigh = my_lcd.Get_Display_Height();
//int16_t PIXEL_NUMBER;

char file_name[FILE_NUMBER][FILE_NAME_SIZE_MAX];

uint16_t read16(File fp) {
    uint8_t low;
    uint16_t high;
    low = fp.read();
    high = fp.read();
    return (high<<8)|low;
}

uint32_t read32(File fp) {
    uint16_t low;
    uint32_t high;
    low = read16(fp);
    high = read16(fp);
    return (high<<16)|low;   
 }
 
bool analysis_bpm_header(File fp) {
    if(read16(fp) != 0x4D42)
    {
      return false;  
    }
    //get bpm size
    read32(fp);
    //get creator information
    read32(fp);
    //get offset information
    bmp_offset = read32(fp);
    //get DIB infomation
    read32(fp);
    //get width and heigh information
    uint32_t bpm_width = read32(fp);
    uint32_t bpm_heigh = read32(fp);
    if((bpm_width != s_width) || (bpm_heigh != s_heigh))
    {
      return false; 
    }
    if(read16(fp) != 1)
    {
        return false;
    }
    read16(fp);
    if(read32(fp) != 0)
    {
      return false; 
     }
     return true;
}

void draw_bmp_picture(File fp) {
  uint16_t i,j,k,l,m=0;
  uint8_t bpm_data[PIXEL_NUMBER*3] = {0};
  uint16_t bpm_color[PIXEL_NUMBER];
  fp.seek(bmp_offset);
  for(i = 0;i < s_heigh;i++)
  {
    for(j = 0;j<s_width/PIXEL_NUMBER;j++)
    {
      m = 0;
      fp.read(bpm_data,PIXEL_NUMBER*3);
      for(k = 0;k<PIXEL_NUMBER;k++)
      {
        bpm_color[k]= my_lcd.Color_To_565(bpm_data[m+2], bpm_data[m+1], bpm_data[m+0]);
        m +=3;
      }
      for(l = 0;l<PIXEL_NUMBER;l++)
      {
        my_lcd.Set_Draw_color(bpm_color[l]);
        my_lcd.Draw_Pixel(j*PIXEL_NUMBER+l,i);
      }    
     }
   }    
}



void draw_rounded_rect(LCDWIKI_KBV * const lcd, uint16_t left, uint16_t top, uint16_t width, uint16_t height, uint16_t color) {
  uint16_t R = 20;
  lcd->Fill_Rect(left + R, top, width - 2*R, height, color);
  lcd->Fill_Rect(left, top + R, R, height - 2*R, color);
  lcd->Fill_Rect(left + width - R, top + R, R, height - 2*R, color);

  // Top left
  for (uint16_t x = left; x < left + R; ++x) {
    for (uint16_t y = top; y < top + R; ++y) {
      float fx = x;
      float fy = y;
      float dx = fx - (left + R);
      float dy = fy - (top + R);
      if (dx*dx + dy*dy < R*R) {
        lcd->Draw_Pixe(x, y, color);
      }
    }
  }

  // Top Right
  for (uint16_t x = left + width - R; x < left + width + R; ++x) {
    for (uint16_t y = top; y < top + R; ++y) {
      float fx = x;
      float fy = y;
      float dx = fx - (left + width - R);
      float dy = fy - (top + R);
      if (dx*dx + dy*dy < R*R) {
        lcd->Draw_Pixe(x, y, color);
      }
    }
  }

  // Bottom left
  for (uint16_t x = left; x < left + R; ++x) {
    for (uint16_t y = top + height - R; y < top + height; ++y) {
      float fx = x;
      float fy = y;
      float dx = fx - (left + R);
      float dy = fy - (top + height - R);
      if (dx*dx + dy*dy < R*R) {
        lcd->Draw_Pixe(x, y, color);
      }
    }
  }

  // Bottom Right
  for (uint16_t x = left + width - R; x < left + width + R; ++x) {
    for (uint16_t y = top + height - R; y < top + height; ++y) {
      float fx = x;
      float fy = y;
      float dx = fx - (left + width - R);
      float dy = fy - (top + height - R);
      if (dx*dx + dy*dy < R*R) {
        lcd->Draw_Pixe(x, y, color);
      }
    }
  }
}

LV_FONT_DECLARE(roboto16);

void draw_pixel(uint16_t x, uint16_t y, uint16_t c) {
  my_lcd.Draw_Pixe(x, y, c);
}

void draw_nopixel(uint16_t, uint16_t, uint16_t) {
}

bool is_word_break(uint32_t c) {
  return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\0');
}

struct Line {
  uint16_t start;
  uint16_t end;
};

struct WrappedDesc {
  uint16_t nlines;
  uint16_t line_width;
  Line lines[20];
};

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

WrappedDesc text_wrapping(uint32_t const * text, uint16_t max_width) {
  WrappedDesc res;
  memset(&res, 0, sizeof(res));
  
  uint16_t line_width = 0;
  uint16_t word_width = 0;
  uint16_t word_begin = 0;

  bool last = false;
  
  for (int ccount = 0; !last; ++ccount) {
    uint32_t const * const str = text + ccount;
    uint32_t c = *str;
    last = c == '\0';
    uint32_t n = last ? *(str + 1) : '\0';


    // Special case, handle first
    if (line_width == 0 && word_width == 0 && is_word_break(c)) {
        word_begin = ccount + 1;
        res.lines[res.nlines].start = ccount + 1;
        continue;
    }

    if (is_word_break(c)) {
      if (line_width + word_width < max_width) {
        res.lines[res.nlines].end = ccount;
        word_begin = ccount + 1;
        line_width += word_width;
        res.line_width = MAX(res.line_width, line_width);
        word_width = 0;
        continue;
      }
      
      // Go try the next line if there's already content on this one
      if (line_width > 0) {
        res.nlines += 1;
        res.line_width = max_width;
        line_width = 0;
        res.lines[res.nlines].start = word_begin;
  
        // Try to fit it again. If not, start breaking it in pieces
        if (line_width + word_width < max_width) {
          res.lines[res.nlines].end = ccount;
          word_begin = ccount + 1;
          line_width += word_width;
          res.line_width = MAX(res.line_width, line_width);
          word_width = 0;
          continue;
        }
      }

      // It is too large to fit on the current line: break it up
      int cindex = word_begin;
      while (cindex < ccount) {
        // Determine how many characters can be put into it
        uint16_t part_width = 0;
        uint16_t last_cwidth = 0;
        while (part_width < max_width && cindex < ccount) {
          last_cwidth = lv_draw_letter(draw_nopixel, 0, 0, &roboto16, text[cindex], text[cindex + 1], 0, 0);
          part_width += last_cwidth;
          cindex += 1;
        }

        // Inc'ed once too many
        if (cindex != ccount) {
          cindex -= 1;
        }

        res.lines[res.nlines].end = cindex;
        word_width -= (part_width - last_cwidth);
        word_begin = cindex;

        // Go fill the next line (if this one was filled)
        if (part_width >= max_width) {
          res.nlines += 1;
          res.line_width = max_width;
          res.lines[res.nlines].start = word_begin;
          line_width = 0;
        }
      }

      word_begin = ccount + 1;
      continue;
    }
    
    word_width += lv_draw_letter(draw_nopixel, 0, 0, &roboto16, c, n, 0, 0);
  }



  // Convert from index to amount
  res.nlines += 1;
  
  return res;
}

uint16_t draw_string(uint16_t left, uint16_t top, uint32_t const * str, uint16_t col, uint16_t bg) {
  while (*str != '\0') {
    uint32_t c = *str++;
    uint32_t n = *str;
    left += lv_draw_letter(draw_pixel, left, top, &roboto16, c, n, col, bg);
  }
  return left;
}

size_t read_string_into_n(uint32_t index, uint32_t * buf, size_t n) {
  File f = SD.open("messages");
  uint32_t const n_messages = read32(f);
  if (index >= n_messages && n > 0) {
    buf[0] = '\0';
    f.close();
    return;
  }

  uint32_t const DATA_START = 4 + index*4;
  f.seek(DATA_START);
  uint32_t const STR_START = read32(f);
  
  size_t written = 0;
  uint32_t readd = 0;
  
  f.seek(STR_START);
  
  while (written < n) {
    uint32_t c = 0;
    uint32_t const fb = f.read();
    if ((fb & 10000000) == 0) {
      c = fb;
    }
    if ((fb & 0b11100000) == 0b11000000) {
      uint32_t sb = f.read();
      
      c = ((fb & 0b00011111) << 6) | ((sb & 0b00111111));
    }
    if ((fb & 0b011110000) == 0b11100000) {
      uint32_t sb = f.read();
      uint32_t tb = f.read();
     
      c = ((fb & 0b00011111) << 12) | ((sb & 0b00111111) << 6) | ((tb & 0b00111111));
    }
    if ((fb & 0b11110000) == 0b11110000) {
      uint32_t b2 = f.read();
      uint32_t b3 = f.read();
      uint32_t b4 = f.read();
      c = ((fb & 0b00000111) << 18) | ((b2 & 0b00111111) << 12) | ((b3 & 0b00111111) << 6) | ((b4 & 0b00111111));
    }
    buf[written] = c;
    ++written;

    if (c == '\0') {
      break;
    }
  }
  buf[n - 1] = '\0';
  f.close();
  return written;
}

uint64_t fast_hash(uint64_t h) {
    h ^= h >> 23;
    h *= 0x2127599bf4325c37ULL;
    h ^= h >> 47;
    return h;
}

uint32_t true_random() {
  uint64_t h = 0xFFFFFFFFFF;
  for (int i = 0; i < 64; ++i) {
    h = fast_hash(h ^ analogRead(A0) );
    delay(1);
  }
  return h;
}


// GLOBALS
uint32_t index_shown = 0;
uint64_t change_period_ms = 5*1000;

enum MessageType {
  STRING,
  IMAGE
}

void next_message() {
    uint32_t candidate;
    while ((candidate = random(0, LENGTH(LOVED_INDICES))) == index_shown);
    Serial.print("Chosen index: "); Serial.println(candidate);
    index_shown = candidate;
    //index_shown = 215; é and emoji

    uint32_t buf[130];
    read_string_into_n(index_shown, buf, LENGTH(buf));

    // Preparations for drawing
    WrappedDesc wrapping = text_wrapping(buf, 150);

    uint16_t const cx = 480 / 2, cy = 320 / 2;
    const uint16_t SPACING = 2;
    uint16_t text_height = roboto16.line_height * wrapping.nlines + SPACING*(wrapping.nlines - 1);

    uint16_t bw = wrapping.line_width + 40, bh = text_height + 40;

    // Clear   
    my_lcd.Fill_Screen(BLUE);

    // Draw box
    draw_rounded_rect(&my_lcd, cx - bw/2, cy - bh / 2, bw, bh, RED);

    // Draw the text
    uint16_t top = cy - text_height/2;
    uint16_t left = cx - wrapping.line_width / 2;
    for (int i = 0; i < wrapping.nlines; ++i) {
      uint32_t const prev = buf[wrapping.lines[i].end];
      buf[wrapping.lines[i].end] = '\0';
      draw_string(left, top, &buf[wrapping.lines[i].start], YELLOW, RED);
      buf[wrapping.lines[i].end] = prev;
      top += roboto16.line_height + SPACING;
    }
}

void setup() {
    pinMode(A0, INPUT);
    Serial.begin(9600);
    
    my_lcd.Init_LCD();
    my_lcd.Set_Rotation(3);

    uint32_t const SEED = true_random();
    randomSeed(SEED);

    next_message();
    uint64_t last_change_at = millis();
    do {
      uint64_t const now = millis();
      if (now - last_change_at > change_period_ms) {
        next_message();
        last_change_at = now;
      }
      
    } while (true);
   
/*
    //Init SD_Card
    pinMode(53, OUTPUT);
    if (!SD.begin(53)) {
      my_lcd.Set_Text_Back_colour(BLUE);
      my_lcd.Set_Text_colour(WHITE);    
      my_lcd.Set_Text_Size(1);
      my_lcd.Print_String("SD Card Init fail!",0,0);
    }
    */
}

void loop() {
  /*
    int i = 0;
    File bmp_file;
    for(i = 0;i<FILE_NUMBER;i++)
    {
       bmp_file = SD.open(file_name[i]);
       if(!bmp_file)
       {
            my_lcd.Set_Text_Back_colour(BLUE);
            my_lcd.Set_Text_colour(WHITE);    
            my_lcd.Set_Text_Size(1);
            my_lcd.Print_String("didnt find BMPimage!",0,10);
            while(1);
        }
        if(!analysis_bpm_header(bmp_file))
        {  
            my_lcd.Set_Text_Back_colour(BLUE);
            my_lcd.Set_Text_colour(WHITE);    
            my_lcd.Set_Text_Size(1);
            my_lcd.Print_String("bad bmp picture!",0,0);
            return;
        }
        draw_bmp_picture(bmp_file);
        bmp_file.close(); 
        delay(5000);
     }
     */
}
