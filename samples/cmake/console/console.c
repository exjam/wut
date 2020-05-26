#include "console.h"
#include "console_colour.h"
#include "console_font.h"

#include <coreinit/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WHBConsoleState WHBConsoleState;
typedef struct WHBConsoleCharacter WHBConsoleCharacter;

enum WHBConsoleGraphicsFlags
{
   WHB_CONSOLE_GRAPHICS_BOLD           = 1 << 0,
   WHB_CONSOLE_GRAPHICS_FAINT          = 1 << 1,
   WHB_CONSOLE_GRAPHICS_ITALIC         = 1 << 2,
   WHB_CONSOLE_GRAPHICS_UNDERLINE      = 1 << 3,
   WHB_CONSOLE_GRAPHICS_BLINK_SLOW     = 1 << 4,
   WHB_CONSOLE_GRAPHICS_BLINK_FAST     = 1 << 5,
   WHB_CONSOLE_GRAPHICS_REVERSE_VIDEO  = 1 << 6,
   WHB_CONSOLE_GRAPHICS_CONCEAL        = 1 << 7,
   WHB_CONSOLE_GRAPHICS_CROSSED_OUT    = 1 << 8,
};

enum WHBConsoleCharacterFlags
{
   // Lower bits: WHBConsoleGraphicsFlags
   WHB_CONSOLE_CHARACTER_FG_COLOUR_INDEX  = 1 << 30,
   WHB_CONSOLE_CHARACTER_BG_COLOUR_INDEX  = 1 << 31,
};

struct WHBConsoleCharacter
{
   int chr;
   uint32_t foregroundColour;
   uint32_t backgroundColour;
   uint32_t flags;
};

struct WHBConsoleState
{
   //! Character buffer.
   WHBConsoleCharacter *buffer;

   //! Scrollback buffer
   WHBConsoleCharacter *scrollbackBuffer;

   //! Console width in characters.
   int width;

   //! Console height in characters.
   int height;

   //! Scrollback length in lines.
   int scrollbackLines;

   //! Current cursor x position, starts from 1.
   int cursorX;

   //! Current cursor y position, starts from 1.
   int cursorY;

   //! Tab width in characters.
   int tabWidth;

   //! Current foreground colour.
   uint32_t foregroundColour;

   //! Current background colour.
   uint32_t backgroundColour;

   //! Current foreground colour index.
   int foregroundColourIndex;

   //! Current background colour index.
   int backgroundColourIndex;

   //! Current graphics flags.
   int graphicsFlags;

   //! Buffer for current escape code.
   char escapeBuffer[32];

   //! Current length of the escapeBuffer.
   int escapeBufferLength;

   //! Current parse position for parseNextEscapeParameter.
   int escapeParsePosition;

   //! Time between fast blinks.
   OSTime fastBlinkInterval;

   //! Time between slow blinks.
   OSTime slowBlinkInterval;
};

// TODO: Scrollback buffer
uint32_t
WHBConsoleCalculateWorkMemorySize(int width, int height, int scrollbackLines)
{
   return sizeof(WHBConsoleState) +
          sizeof(WHBConsoleCharacter) * width * (height + scrollbackLines);
}

static WHBConsoleState *
getConsoleState(WHBConsole *console)
{
   return (WHBConsoleState *)console;
}

static WHBConsoleCharacter *
getConsoleCharacter(WHBConsoleState *state, int x, int y)
{
   return state->buffer + (x - 1) + (y - 1) * state->width;
}

WHBConsole *
WHBConsoleInit(void *workMemory,
               uint32_t workMemorySize,
               int width,
               int height,
               int scrollbackLines)
{
   if (workMemorySize < WHBConsoleCalculateWorkMemorySize(width, height, scrollbackLines)) {
      return NULL;
   }

   WHBConsole *console = (WHBConsole *)workMemory;
   WHBConsoleState *state = (WHBConsoleState *)workMemory;
   memset(state, 0, sizeof(WHBConsoleState));

   // Setup console buffer
   state->buffer = (WHBConsoleCharacter *)(state + 1);
   state->width = width;
   state->height = height;
   state->scrollbackBuffer = state->buffer + width * height;
   state->scrollbackLines = scrollbackLines;

   // Setup default parameters
   WHBConsoleSetTabWidth(console, 4);
   WHBConsoleSetSlowBlinkInterval(console, OSMillisecondsToTicks(1200));
   WHBConsoleSetFastBlinkInterval(console, OSMillisecondsToTicks(100));
   WHBConsoleGraphicsSetForegroundColourDefault(console);
   WHBConsoleGraphicsSetBackgroundColourDefault(console);
   WHBConsoleClear(console);
   return console;
}

void
WHBConsoleClear(WHBConsole *console)
{
   WHBConsoleState *state = getConsoleState(console);
   memset(state->buffer, 0, sizeof(WHBConsoleCharacter) * state->width * state->height);
   memset(state->scrollbackBuffer, 0, sizeof(WHBConsoleCharacter) * state->width * state->scrollbackLines);

   state->cursorX = 1;
   state->cursorY = 1;
   state->escapeBufferLength = 0;

   for (int y = 1; y <= state->height; ++y) {
      for (int x = 1; x <= state->width; ++x) {
         WHBConsoleCharacter *character = getConsoleCharacter(state, x, y);

         if (state->backgroundColourIndex == -1) {
            character->backgroundColour = state->backgroundColour;
         } else {
            character->flags |= WHB_CONSOLE_CHARACTER_BG_COLOUR_INDEX;
            character->backgroundColour = state->backgroundColourIndex;
         }
      }
   }
}

void
WHBConsoleSetChar(WHBConsole *console, int x, int y, int chr)
{
   WHBConsoleState *state = getConsoleState(console);
   WHBConsoleCharacter *character = getConsoleCharacter(state, x, y);
   character->chr = chr;
   character->flags = state->graphicsFlags;

   if (state->foregroundColourIndex == -1) {
      character->foregroundColour = state->foregroundColour;
   } else {
      character->flags |= WHB_CONSOLE_CHARACTER_FG_COLOUR_INDEX;
      character->foregroundColour = state->foregroundColourIndex;
   }

   if (state->backgroundColourIndex == -1) {
      character->backgroundColour = state->backgroundColour;
   } else {
      character->flags |= WHB_CONSOLE_CHARACTER_BG_COLOUR_INDEX;
      character->backgroundColour = state->backgroundColourIndex;
   }
}

void
WHBConsoleSetTabWidth(WHBConsole *console, int width)
{
   WHBConsoleState *state = getConsoleState(console);
   state->tabWidth = width;
}

void
WHBConsoleSetFastBlinkInterval(WHBConsole *console, OSTime interval)
{
   WHBConsoleState *state = getConsoleState(console);
   state->fastBlinkInterval = interval;
}

void
WHBConsoleSetSlowBlinkInterval(WHBConsole *console, OSTime interval)
{
   WHBConsoleState *state = getConsoleState(console);
   state->slowBlinkInterval = interval;
}

void
WHBConsoleCursorUp(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorY -= n;
   if (state->cursorY < 1) {
      state->cursorY = 1;
   }
}

void
WHBConsoleCursorDown(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorY += n;
   if (state->cursorY > state->height) {
      state->cursorY = state->height;
   }
}

void
WHBConsoleCursorBack(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX -= n;
   if (state->cursorX < 1) {
      state->cursorX = 1;
   }
}

void
WHBConsoleCursorForward(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX += n;
   if (state->cursorX > state->width) {
      state->cursorX = state->width;
   }
}

void
WHBConsoleCursorNextLine(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX = 1;
   state->cursorY += n;
   if (state->cursorY > state->height) {
      state->cursorY = state->height;
   }
}

void
WHBConsoleCursorPreviousLine(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX = 1;
   state->cursorY -= n;
   if (state->cursorY < 1) {
      state->cursorY = 1;
   }
}

void
WHBConsoleCursorHorizontalAbsolute(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX = n;

   if (state->cursorX < 1) {
      state->cursorX = 1;
   }

   if (state->cursorX > state->width) {
      state->cursorX = state->width;
   }
}

void
WHBConsoleCursorPosition(WHBConsole *console, int x, int y)
{
   WHBConsoleState *state = getConsoleState(console);
   state->cursorX = x;
   state->cursorY = y;

   if (state->cursorX < 1) {
      state->cursorX = 1;
   }

   if (state->cursorX > state->width) {
      state->cursorX = state->width;
   }

   if (state->cursorY < 1) {
      state->cursorY = 1;
   }

   if (state->cursorY > state->height) {
      state->cursorY = state->height;
   }
}

void
WHBConsoleCursorForwardTabulation(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   int toNextTab =
      state->tabWidth - ((state->cursorX - 1) % state->tabWidth);
   WHBConsoleCursorForward(console, n * (state->tabWidth - 1) + toNextTab);
}

void
WHBConsoleEraseInDisplay(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   if (n == 0) { // Clear from cursor to end of screen
      // Clear the rest of the current line
      for (int x = state->cursorX; x <= state->width; ++x) {
         WHBConsoleSetChar(console, x, state->cursorY, ' ');
      }

      // Clear the subsequent lines to end of screen
      for (int y = state->cursorY + 1; y <= state->height; ++y) {
         for (int x = 1; x <= state->width; ++x) {
            WHBConsoleSetChar(console, x, y, ' ');
         }
      }
   } else if (n == 1) { // Clear from cursor to beginning of screen
      // Clear the start of current line
      for (int x = 1; x <= state->cursorX; ++x) {
         WHBConsoleSetChar(console, x, state->cursorY, ' ');
      }

      // Clear lines from start of screen up to current line
      for (int y = 1; y < state->cursorY; ++y) {
         for (int x = 1; x <= state->width; ++x) {
            WHBConsoleSetChar(console, x, y, ' ');
         }
      }
   } else if (n == 2) { // Clear entire screen
      for (int y = 1; y <= state->height; ++y) {
         for (int x = 1; x <= state->width; ++x) {
            WHBConsoleSetChar(console, x, y, ' ');
         }
      }
   }
}

void
WHBConsoleEraseInLine(WHBConsole *console, int n)
{
   WHBConsoleState *state = getConsoleState(console);
   if (n == 0) {
      // Clear from cursor to end of line
      for (int x = state->cursorX; x <= state->width; ++x) {
         WHBConsoleSetChar(console, x, state->cursorY, ' ');
      }
   } else if (n == 1) {
      // Clear from cursor to beginning of line
      for (int x = 1; x <= state->cursorX; ++x) {
         WHBConsoleSetChar(console, x, state->cursorY, ' ');
      }
   } else if (n == 2) {
      // Clear entire line
      for (int x = 1; x <= state->width; ++x) {
         WHBConsoleSetChar(console, x, state->cursorY, ' ');
      }
   }
}

void
WHBConsoleGraphicsReset(WHBConsole *console)
{
   WHBConsoleState *state = getConsoleState(console);
   state->graphicsFlags = 0;
   WHBConsoleGraphicsSetBackgroundColourDefault(console);
   WHBConsoleGraphicsSetForegroundColourDefault(console);
}

uint32_t
WHBConsoleGraphicsGetBackgroundColour(WHBConsole *console,
                                      const WHBColourTable *colourTable)
{
   WHBConsoleState *state = getConsoleState(console);
   if (state->backgroundColourIndex != -1) {
      return WHBConsoleColourTableLookup(colourTable, state->backgroundColourIndex);
   }

   return state->backgroundColour;
}

void
WHBConsoleGraphicsSetBold(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_BOLD;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_BOLD;
   }
}

void
WHBConsoleGraphicsSetFaint(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_FAINT;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_FAINT;
   }
}

void
WHBConsoleGraphicsSetItalic(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_ITALIC;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_ITALIC;
   }
}

void
WHBConsoleGraphicsSetUnderline(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_UNDERLINE;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_UNDERLINE;
   }
}

void
WHBConsoleGraphicsSetFastBlink(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_BLINK_FAST;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_BLINK_FAST;
   }
}

void
WHBConsoleGraphicsSetSlowBlink(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_BLINK_SLOW;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_BLINK_SLOW;
   }
}

void
WHBConsoleGraphicsSetReverse(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_REVERSE_VIDEO;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_REVERSE_VIDEO;
   }
}

void
WHBConsoleGraphicsSetConceal(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_CONCEAL;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_CONCEAL;
   }
}

void
WHBConsoleGraphicsSetCrossedOut(WHBConsole *console, BOOL on)
{
   WHBConsoleState *state = getConsoleState(console);
   if (on) {
      state->graphicsFlags |= WHB_CONSOLE_GRAPHICS_CROSSED_OUT;
   } else {
      state->graphicsFlags &= ~WHB_CONSOLE_GRAPHICS_CROSSED_OUT;
   }
}

void
WHBConsoleGraphicsSetForegroundColourDefault(WHBConsole *console)
{
   WHBConsoleGraphicsSetForegroundColourIndex(console, WHB_CONSOLE_COLOUR_WHITE);
}

void
WHBConsoleGraphicsSetBackgroundColourDefault(WHBConsole *console)
{
   WHBConsoleGraphicsSetBackgroundColourIndex(console, WHB_CONSOLE_COLOUR_BLACK);
}

void
WHBConsoleGraphicsSetForegroundColourIndex(WHBConsole *console, int index)
{
   WHBConsoleState *state = getConsoleState(console);
   state->foregroundColourIndex = index;
}

void
WHBConsoleGraphicsSetBackgroundColourIndex(WHBConsole *console, int index)
{
   WHBConsoleState *state = getConsoleState(console);
   state->backgroundColourIndex = index;
}

void
WHBConsoleGraphicsSetForegroundTrueColour(WHBConsole *console, int r, int g, int b)
{
   WHBConsoleState *state = getConsoleState(console);
   state->foregroundColour = (r << 24) | (g << 16) | (b << 8) | 0xFF;
   state->foregroundColourIndex = -1;
}

void
WHBConsoleGraphicsSetBackgroundTrueColour(WHBConsole *console, int r, int g, int b)
{
   WHBConsoleState *state = getConsoleState(console);
   state->backgroundColour = (r << 24) | (g << 16) | (b << 8) | 0xFF;
   state->backgroundColourIndex = -1;
}

static int
parseNextEscapeParameter(WHBConsoleState *state, int defaultValue)
{
   char *startPtr = state->escapeBuffer + state->escapeParsePosition;
   char *endPtr = NULL;
   int result = strtol(startPtr, &endPtr, 10);
   if (endPtr == startPtr) {
      result = defaultValue;
   }

   state->escapeParsePosition = endPtr - state->escapeBuffer;
   if (*endPtr == ';') {
      state->escapeParsePosition++;
   }

   return result;
}

static void
handleEscapeCode(WHBConsoleState *state, int chr)
{
   WHBConsole *console = (WHBConsole *)state;
   if (state->escapeBufferLength > 0) {
      if (chr < 0x20 || chr > 0x7E) {
         // Ignore invalid bytes
         return;
      }
   }

   state->escapeBuffer[state->escapeBufferLength++] = chr;
   state->escapeBuffer[state->escapeBufferLength] = 0;

   if (state->escapeBufferLength == 2 && chr != '[') {
      // Do not have an escape sequence, draw the characters as normal
      WHBConsoleSetChar(console, state->cursorX, state->cursorY, state->escapeBuffer[0]);
      WHBConsoleSetChar(console, state->cursorX + 1, state->cursorY,  state->escapeBuffer[1]);
      state->cursorX += 2;
      state->escapeBufferLength = 0;
      return;
   }

   if (state->escapeBufferLength <= 2) {
      // Wait for more characters
      return;
   }

   if (chr >= 0x30 && chr <= 0x3F) {
      // Parameter bytes
      return;
   }

   if (chr >= 0x20 && chr <= 0x2F) {
      // Intermediate bytes
      return;
   }

   state->escapeParsePosition = 2;

   // Handle command
   switch (chr) {
   case 'A': // Cursor Up
      WHBConsoleCursorUp(console, parseNextEscapeParameter(state, 1));
      break;
   case 'B': // Cursor Down
      WHBConsoleCursorDown(console, parseNextEscapeParameter(state, 1));
      break;
   case 'C': // Cursor Forward
      WHBConsoleCursorForward(console, parseNextEscapeParameter(state, 1));
      break;
   case 'D': // Cursor Back
      WHBConsoleCursorBack(console, parseNextEscapeParameter(state, 1));
      break;
   case 'E': // Cursor Next Line
      WHBConsoleCursorNextLine(console, parseNextEscapeParameter(state, 1));
      break;
   case 'F': // Cursor Previous Line
      WHBConsoleCursorPreviousLine(console, parseNextEscapeParameter(state, 1));
      break;
   case 'G': // Cursor Horizontal Absolute
      WHBConsoleCursorHorizontalAbsolute(console, parseNextEscapeParameter(state, 1));
      break;
   case 'H': // Cursor Position
   {
      int x = parseNextEscapeParameter(state, 1);
      int y = parseNextEscapeParameter(state, 1);
      WHBConsoleCursorPosition(console, x, y);
      break;
   }
   case 'I': // Cursor Forward Tabulation
      WHBConsoleCursorForwardTabulation(console, parseNextEscapeParameter(state, 0));
      break;
   case 'J': // Erase in Display
      WHBConsoleEraseInDisplay(console, parseNextEscapeParameter(state, 0));
      break;
   case 'K': // Erase in Line
      WHBConsoleEraseInLine(console, parseNextEscapeParameter(state, 0));
      break;
   case 'L': // Insert Lines
   case 'M': // Delete Lines
   case 'P': // Delete Characters
      // Not sure how these work
      break;
   case 'S': // Scroll Up
   case 'T': // Scroll Down
      // No scroll buffer
      break;
   case 'm':
   {
      do {
         int code = parseNextEscapeParameter(state, 0);

         switch (code) {
         case 0:
            WHBConsoleGraphicsReset(console);
            break;
         case 1:
            WHBConsoleGraphicsSetBold(console, TRUE);
            break;
         case 2:
            WHBConsoleGraphicsSetFaint(console, TRUE);
            break;
         case 3:
            WHBConsoleGraphicsSetItalic(console, TRUE);
            break;
         case 4:
            WHBConsoleGraphicsSetUnderline(console, TRUE);
            break;
         case 5:
            WHBConsoleGraphicsSetSlowBlink(console, TRUE);
            break;
         case 6:
            WHBConsoleGraphicsSetFastBlink(console, TRUE);
            break;
         case 7:
            WHBConsoleGraphicsSetReverse(console, TRUE);
            break;
         case 8:
            WHBConsoleGraphicsSetConceal(console, TRUE);
            break;
         case 9:
            WHBConsoleGraphicsSetCrossedOut(console, TRUE);
            break;
         case 21:
            WHBConsoleGraphicsSetBold(console, FALSE);
            break;
         case 22:
            WHBConsoleGraphicsSetBold(console, FALSE);
            WHBConsoleGraphicsSetFaint(console, FALSE);
            break;
         case 23:
            WHBConsoleGraphicsSetItalic(console, FALSE);
            break;
         case 24:
            WHBConsoleGraphicsSetUnderline(console, FALSE);
            break;
         case 25:
            WHBConsoleGraphicsSetFastBlink(console, FALSE);
            WHBConsoleGraphicsSetSlowBlink(console, FALSE);
            break;
         case 27:
            WHBConsoleGraphicsSetReverse(console, FALSE);
            break;
         case 28:
            WHBConsoleGraphicsSetConceal(console, FALSE);
            break;
         case 29:
            WHBConsoleGraphicsSetCrossedOut(console, FALSE);
            break;
         case 30:
         case 31:
         case 32:
         case 33:
         case 34:
         case 35:
         case 36:
         case 37:
            WHBConsoleGraphicsSetForegroundColourIndex(console, code - 30);
            break;
         case 38:
         {
            int mode = parseNextEscapeParameter(state, 0);
            if (mode == 2) { // True Colour
               int r = parseNextEscapeParameter(state, 0);
               int g = parseNextEscapeParameter(state, 0);
               int b = parseNextEscapeParameter(state, 0);
               WHBConsoleGraphicsSetForegroundTrueColour(console, r, g, b);
            } else if (mode == 5) { // 256 Colour
               WHBConsoleGraphicsSetForegroundColourIndex(console, parseNextEscapeParameter(state, 0));
            }
            break;
         }
         case 39:
            WHBConsoleGraphicsSetForegroundColourDefault(console);
            break;
         case 40:
         case 41:
         case 42:
         case 43:
         case 44:
         case 45:
         case 46:
         case 47:
            WHBConsoleGraphicsSetBackgroundColourIndex(console, code - 40);
            break;
         case 48:
         {
            int mode = parseNextEscapeParameter(state, 0);
            if (mode == 2) { // True Colour
               int r = parseNextEscapeParameter(state, 0);
               int g = parseNextEscapeParameter(state, 0);
               int b = parseNextEscapeParameter(state, 0);
               WHBConsoleGraphicsSetBackgroundTrueColour(console, r, g, b);
            } else if (mode == 5) { // 256 Colour
               WHBConsoleGraphicsSetBackgroundColourIndex(console, parseNextEscapeParameter(state, 0));
            }
            break;
         }
         case 49:
            WHBConsoleGraphicsSetBackgroundColourDefault(console);
            break;
         default:
            // Unrecongised, lets stop parsing
            state->escapeParsePosition = state->escapeBufferLength;
         }
      } while (state->escapeParsePosition < state->escapeBufferLength - 1);
      break;
   }
   default:
      // Unsupported sequence, ignore.
      break;
   }

   state->escapeBufferLength = 0;
   state->escapeBuffer[0] = 0;
}

static void
scrollScreenForNewLine(WHBConsoleState *state)
{
   WHBConsole *console = (WHBConsole *)state;

   // Scroll up lines
   for (int y = 1; y < state->height; ++y) {
      for (int x = 1; x <= state->width; ++x) {
         *getConsoleCharacter(state, x, y) = *getConsoleCharacter(state, x, y + 1);
      }
   }

   // Clear last line
   for (int x = 1; x <= state->width; ++x) {
      WHBConsoleSetChar(console, x, state->height, ' ');
   }
}

void
WHBConsoleWriteChar(WHBConsole *console, int chr)
{
   WHBConsoleState *state = getConsoleState(console);
   if (chr == 0x1B || state->escapeBufferLength) {
      handleEscapeCode(state, chr);
      return;
   }

   if (chr == '\t') {
      int diffX = state->tabWidth - ((state->cursorX - 1) % state->tabWidth);
      for (int i = 0; i < diffX; ++i) {
         WHBConsoleWriteChar(console, ' ');
      }
   } else if (chr == '\n') {
      if (state->cursorY == state->height) {
         scrollScreenForNewLine(state);
      } else {
         state->cursorY++;
      }

      state->cursorX = 1;
   } else if (chr == '\r') {
      WHBConsoleCursorHorizontalAbsolute(console, 1);
   } else if (chr == '\b') {
      WHBConsoleCursorBack(console, 1);
      WHBConsoleSetChar(console, state->cursorX, state->cursorY, ' ');
   } else {
      WHBConsoleSetChar(console, state->cursorX, state->cursorY, chr);
      WHBConsoleCursorForward(console, 1);
   }
}

void
WHBConsoleWriteText(WHBConsole *console, const char *text)
{
   while (*text) {
      WHBConsoleWriteChar(console, *text++);
   }
}

static uint32_t
lookupColour(const WHBColourTable *colourTable, int index)
{
   if (index >= colourTable->numColours) {
      return colourTable->colours[WHB_CONSOLE_COLOUR_WHITE];
   }

   return colourTable->colours[index];
}

static const uint8_t *
lookupFont(const WHBConsoleFont *font, int chr)
{
   chr -= font->offset;
   if (chr < 0 || chr >= font->count) {
      chr = 0;
   }

   return font->buffer + font->stride * chr;
}

/*
TODO:
   WHB_CONSOLE_GRAPHICS_FAINT          = 1 << 1,
   WHB_CONSOLE_GRAPHICS_ITALIC         = 1 << 2,
 */
void
WHBConsoleRender(WHBConsole *console,
                 uint32_t *buffer,
                 int bufferStride,
                 int bufferHeight,
                 const WHBColourTable *colourTable,
                 const WHBConsoleFont *font)
{
   WHBConsoleState *state = getConsoleState(console);
   OSTime now = OSGetSystemTime();
   BOOL slowBlinkVisible = (now / state->slowBlinkInterval) % 2;
   BOOL fastBlinkVisible = (now / state->fastBlinkInterval) % 2;

   for (int y = 1; y <= state->height; ++y) {
      for (int x = 1; x <= state->width; ++x) {
         WHBConsoleCharacter *character = getConsoleCharacter(state, x, y);
         uint32_t foregroundColour = character->foregroundColour;
         uint32_t backgroundColour = character->backgroundColour;

         if (character->flags & WHB_CONSOLE_CHARACTER_FG_COLOUR_INDEX) {
            if (character->flags & WHB_CONSOLE_GRAPHICS_BOLD) {
               if (foregroundColour < 8) {
                  foregroundColour += 8;
               }
            }

            foregroundColour = WHBConsoleColourTableLookup(colourTable, foregroundColour);
         }

         if (character->flags & WHB_CONSOLE_CHARACTER_BG_COLOUR_INDEX) {
            backgroundColour = WHBConsoleColourTableLookup(colourTable, backgroundColour);
         }

         if (character->flags & WHB_CONSOLE_GRAPHICS_REVERSE_VIDEO) {
            uint32_t tmp = backgroundColour;
            backgroundColour = foregroundColour;
            foregroundColour = tmp;
         }

         uint32_t *screenDst = buffer + (y - 1) * font->height * bufferStride + (x - 1) * font->width;
         uint32_t fontPixelStride = (font->width + 7) / 8;

         if (font->format == WHB_CONSOLE_FONT_FORMAT_BITMAP ||
             font->format == WHB_CONSOLE_FONT_FORMAT_BITMAP_INVERSE) {
            const uint8_t *bitmapFont = lookupFont(font, character->chr);

            for (int fY = 0; fY < font->height; ++fY) {
               uint32_t *dst = screenDst + fY * bufferStride;
               const uint8_t *src = bitmapFont + fY * fontPixelStride;

               for (int fX = 0; fX < font->width; ++fX) {
                  uint8_t mask = src[fX / 8];

                  if (character->flags & WHB_CONSOLE_GRAPHICS_CROSSED_OUT) {
                     if (fY == (font->height - 1) / 2) {
                        mask = 0xFF;
                     }
                  }

                  if (character->flags & WHB_CONSOLE_GRAPHICS_UNDERLINE) {
                     if (fY == (font->height - 1)) {
                        mask = 0xFF;
                     }
                  }

                  if (character->flags & WHB_CONSOLE_GRAPHICS_CONCEAL) {
                     mask = 0;
                  }

                  if (character->flags & WHB_CONSOLE_GRAPHICS_BLINK_SLOW) {
                     if (!slowBlinkVisible) {
                        mask = 0;
                     }
                  }

                  if (character->flags & WHB_CONSOLE_GRAPHICS_BLINK_FAST) {
                     if (!fastBlinkVisible) {
                        mask = 0;
                     }
                  }

                  if (font->format == WHB_CONSOLE_FONT_FORMAT_BITMAP_INVERSE) {
                     mask &= 1 << ((font->width - 1) - (fX % 8));
                  } else {
                     mask &= 1 << (fX % 8);
                  }

                  *dst++ = mask ? foregroundColour : backgroundColour;
               }
            }
         }
      }
   }
}
