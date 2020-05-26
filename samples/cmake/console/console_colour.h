#pragma once
#include <wut.h>

enum WHHConsoleColours
{
   WHB_CONSOLE_COLOUR_BLACK,
   WHB_CONSOLE_COLOUR_RED,
   WHB_CONSOLE_COLOUR_GREEN,
   WHB_CONSOLE_COLOUR_YELLOW,
   WHB_CONSOLE_COLOUR_BLUE,
   WHB_CONSOLE_COLOUR_MAGENTA,
   WHB_CONSOLE_COLOUR_CYAN,
   WHB_CONSOLE_COLOUR_WHITE,

   WHB_CONSOLE_COLOUR_BRIGHT_BLACK,
   WHB_CONSOLE_COLOUR_BRIGHT_RED,
   WHB_CONSOLE_COLOUR_BRIGHT_GREEN,
   WHB_CONSOLE_COLOUR_BRIGHT_YELLOW,
   WHB_CONSOLE_COLOUR_BRIGHT_BLUE,
   WHB_CONSOLE_COLOUR_BRIGHT_MAGENTA,
   WHB_CONSOLE_COLOUR_BRIGHT_CYAN,
   WHB_CONSOLE_COLOUR_BRIGHT_WHITE,

   // From here onwards are the 16..256 colour table entries
};

typedef struct WHBColourTable WHBColourTable;

struct WHBColourTable
{
   int numColours;
   const uint32_t *colours;
};

const WHBColourTable *
WHBConsoleGetDefaultColourTable();

static uint32_t
WHBConsoleColourTableLookup(const WHBColourTable *colourTable, int index)
{
   if (index >= colourTable->numColours) {
      return colourTable->colours[WHB_CONSOLE_COLOUR_WHITE];
   }

   return colourTable->colours[index];
}
