#include <config.h>
#include <render.h>
#include <stdlib.h>
#include <fonts_manager.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


static int g_iIntervalSecond = 1;

void GetIntervalTime(int *piIntervalSecond)
{
    *piIntervalSecond = g_iIntervalSecond;
}
