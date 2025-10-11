#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int main() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char dateTime[15] = { 0 };

    char fileName[19] = { 0 };
    
    strftime(dateTime, sizeof(dateTime), "%d%m%Y%H%M%S", t);
    printf("Formatted time: %s\n", dateTime);

    strcpy(fileName, dateTime);
    strcat(fileName, ".png");

    puts(fileName);

    return 0;
}

// Note: This example displays the server's local time, which may differ from your local time.