#pragma once
#include <wut.h>
#include <coreinit/time.h>

#include "console_colour.h"
#include "console_font.h"

typedef void WHBConsole;

uint32_t
WHBConsoleCalculateWorkMemorySize(int width,
                                  int height,
                                  int scrollbackLines);

WHBConsole *
WHBConsoleInit(void *workMemory,
               uint32_t workMemorySize,
               int width,
               int height,
               int scrollbackLines);

void
WHBConsoleRender(WHBConsole *console,
                 uint32_t *buffer,
                 int bufferStride,
                 int bufferHeight,
                 const WHBColourTable *colourTable,
                 const WHBConsoleFont *font);

void
WHBConsoleClear(WHBConsole *console);

void
WHBConsoleSetChar(WHBConsole *console, int x, int y, int chr);

void
WHBConsoleSetTabWidth(WHBConsole *console, int width);

void
WHBConsoleSetFastBlinkInterval(WHBConsole *console, OSTime interval);

void
WHBConsoleSetSlowBlinkInterval(WHBConsole *console, OSTime interval);

void
WHBConsoleWriteChar(WHBConsole *console, int chr);

void
WHBConsoleWriteText(WHBConsole *console, const char *text);

void
WHBConsoleCursorUp(WHBConsole *console, int n);

void
WHBConsoleCursorDown(WHBConsole *console, int n);

void
WHBConsoleCursorBack(WHBConsole *console, int n);

void
WHBConsoleCursorForward(WHBConsole *console, int n);

void
WHBConsoleCursorNextLine(WHBConsole *console, int n);

void
WHBConsoleCursorPreviousLine(WHBConsole *console, int n);

void
WHBConsoleCursorHorizontalAbsolute(WHBConsole *console, int n);

void
WHBConsoleCursorPosition(WHBConsole *console, int x, int y);

void
WHBConsoleCursorForwardTabulation(WHBConsole *console, int n);

void
WHBConsoleEraseInDisplay(WHBConsole *console, int n);

void
WHBConsoleEraseInLine(WHBConsole *console, int n);

void
WHBConsoleGraphicsReset(WHBConsole *console);

uint32_t
WHBConsoleGraphicsGetBackgroundColour(WHBConsole *console,
                                      const WHBColourTable *colourTable);

void
WHBConsoleGraphicsSetBold(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetFaint(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetItalic(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetUnderline(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetFastBlink(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetSlowBlink(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetReverse(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetConceal(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetCrossedOut(WHBConsole *console, BOOL on);

void
WHBConsoleGraphicsSetForegroundColourDefault(WHBConsole *console);

void
WHBConsoleGraphicsSetBackgroundColourDefault(WHBConsole *console);

void
WHBConsoleGraphicsSetForegroundColourIndex(WHBConsole *console, int index);

void
WHBConsoleGraphicsSetBackgroundColourIndex(WHBConsole *console, int index);

void
WHBConsoleGraphicsSetForegroundColourIndex256(WHBConsole *console, int index);

void
WHBConsoleGraphicsSetBackgroundColourIndex256(WHBConsole *console, int index);

void
WHBConsoleGraphicsSetForegroundTrueColour(WHBConsole *console, int r, int g, int b);

void
WHBConsoleGraphicsSetBackgroundTrueColour(WHBConsole *console, int r, int g, int b);
