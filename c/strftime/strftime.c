#include <stdio.h>
#include <time.h>

int main() {
    time_t tt;
    char tmpbuf[80];
    tt=time(NULL);
    strftime(tmpbuf,80,"%Y-%m-%d %H:%M:%S\n",localtime(&tt));
    printf(tmpbuf);
    return 0;
}



