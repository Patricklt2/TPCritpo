#include <Permutation_Table.h>
#define MAX 50
#define SET 10
/* variable global */

int64_t seed; /* seed debe ser de 48 bits; se elige este tipo de 64 bits */



void setSeed(int64_t s) {
    seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t nextChar(void) {
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(seed >> 40);
}

void generate_permutation_table(uint16_t* R, int size, uint16_t seed) {
    setSeed(seed);
    for (int i = 0; i < size; i++) {
        R[i] = nextChar() % size;  // Ensure values are in [0, size-1]
    }
}

/*
int main() {
    int i;
    uint8_t num;
    setSeed(SET);
    for (i = 0; i < MAX; i++) {
        num = nextChar();
        printf("%d\t", num);
    }
    return EXIT_SUCCESS;
}
*/