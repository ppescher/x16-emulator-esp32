#include <Arduino.h>
#include <SPI.h>
#include "MD5Builder.h"
#include "FS.h"
#include "SD_MMC.h"

#include "EVE.h"

#include "rom/cache.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

extern "C" int
main(int argc, const char *argv[]);

extern "C"
void *emulator_loop(void *param);

extern "C" void vga_init()
{
  EVE_cmd_dl(CMD_DLSTART);
  EVE_cmd_dl(DL_CLEAR_COLOR_RGB | 0x000000); /* set the default clear color to black */
  EVE_cmd_dl(DL_CLEAR | CLR_COL);            /* clear the screen - this and the previous prevent artifacts between lists, attributes are the color, stencil and tag buffers */

  constexpr uint32_t BPP = 1; // bytes per pixel
  EVE_cmd_dl(BITMAP_HANDLE(1));
  EVE_cmd_dl(BITMAP_SOURCE(0x1000));
  EVE_cmd_dl(BITMAP_LAYOUT_H(SCREEN_WIDTH * BPP, SCREEN_HEIGHT));
  EVE_cmd_dl(BITMAP_LAYOUT(EVE_PALETTED565, SCREEN_WIDTH * BPP, SCREEN_HEIGHT));
  EVE_cmd_dl(BITMAP_TRANSFORM_E(128));  // double height
  EVE_cmd_dl(BITMAP_SIZE_H(SCREEN_WIDTH, SCREEN_HEIGHT * 2));
  EVE_cmd_dl(BITMAP_SIZE(EVE_NEAREST, EVE_BORDER, EVE_BORDER, SCREEN_WIDTH, SCREEN_HEIGHT * 2));
  EVE_cmd_dl(PALETTE_SOURCE(0x0));
  EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
  EVE_cmd_dl(VERTEX2II(0, 0 * BPP, 1, 0));
  EVE_cmd_dl(DL_END);
  EVE_cmd_dl(DL_DISPLAY); /* mark the end of the display-list */
  EVE_cmd_dl(CMD_SWAP);   /* make this list active */
  EVE_execute_cmd();      /* wait for EVE to be no longer busy */

  EVE_switch_SPI(true);
}

constexpr uint32_t pixel_offset(int x, int y) {
  return (y * SCREEN_WIDTH + x) * 1;
}

extern "C" void vga_display(void* framebuffer, void* palette)
{
  long now = millis();

  EVE_memWrite_sram_buffer(0x0, (uint8_t *)palette, 2*256);

  constexpr int ROWS = 32;
  for (int y=0; y<SCREEN_HEIGHT; y+=ROWS)
  {
    //esp_cache_msync(&panel->fbs[panel->bb_fb_index][panel->bounce_pos_px * bytes_per_pixel], (size_t)panel->bb_size, ESP_CACHE_MSYNC_FLAG_INVALIDATE);
    Cache_WriteBack_Addr((uint32_t)framebuffer + pixel_offset(0, y), pixel_offset(0, ROWS));
    EVE_memWrite_sram_buffer(0x1000 + pixel_offset(0, y), (uint8_t *)framebuffer + pixel_offset(0, y), pixel_offset(0, ROWS));
  }

  now = millis() - now;
  printf("Blit VGA = %li ms\n", now);
}

void setup() {

  pinMode(EVE_CS, OUTPUT);
  digitalWrite(EVE_CS, HIGH);
  //pinMode(EVE_PDN, OUTPUT);
  //digitalWrite(EVE_PDN, LOW);
  Serial.begin(115200);
  delay(1000);
  pinMode(0, OUTPUT);

  EVE_init_spi();
  EVE_switch_SPI(false);

  /*while (true)
  {
    uint8_t pattern[4]={0xF0,0xAA,0x0F,0x55};
    EVE_memWrite_sram_buffer(0,pattern,4);
    delay(100);
  };*/

  if (E_OK == EVE_init()) /* make sure the init finished correctly */
  {
    delay(1000);
    EVE_cmd_dl(CMD_DLSTART);                            /* instruct the co-processor to start a new display list */
    EVE_cmd_dl(DL_CLEAR_COLOR_RGB | 0xFFFFFF);          /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, attributes are the color, stencil and tag buffers */
    EVE_color_rgb(0x000000);                            /* set the color to black */
    EVE_cmd_text(EVE_HSIZE / 2, EVE_VSIZE / 2, 30, EVE_OPT_CENTER, "Hello, World!");
    EVE_cmd_dl(DL_DISPLAY); /* mark the end of the display-list */
    EVE_cmd_dl(CMD_SWAP);   /* make this list active */
    EVE_execute_cmd();      /* wait for EVE to be no longer busy */
    delay(1000);
    EVE_cmd_dl(CMD_TESTCARD);
    delay(1000);
    EVE_execute_cmd(); /* wait for EVE to be no longer busy */

    EVE_memWrite8(REG_VOL_SOUND, 255U);    /* turn synthesizer volume down, reset-default is 0xff */
    EVE_memWrite16(REG_SOUND, EVE_UNMUTE); /* set synthesizer to mute */
    EVE_memWrite8(REG_PLAY, 1);
    delay(1000);

    SD_MMC.setPins(45, 21, 38, 39, 40, 41);
    if (!SD_MMC.begin()) {
      Serial.println("Card Mount Failed");
    } else {
      uint8_t cardType = SD_MMC.cardType();

      if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        return;
      }

      Serial.print("SD_MMC Card Type: ");
      if (cardType == CARD_MMC) {
        Serial.println("MMC");
      } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
      } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
      } else {
        Serial.println("UNKNOWN");
      }

      uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
      Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
      return;
    }
  }
  for (;;) {
    Serial.println("Initialization failed");
    delay(1000);
  }
}

void vga_loop() {
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
  delay(500);

  uint32_t bg = (rand() & 255) + (rand() & 255) * 256 + (rand() & 255) * 65536;
  uint32_t fg = bg ^ 0xA5A5A5;
  int x = rand() % EVE_HSIZE / 2;
  int y = rand() % EVE_VSIZE / 2;

  EVE_cmd_dl(CMD_DLSTART); /* instruct the co-processor to start a new display list */
  //EVE_cmd_dl(BITMAP_TRANSFORM_E(128));
  EVE_cmd_dl(DL_CLEAR_COLOR_RGB | bg);                /* set the default clear color to white */
  EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, attributes are the color, stencil and tag buffers */
  EVE_color_rgb(fg);                                  /* set the color to black */
  EVE_cmd_text(EVE_HSIZE / 4 + x, EVE_VSIZE / 4 + y, 30, EVE_OPT_CENTER, "Hello, World!");
  EVE_cmd_dl(DL_DISPLAY); /* mark the end of the display-list */
  EVE_cmd_dl(CMD_SWAP);   /* make this list active */

  uint8_t note = EVE_MIDI_A0 + (rand() % (EVE_MIDI_C8 - EVE_MIDI_A0));
  uint8_t inst = EVE_SQUAREWAVE + (rand() % (EVE_CHACK - EVE_SQUAREWAVE));

  EVE_memWrite16(REG_SOUND, (note << 8) | inst);
  EVE_memWrite8(REG_PLAY, 1);
}

#include <SDL.h>

enum { KS_IDLE, KS_ESCAPE, KS_BRACKET } ks = KS_IDLE;

struct Key {
  bool d;
  int s, k;
  Key() {}
  Key(int sc, int ky, bool dn) : d(dn), s(sc), k(ky) {}
};

Key keybuf[16];
size_t keynum = 0, keyidx = 0;

#define KEY_DN(s) keybuf[keynum++] = Key{s,c,true}
#define KEY_UP(s) keybuf[keynum++] = Key{s,c,false}
#define KEY(s) KEY_DN(s),KEY_UP(s)
#define SHIFT_KEY(s) KEY_DN(SDL_SCANCODE_LSHIFT),KEY(s),KEY_UP(SDL_SCANCODE_LSHIFT)

extern "C" int
vga_poll_event(SDL_Event * event)
{
  switch (ks)
  {
  case KS_IDLE:
    if (keynum == 0)
    {
      int c = Serial.read();
      if (c == '\033') //Escape
        ks = KS_ESCAPE;
      else if (c >= 'a' && c <= 'z')
        KEY(SDL_SCANCODE_A + c - 'a');
      else if (c >= 'A' && c <= 'Z')
        KEY(SDL_SCANCODE_A + c - 'A');
      else if (c >= '1' && c <= '9')
        KEY(SDL_SCANCODE_1 + c - '1');
      else if (c == '0')
        KEY(SDL_SCANCODE_0);
      else if (c == ',')
        KEY(SDL_SCANCODE_COMMA);
      else if (c == '<')
        SHIFT_KEY(SDL_SCANCODE_COMMA);
      else if (c == '.')
        KEY(SDL_SCANCODE_PERIOD);
      else if (c == '>')
        SHIFT_KEY(SDL_SCANCODE_PERIOD);
      else if (c == '\r')
        KEY(SDL_SCANCODE_RETURN);
      else if (c == '\b')
        KEY(SDL_SCANCODE_BACKSPACE);
      else if (c == '\t')
        KEY(SDL_SCANCODE_TAB);
      else if (c == ' ')
        KEY(SDL_SCANCODE_SPACE);
      else if (c == ';')
        KEY(SDL_SCANCODE_SEMICOLON);
      else if (c == ':')
        SHIFT_KEY(SDL_SCANCODE_SEMICOLON);
      else if (c == '+')
        KEY(SDL_SCANCODE_KP_PLUS);
      else if (c == '*')
        KEY(SDL_SCANCODE_KP_MULTIPLY);
      else if (c == '/')
        KEY(SDL_SCANCODE_SLASH);
      else if (c == '-')
        KEY(SDL_SCANCODE_MINUS);
      else if (c == '\'')
        KEY(SDL_SCANCODE_APOSTROPHE);
      else if (c == '\"')
        SHIFT_KEY(SDL_SCANCODE_APOSTROPHE);
      else if (c == '=')
        KEY(SDL_SCANCODE_EQUALS);
      else if (c == '[')
        KEY(SDL_SCANCODE_LEFTBRACKET);
      else if (c == ']')
        KEY(SDL_SCANCODE_RIGHTBRACKET);
      else if (c == '(')
        SHIFT_KEY(SDL_SCANCODE_9);
      else if (c == ')')
        SHIFT_KEY(SDL_SCANCODE_0);
    }
    if (keynum > 0 && keyidx < keynum)
    {
      event->key.keysym.sym = SDL_Keycode(keybuf[keyidx].k);
      event->key.keysym.scancode = SDL_Scancode(keybuf[keyidx].s);
      event->type = keybuf[keyidx].d ? SDL_KEYDOWN : SDL_KEYUP;
      keyidx++;
      return true;
    }
    else
    {
      keyidx = 0;
      keynum = 0;
    }
    break;

  case KS_ESCAPE:
    {
      int c = Serial.read();
      if (c == '[')
        ks = KS_BRACKET;
      else
        ks = KS_IDLE;
    }
    break;

  case KS_BRACKET:
    {
      int c = Serial.read();
      if (c == 'H')
        KEY(SDL_SCANCODE_HOME);
      else if (c == 'A')
        KEY(SDL_SCANCODE_UP);
      else if (c == 'B')
        KEY(SDL_SCANCODE_DOWN);
      else if (c == 'C')
        KEY(SDL_SCANCODE_RIGHT);
      else if (c == 'D')
        KEY(SDL_SCANCODE_LEFT);
    }
    ks = KS_IDLE;
    break;
  }
  return false;
}

void loop()
{
  SDL_setenv("SDL_VIDEODRIVER","dummy",1);
  SDL_setenv("SDL_AUDIODRIVER","dummy",1);
  const char* argv[] = { "x16emu", "-mhz", "1", /*"-log", "KS"*/ };
  main(sizeof(argv)/sizeof(*argv), argv);

  extern uint8_t* ROM;
  MD5Builder passmd5;
  passmd5.begin();
  passmd5.add(ROM, 212992u);
  passmd5.calculate();
  Serial.print("ROM MD5 = ");
  Serial.println(passmd5.toString());

  for (;;)
  {
    digitalWrite(0, HIGH);
    emulator_loop(NULL);
    digitalWrite(0, LOW);
    emulator_loop(NULL);
  }
}
