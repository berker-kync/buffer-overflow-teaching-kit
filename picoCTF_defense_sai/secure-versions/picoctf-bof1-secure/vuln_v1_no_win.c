#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vuln() {
    char buf[32];

    printf("Enter some text: ");

    // SAFE REPLACEMENT: fgets instead of gets
    // fgets reads at most sizeof(buf)-1 characters and prevents overflow
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }

    // Remove newline from fgets
    buf[strcspn(buf, "\n")] = '\0';

    printf("You entered: %s\n", buf);
}

int main() {
    vuln();
    return 0;
}
