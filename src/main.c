

#include <stdio.h>

int main(int argc, char *argv[]) {

    // Check if the correct number of arguments is provided

    if (argc != 6) {
        printf("Use: %s -r|--d -secret <imagen> -k <numero>\n", argv[0]);
        return 1;
    }

    int mode = 0; // 1 = -r, 2 = --d
    if (strcmp(argv[1], "-r") == 0) {
        mode = 1;
    } else if (strcmp(argv[1], "-d") == 0) {
        mode = 2;
    } else {
        printf("First argument must be -r o -d\n");
        return 1;
    }

    if (strcmp(argv[2], "-secret") != 0) {
        printf("Second argument must be -secret plus an image\n");
        return 1;
    }

    char *imagen = argv[3];

    if (strcmp(argv[4], "-k") != 0) {
        printf("No argument -k found\n");
        return 1;
    }

    int shades = atoi(argv[5]);

    if (shades < 2 || shades > 10) {
        printf("The shades must be a number between 2 and 10\n");
        return 1;
    }

    int result = 0;
    // Call the appropriate function based on the mode
    if (mode == 1) {
        // Call the function to hide the secret in the image
        result = encrypt_image(imagen, shades);
    } else if (mode == 2) {
        // Call the function to reveal the secret from the image
        result = decrypt_image(imagen, shades);
    }
    else {
        printf("Invalid mode selected\n");
        return 1;
    }
    
    if (result != 0) {
        printf("An error occurred during the operation.\n");
        return result;
    }
    printf("Operation completed successfully.\n");
    return 0;
}

