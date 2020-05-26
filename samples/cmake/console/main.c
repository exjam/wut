#include "console.h"

#include <coreinit/cache.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/screen.h>

#include <gx2/event.h>
#include <whb/proc.h>

#include <stdio.h>

#define FRAME_HEAP_TAG (0x000DECAF)

static BOOL isDecaf = FALSE;

int
main(int argc, char **argv)
{
   WHBProcInit();

   MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
   MEMRecordStateForFrmHeap(heap, FRAME_HEAP_TAG);

   const WHBConsoleFont *font = WHBConsoleGetFont12x19();
   int width = 854 / font->width;
   int height = 480 / font->height;
   int scrollbackLines = 0;
   uint32_t workMemorySize = WHBConsoleCalculateWorkMemorySize(width, height, scrollbackLines);
   void *workMemory = MEMAllocFromDefaultHeap(workMemorySize);
   WHBConsole *console = WHBConsoleInit(workMemory, workMemorySize, width, height, scrollbackLines);

   OSScreenInit();
   uint32_t bufferSizeTV = OSScreenGetBufferSizeEx(SCREEN_TV);
   uint32_t bufferSizeDRC = OSScreenGetBufferSizeEx(SCREEN_DRC);

   void *bufferTV = MEMAllocFromFrmHeapEx(heap, bufferSizeTV, 4);
   void *bufferDRC = MEMAllocFromFrmHeapEx(heap, bufferSizeDRC, 4);

   OSScreenSetBufferEx(SCREEN_TV, bufferTV);
   OSScreenSetBufferEx(SCREEN_DRC, bufferDRC);

   OSScreenEnableEx(SCREEN_TV, 1);
   OSScreenEnableEx(SCREEN_DRC, 1);


   WHBConsoleGraphicsSetBackgroundColourIndex(console, WHB_CONSOLE_COLOUR_RED);
   WHBConsoleClear(console);

   WHBConsoleWriteText(console, "Normal\n");
   WHBConsoleWriteText(console, "\x1B[1mBold\x1B[21m\n");
   WHBConsoleWriteText(console, "\x1B[2mFaint\x1B[22m\n");
   WHBConsoleWriteText(console, "\x1B[3mItalic\x1B[23m\n");
   WHBConsoleWriteText(console, "\x1B[4mUnderline\x1B[24m\n");
   WHBConsoleWriteText(console, "\x1B[5mBlink Slow\x1B[25m\n");
   WHBConsoleWriteText(console, "\x1B[6mBlink Fast\x1B[25m\n");
   WHBConsoleWriteText(console, "\x1B[7mReverse\x1B[27m\n");
   WHBConsoleWriteText(console, "\x1B[8mConcealed\x1B[28m(<-- Concealed)\n");
   WHBConsoleWriteText(console, "\x1B[9mCrossed out\x1B[29m\n");
   WHBConsoleWriteText(console, "\x1B[32mGreen Text\n");
   WHBConsoleWriteText(console, "\x1B[m"); // Should reset
   WHBConsoleWriteText(console, "Reset Text\n");
   WHBConsoleWriteText(console, "\x1B[1235m"); // Should do nothing
   WHBConsoleWriteText(console, "Reset Text\n");
   WHBConsoleWriteText(console, "\x1B[1;31;42m Yes it is awful \x1B[0m");

   WHBConsoleGraphicsSetBackgroundColourIndex(console, WHB_CONSOLE_COLOUR_RED);

   while (WHBProcIsRunning()) {
      OSScreenClearBufferEx(SCREEN_TV, WHBConsoleGraphicsGetBackgroundColour(console, WHBConsoleGetDefaultColourTable()));
      OSScreenClearBufferEx(SCREEN_DRC, WHBConsoleGraphicsGetBackgroundColour(console, WHBConsoleGetDefaultColourTable()));

      WHBConsoleRender(console,
                       bufferDRC,
                       isDecaf ? 854 : 896,
                       480,
                       WHBConsoleGetDefaultColourTable(),
                       font);

      if (!isDecaf) {
         WHBConsoleRender(console,
                          (uint32_t *)(((uint8_t *)bufferDRC) + 896 * 4 * 480),
                          isDecaf ? 854 : 896,
                          480,
                          WHBConsoleGetDefaultColourTable(),
                          font);
      }

      DCFlushRange(bufferTV, bufferSizeTV);
      DCFlushRange(bufferDRC, bufferSizeDRC);
      OSScreenFlipBuffersEx(SCREEN_TV);
      OSScreenFlipBuffersEx(SCREEN_DRC);
      OSSleepTicks(OSMillisecondsToTicks(100));
   }

   WHBProcShutdown();
   GX2GetLastSubmittedTimeStamp();
   return 0;
}
