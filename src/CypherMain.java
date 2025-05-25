package src;

import java.nio.file.*;

public class CypherMain {
    // 1 Byte = 1 pixel => escala de grises
    // los bmp tienen 54 de encabezado --> Ver BITMAPINFOHEADER
    // La imagen se lee de abajo hacia arriba y de izq a derecha
    // K esta en [2, 10] y k <= n
    // n >= 2
    // para una imagen de m pixeles se generaran m valores de [0, 255]
    // se generará a partir de una semilla ocultada en el archivo bmp

    /**
     * Equema k, n  donde k es la minima cantidad de sombras para
     * recuperar el secreto original y n la cantidad de sombras
     *
     * Si k == 8 las imagenes deben tener mismo largo que ancho (cuadrado) y ademas uso LSB para ocultar
     * Si no hay n imagenes que cumplan eso, fallo con msj de error y abort
     * Para k != 8 queda en nosotros definir el tamaño de img y metodo de ocultamiento
     * Usamos MOD 257
     */

    // Las seeds para cada uno es de 2 bytes y se esconde en los bytes 6 y 7 del bmp
    // los bytes 8 y 9 se usaran para indicar el numero de sombra


    private static final int MOD = 257;

    public static void main(String[] args) throws Exception {
        String inputPath = "src/assets/Alfred.bmp";
        byte[] bmp = Files.readAllBytes(Paths.get(inputPath));

        // Se guardan en formato little endian
        // Posicion en bytes donde arranca el image data se extrae de Bytes 11 a 14
        int offset = ((bmp[13] & 0xFF) << 24)
                | ((bmp[12] & 0xFF) << 16)
                | ((bmp[11] & 0xFF) << 8)
                | (bmp[10] & 0xFF);

        // Ancho de 19 a 22
        int width = ((bmp[21] & 0xFF) << 24)
                | ((bmp[20] & 0xFF) << 16)
                | ((bmp[19] & 0xFF) << 8)
                | (bmp[18] & 0xFF);

        // Alto de 23 a 26
        int height = ((bmp[25] & 0xFF) << 24)
                | ((bmp[24] & 0xFF) << 16)
                | ((bmp[23] & 0xFF) << 8)
                | (bmp[22] & 0xFF);


    }
}



