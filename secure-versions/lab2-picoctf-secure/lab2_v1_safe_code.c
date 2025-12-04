#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Version 1: Safe Code Practices
// Defense: Input validation + safe functions
// This prevents overflow at the CODE level

void win() {
    printf("Congratulations! You win!\n");
}

void vuln() {
    char buf[32];
    char input[256];

    printf("Enter some text: ");
    
    // DEFENSE 1: Use safe input function (fgets)
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }
    
    input[strcspn(input, "\n")] = 0;
    
    // DEFENSE 2: Validate input length
    if (strlen(input) >= sizeof(buf)) {
        printf("Input too long! Maximum %lu characters\n", sizeof(buf) - 1);
        return;
    }
    
    // DEFENSE 3: Use safe copy function
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    
    printf("You entered: %s\n", buf);
}

int main(int argc, char **argv) {
    vuln();
    return 0;
}
