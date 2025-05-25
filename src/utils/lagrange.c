#include "lagrange.h"

int mod_inverse(int a) {
    int t = 0, nt = 1, r = MOD, nr = a;
    while (nr != 0) {
        int q = r / nr;
        int tmp = nt; nt = t - q * nt; t = tmp;
        tmp = nr; nr = r - q * nr; r = tmp;
    }
    if (r > 1) return -1; // Si no hay inversa
    return (t < 0) ? t + MOD : t;
}

int lagrange_interpolate(int x[], int y[], int k) {
    int secret = 0;
    for (int i = 0; i < k; i++) {
        int term = y[i];
        for (int j = 0; j < k; j++) {
            if (i != j) {
                int denom = (x[j] - x[i]) % MOD;
                if (denom < 0) denom += MOD;
                int inv_denom = mod_inverse(denom);
                term = (term * x[j]) % MOD;
                term = (term * inv_denom) % MOD;
            }
        }
        secret = (secret + term) % MOD;
    }
    return secret;
}