#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Version 1: Safe Code Practices
// Defense: Input validation + safe functions
// Prevents overflow at CODE level

int bof(char *str)
{
    char buffer[24];
    
    // DEFENSE 1: Validate input length
    size_t input_len = strlen(str);
    if (input_len >= sizeof(buffer)) {
        printf("Error: Input too long (%zu bytes). Maximum: %zu bytes\n",
               input_len, sizeof(buffer) - 1);
        return 0;
    }
    
    // DEFENSE 2: Use safe copy
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    printf("Input processed: %s\n", buffer);
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];
    FILE *badfile;

    badfile = fopen("badfile", "r");
    if (badfile == NULL) {
        printf("Error: Cannot open badfile\n");
        return 1;
    }
    
    size_t bytes_read = fread(str, sizeof(char), 517, badfile);
    str[bytes_read] = '\0';
    fclose(badfile);
    
    if (bof(str)) {
        printf("Returned Properly\n");
    } else {
        printf("Input rejected\n");
    }
    
    return 0;
}
