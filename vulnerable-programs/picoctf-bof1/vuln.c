#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void win() {
    printf("You successfully jumped to the win function!\n");
}

void vuln() {
    char buffer[32];
    printf("Enter some text: ");
    gets(buffer);
}

int main() {
    vuln();
    return 0;
}
