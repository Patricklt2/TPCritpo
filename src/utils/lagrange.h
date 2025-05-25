#ifndef LAGRANGE_H
#define LAGRANGE_H

#include <stdio.h>
#include <stdlib.h>

#define MOD 257

/**
* Inverso modular con algoritmo extendido de Euclides (SE UTILIZA MOD 257)
* @param a entero del que se quiere calcular el inverso 
*/
int mod_inverse(int a);

/**
* Polinomio interpolador de lagrange para recuperar el secreto
* @param xi numero de sombra
* @param yi valor de la sombra
* @param k numero de sombras minimo para reconstruir el secreto
*/
int lagrange_interpolate(int x[], int y[], int k);
#endif