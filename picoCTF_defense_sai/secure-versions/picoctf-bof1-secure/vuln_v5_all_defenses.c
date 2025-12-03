#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("Congratulations! You win!\n");
}

void vuln() {
    char buf[32];
    char input[256];

    // DEFENSE 1: Safe input function (fgets instead of gets)
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }

    // Remove newline if present
    input[strcspn(input, "\n")] = 0;

    // DEFENSE 2: Input validation (length check)
    if (strlen(input) >= sizeof(buf)) {
        printf("Input too long! Maximum %lu characters.\n",
               sizeof(buf) - 1);
        return;
    }

    // DEFENSE 3: Safe copy into fixed-size buffer
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    puts(buf);
}

int main(int argc, char **argv) {
    printf("Enter some text: ");
    vuln();
    return 0;
}
