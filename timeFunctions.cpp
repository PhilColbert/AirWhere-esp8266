#include <Arduino.h>
#include "WebHelper.h"

bool myDelay(int timeToWait)
{
  int timeNow=millis();
  while (timeNow+timeToWait>millis())
  {
    yield();
    Web_loop();
  }
  
}

bool yieldDelay(int timeToWait)
{
  int timeNow=millis();
  while (timeNow+timeToWait>millis())
  {
    yield();
  }
  
}


