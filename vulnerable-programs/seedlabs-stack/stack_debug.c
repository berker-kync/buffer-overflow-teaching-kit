#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int bof(char *str)
{
    char buffer[24];
    printf("DEBUG: Buffer at %p\n", buffer);  // Print buffer address!
    strcpy(buffer, str);
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];
    FILE *badfile;
    
    badfile = fopen("badfile", "r");
    if (badfile == NULL) {
        printf("Error: cannot open badfile\n");
        return 1;
    }
    
    fread(str, sizeof(char), 517, badfile);
    bof(str);
    printf("Returned Properly\n");
    fclose(badfile);
    return 1;
}
