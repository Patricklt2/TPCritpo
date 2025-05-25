package src.utils;

import java.util.Random;

/**
 * Implementacion de la catedra
 */
public class tablaDePermutacion {
    public static void main(String[] args) {
        Random rnd = new Random();
        rnd.setSeed(10);
        for(int i=0; i < 50; i++)
        {
            int num = rnd.nextInt(256);/*genera nro en [0,255]*/
            System.out.println(num);
        }
    }
}