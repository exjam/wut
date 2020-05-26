#pragma once
#include <wut.h>

typedef struct WHBConsoleFont WHBConsoleFont;
typedef enum WHHConsoleFontFormat WHHConsoleFontFormat;

enum WHHConsoleFontFormat
{
   WHB_CONSOLE_FONT_FORMAT_INVALID,

   //! Font is a bit array where 1 = foreground colour, 0 = background colour
   WHB_CONSOLE_FONT_FORMAT_BITMAP,

   //! Same as WHB_CONSOLE_FONT_FORMAT_BITMAP but the bits in a byte are in the
   //! opposite order...
   WHB_CONSOLE_FONT_FORMAT_BITMAP_INVERSE,
};

struct WHBConsoleFont
{
   //! Format of font in buffer.
   WHHConsoleFontFormat format;

   //! Font buffer
   const uint8_t *buffer;

   //! Width of a character in pixels.
   int width;

   //! Height of a character in pixels.
   int height;

   //! The number of bytes between each character in the font.
   int stride;

   //! The ASCII code of the first character in the font.
   int offset;

   //! The maximum number of characters in the font.
   int count;
};

const WHBConsoleFont *
WHBConsoleGetDefaultFont();

const WHBConsoleFont *
WHBConsoleGetFont8x8();

const WHBConsoleFont *
WHBConsoleGetFont12x19();
