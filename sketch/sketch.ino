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
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "src/LCDWIKI_GUI.h" //Core graphics library
#include "src/LCDWIKI_KBV.h" //Hardware-specific library
#include "src/lvgl/lvgl.h"

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

// GLOBALS
uint32_t index_shown = 0;
#define COLOUR_BG (WHITE)
#define COLOUR_TEXT (WHITE)
#define COLOUR_DIALOG (my_lcd.Color_To_565(0x00, 0x7f, 0xf9))
#define COLOUR_NAME (BLACK)
uint16_t s_width;  
uint16_t s_height;
LV_FONT_DECLARE(roboto16)


enum class Person {
  HIM,
  HER
};


static uint32_t const NAME_HIM[] = {0x43c, 0x430, 0x43d, 0x435, 0x447, 0x43a, 0x430, '\0'};
static uint32_t const NAME_HER[] = {0x440, 0x438, 0x442, 0x43e, 0x447, 0x43a, 0x430, '\0'};
static uint32_t const * const NAMES[] = {NAME_HIM, NAME_HER};

volatile Person person = Person::HIM;
volatile uint64_t person_button_changed;
int person_button_previous_value = HIGH;



void person_button_interrupt() {
  uint64_t const now = millis();
  person_button_changed = now;
}

Person get_person() {
  int const button_state = digitalRead(2);

  uint64_t const now = millis();
  if (now - person_button_changed > 100) {
    if (person_button_previous_value == HIGH && button_state == LOW) {
      person = static_cast<Person>(!static_cast<bool>(person));
    }

    person_button_previous_value = button_state;
  }

  return person;
}

uint16_t read16le(File fp) {
    uint16_t const low = fp.read();
    uint16_t const high = fp.read();
    return (high<<8)|low;
}
uint32_t read32le(File fp) {
    uint32_t const low = read16le(fp);
    uint32_t const high = read16le(fp);
    return (high << 16) | low;   
}

uint16_t read16(File fp) {
    uint16_t const high = fp.read();
    uint16_t const low = fp.read();
    return (high<<8)|low;
}

uint32_t read32(File fp) {
    uint32_t const high = read16(fp);
    uint32_t const low = read16(fp);
    return (high << 16) | low;   
}

struct BmpHeader {
  uint32_t width;
  int32_t height;
  uint32_t offset;
  uint8_t bpp;
  bool valid;
};
 
struct BmpHeader analysis_bpm_header(File fp) {
    struct BmpHeader header;
    memset(&header, 0, sizeof(header));
    
    if(read16le(fp) != 0x4D42) {
      Serial.println("Magic bytes don't match.");
      return header;  
    }
    //get bpm size
    read32le(fp);
    //get creator information
    read32le(fp);
    //get offset information
    header.offset = read32le(fp);
    //get DIB infomation
    uint32_t const dib_size = read32le(fp);
    if (dib_size != 124) {
      Serial.print(fp.name()); Serial.println(" does not use BITMAPV5HEADER DIB header.");
      return header;
    }
    //get width and heigh information
    header.width  = read32le(fp);
    uint32_t const uheight = read32le(fp);
    memcpy(&header.height, &uheight, sizeof(uheight));
    
    read16le(fp);
    header.bpp = read16le(fp);
    if (!( header.bpp == 24 || header.bpp == 32 )) {
      Serial.print(fp.name()); Serial.println(" does not have RGB encoding.");
      return header;
    }
    // Compression type should be None
    uint32_t const compression = read32le(fp);
    if(compression != 0) {
      Serial.print(fp.name()); Serial.print(" use compression="); Serial.println(compression);
      return header; 
    }
    header.valid = true;
    return header;
}

#define BMP_PIXEL_BUF_AMOUNT (32)

static uint8_t  data[BMP_PIXEL_BUF_AMOUNT*4] = {0};
static uint16_t colors[BMP_PIXEL_BUF_AMOUNT];

void draw_bmp_picture(BmpHeader header, File fp) {
  uint16_t const LEFT_END = s_width/2 + (header.width / 2) + 2*(header.width & 0x1);
  uint16_t const LEFT_BEGIN = s_width/2 - header.width / 2;
  uint16_t left = LEFT_BEGIN;
  uint16_t top  = (s_height + header.height - 1) / 2;

  uint32_t pixels_left = header.width * header.height;
  uint8_t const Bpp = header.bpp / 8;
  
  fp.seek(header.offset);
  while (pixels_left > 0) {
    size_t m = 0;
    fp.read(data, BMP_PIXEL_BUF_AMOUNT*Bpp);
    for (size_t k = 0; k < BMP_PIXEL_BUF_AMOUNT; k++) {
      colors[k] = my_lcd.Color_To_565(data[m+2], data[m+1], data[m+0]);
      m += Bpp;
    }
    for (size_t l = 0; l < BMP_PIXEL_BUF_AMOUNT && pixels_left; l++) {
      my_lcd.Draw_Pixe(left, top, colors[l]);
      pixels_left -= 1;
      left += 1;
      if (left == LEFT_END) {
        left = LEFT_BEGIN;
        top -= 1;
      }
    }
  }  
}



void draw_rounded_rect(LCDWIKI_KBV * const lcd, uint16_t left, uint16_t top, uint16_t width, uint16_t height, uint16_t R, uint16_t color) {
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

struct WrappedDesc text_wrapping(uint32_t const * text, uint16_t max_width) {
  struct WrappedDesc res;
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

    uint16_t const char_width = lv_draw_letter(draw_nopixel, 0, 0, &roboto16, c, n, 0, 0);
    word_width += char_width;
    
    if (is_word_break(c)) {
      // Don't take whitespace width into account for wrapping purposes
      if (line_width + word_width - char_width < max_width) {
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
        res.line_width = MAX(res.line_width, line_width);
        line_width = 0;
        res.lines[res.nlines].start = word_begin;
  
        // Try to fit it again. If not, start breaking it in pieces
        if (line_width + word_width - char_width < max_width) {
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
          res.line_width = MAX(res.line_width, part_width);
          res.lines[res.nlines].start = word_begin;
          line_width = 0;
        }
      }

      word_begin = ccount + 1;
      continue;
    }
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

size_t read_string_into_n(File * const f, uint32_t * buf, size_t n) {
  size_t written = 0;
  
  while (written < n) {
    uint32_t c = 0;
    uint32_t const fb = f->read();
    if ((fb & 10000000) == 0) {
      c = fb;
    }
    if ((fb & 0b11100000) == 0b11000000) {
      uint32_t sb = f->read();
      
      c = ((fb & 0b00011111) << 6) | ((sb & 0b00111111));
    }
    if ((fb & 0b011110000) == 0b11100000) {
      uint32_t sb = f->read();
      uint32_t tb = f->read();
     
      c = ((fb & 0b00011111) << 12) | ((sb & 0b00111111) << 6) | ((tb & 0b00111111));
    }
    if ((fb & 0b11110000) == 0b11110000) {
      uint32_t b2 = f->read();
      uint32_t b3 = f->read();
      uint32_t b4 = f->read();
      c = ((fb & 0b00000111) << 18) | ((b2 & 0b00111111) << 12) | ((b3 & 0b00111111) << 6) | ((b4 & 0b00111111));
    }
    buf[written] = c;
    ++written;

    if (c == '\0') {
      break;
    }
  }
  buf[n - 1] = '\0';
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
    h = fast_hash(h ^ analogRead(A1) );
    delay(1);
  }
  return h;
}

enum MessageType {
  STRING = 0,
  IMAGE = 1
};

void next_message() {
    char const * const mfile = (person == Person::HIM) ? "/him" : "/her";
    File f = SD.open(mfile, FILE_READ);
    if (!f) {
      Serial.println("Unable to open messages file.");
      return;
    }
    uint32_t const n_messages = read32(f);
    uint32_t const msg_start = 4 + 4*n_messages; // (Last offset points to bmp data area)
   
    uint32_t candidate;
    while ((candidate = random(0, n_messages)) == index_shown);
    Serial.print("Chosen index: "); Serial.println(candidate);
    index_shown = candidate;
    
    uint32_t const offset_addr = 4 + index_shown*4;
    f.seek(offset_addr);
    uint32_t const msg_offset = read32(f);

    f.seek(msg_start + msg_offset);
    uint8_t const msg_type = f.read();

    if (msg_type == MessageType::STRING) {
      uint32_t buf[130];
      read_string_into_n(&f, buf, LENGTH(buf));     
      f.close(); 

      
      uint16_t const R = 20;
      uint16_t const W = 150;
      // Preparations for drawing
      WrappedDesc wrapping = text_wrapping(buf, W);

      uint16_t const cx = 480 / 2, cy = 320 / 2;
      const uint16_t SPACING = 2;
      uint16_t text_height = roboto16.line_height * wrapping.nlines + SPACING*(wrapping.nlines - 1);
  
      uint16_t bw = wrapping.line_width + 2*R, bh = text_height + 2*R;
  
      // Clear   
      my_lcd.Fill_Screen(COLOUR_BG);

      // Calculate box dimensions
      uint16_t bleft = cx - bw/2;
      uint16_t btop  = cy - bh / 2;
      
      // Draw Writer name
      int16_t wtop = btop - roboto16.line_height;
      if (wtop >= 0) {
        draw_string(bleft + 10, wtop, NAMES[static_cast<int>(person)], COLOUR_NAME, COLOUR_BG);
      }
  
      // Draw box
      draw_rounded_rect(&my_lcd, bleft, btop, bw, bh, R, COLOUR_DIALOG);
  
      // Draw the text
      uint16_t top = cy - text_height/2;
      uint16_t left = cx - wrapping.line_width / 2;
      for (int i = 0; i < wrapping.nlines; ++i) {
        uint32_t const prev = buf[wrapping.lines[i].end];
        buf[wrapping.lines[i].end] = '\0';
        draw_string(left, top, &buf[wrapping.lines[i].start], COLOUR_TEXT, COLOUR_DIALOG);
        buf[wrapping.lines[i].end] = prev;
        top += roboto16.line_height + SPACING;
      }
    } else {  
      my_lcd.Fill_Screen(COLOUR_BG);

      //8.3 filenames
      char fname[21] = "/images/";
      f.read(&fname[8], 12);
      fname[8+12] = '\0';
      f.close();

      File imgf = SD.open(fname, FILE_READ);
      if (!imgf) {
        Serial.print("Could not read file ("); Serial.print(index_shown); Serial.print("): "); Serial.println(fname);
        imgf.close();
        return;
      }

      BmpHeader const header = analysis_bpm_header(imgf);
      if (!header.valid) {
        Serial.print("Not a valid bmp ("); Serial.print(index_shown); Serial.print("): "); Serial.println(fname);
        imgf.close();
        return;
      }
      Serial.print("Valid bmp header in  "); Serial.println(fname);

      draw_bmp_picture(header, imgf);
      
    }

}

void setup() {
    // POT
    pinMode(A0, INPUT_PULLUP);
    analogReference(INTERNAL1V1);

    // Person Button
    pinMode(2, INPUT_PULLUP);
    person_button_changed = millis();
    attachInterrupt(digitalPinToInterrupt(2), &person_button_interrupt, CHANGE);
    
    Serial.begin(9600);
    
    my_lcd.Init_LCD();
    my_lcd.Set_Rotation(3);
    s_width = my_lcd.Get_Display_Width();
    s_height = my_lcd.Get_Display_Height();

    uint32_t const SEED = true_random();
    randomSeed(SEED);

    //Init SD_Card
    pinMode(53, OUTPUT);
    if (!SD.begin(53)) {
      Serial.println("Unable to begin SD!");
      my_lcd.Fill_Screen(RED);
      while (true);
    }

    next_message();
}

float pot() {
  uint16_t const a = analogRead(A0);
  return a / 1024.f;
}

void loop() {
    static uint64_t last_change_at = millis();

    float const p = pot();
    uint64_t change_period_ms = (59*p + 1) * 1000;
    if (p > 0.95) {
      change_period_ms = UINT64_MAX;
    }

    static Person last_person = person;
    Person person = get_person();
    
    uint64_t const now = millis();
    if (now - last_change_at > change_period_ms || last_person != person) {
      next_message();
      
      last_change_at = now;
    }

    last_person = person;
}
