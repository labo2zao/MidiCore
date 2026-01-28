#include <stdio.h>
#include <stdint.h>

#define SIZE 256

int main() {
    // Test the boundary case
    uint16_t pos = 255;
    
    printf("Buffer size: %d\n", SIZE);
    printf("Current pos: %d\n", pos);
    printf("\n");
    
    // OLD check (buggy?):
    if (pos < SIZE) {
        printf("OLD check (pos < SIZE): TRUE - would write to buffer[%d]\n", pos);
        pos++;
        printf("  After write: pos = %d\n", pos);
        if (pos > SIZE) {
            printf("  ERROR: pos > SIZE! This is out of bounds!\n");
        } else if (pos == SIZE) {
            printf("  OK: pos == SIZE (buffer full, 256 bytes written)\n");
        }
    }
    printf("\n");
    
    pos = 255;
    // NEW check (fixed):
    if (pos + 1 <= SIZE) {
        printf("NEW check (pos + 1 <= SIZE): TRUE - would write to buffer[%d]\n", pos);
        pos++;
        printf("  After write: pos = %d\n", pos);
        if (pos > SIZE) {
            printf("  ERROR: pos > SIZE! This is out of bounds!\n");
        } else if (pos == SIZE) {
            printf("  OK: pos == SIZE (buffer full, 256 bytes written)\n");
        }
    }
    
    return 0;
}
