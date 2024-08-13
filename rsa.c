#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Base64 encoding/decoding table
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Table used for padding during Base64 encoding
static const int mod_table[] = {0, 2, 1};

// Function to encode data to Base64
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = (char *)malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = b64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = '\0';
    return encoded_data;
}

// Function to decode Base64 data
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length) {
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = (unsigned char *)malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : strchr(b64_table, data[i++]) - b64_table;
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : strchr(b64_table, data[i++]) - b64_table;
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : strchr(b64_table, data[i++]) - b64_table;
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : strchr(b64_table, data[i++]) - b64_table;

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

// The Modular Exponentiation Algorithm
int MEA(int p, int e, int n) {
    int r2 = 1;
    int r1 = 0;
    int Q = 0;
    int R = 0;

    while(e != 0) {
        R = (e % 2);
        Q = ((e - R) / 2);
        r1 = ((p * p) % n);

        if(R == 1) {
            r2 = ((r2 * p) % n);
        }
        p = r1;
        e = Q;
    }
    return r2;
}

// Encode function
void encode(int e, int n) {
    char input[100000];
    int text[100000];
    size_t len = 0;

    printf("Enter text to be encrypted: ");
    getchar();  // Consume leftover newline
    fgets(input, sizeof(input), stdin);

    size_t input_len = strlen(input);
    if(input[input_len - 1] == '\n') {
        input[input_len - 1] = '\0';
        input_len--;
    }

    for(size_t i = 0; i < input_len; i++) {
        text[i] = MEA(input[i], e, n);
    }
    len = input_len;

    unsigned char *encoded_bytes = (unsigned char *)text;
    size_t output_length;
    char *base64_encoded = base64_encode(encoded_bytes, len * sizeof(int), &output_length);

    if(base64_encoded != NULL) {
        printf("\n*************************************\n");
        printf("  Ciphered text (Base64): %s\n", base64_encoded);
        free(base64_encoded);
    }
}

// Decode function
void decode(int d, int n) {
    char base64_input[100000];
    size_t output_length;
    int text[100000];

    printf("Enter the ciphered text (Base64): ");
    getchar();  // Consume leftover newline
    fgets(base64_input, sizeof(base64_input), stdin);

    size_t base64_len = strlen(base64_input);
    if(base64_input[base64_len - 1] == '\n') {
        base64_input[base64_len - 1] = '\0';
    }

    unsigned char *decoded_bytes = base64_decode(base64_input, strlen(base64_input), &output_length);
    if(decoded_bytes == NULL) {
        printf("Invalid Base64 input.\n");
        return;
    }

    size_t len = output_length / sizeof(int);
    memcpy(text, decoded_bytes, output_length);

    printf("\n*************************************\n");
    printf("  Deciphered text: ");
    for(size_t i = 0; i < len; i++) {
        int decoded_char = MEA(text[i], d, n);
        printf("%c", decoded_char);
    }
    printf("\n");

    free(decoded_bytes);
}

// Helper function to check if the input number is a prime number
bool check_prime(int num) {
    if(num < 2) {
        return false;
    }
    for(int i = 2; i * i <= num; i++) {
        if(num % i == 0) {
            return false;
        }
    }
    return true;
}

// Helper function to find the greatest common divisor between two numbers
int gcd(int num1, int num2) {
    while(num2 != 0) {
        int temp = num2;
        num2 = num1 % num2;
        num1 = temp;
    }
    return num1;
}

// Function to find modular inverse
int mod_inverse(int e, int phi) {
    int t = 0, new_t = 1;
    int r = phi, new_r = e;

    while(new_r != 0) {
        int quotient = r / new_r;
        int temp = t;
        t = new_t;
        new_t = temp - quotient * new_t;

        temp = r;
        r = new_r;
        new_r = temp - quotient * new_r;
    }

    if(r > 1) return -1;  // No modular inverse
    if(t < 0) t += phi;

    return t;
}

// Function to check if 'e' is valid
bool check_e(int e, int phi) {
    return (gcd(e, phi) == 1) && (e > 1 && e < phi);
}

// Main menu function
int menu() {
    int selection = 0;

    printf("\nMENU\n");
    printf("1. Create encryption & decryption keys\n");
    printf("2. RSA Encryption (Key needed)\n");
    printf("3. RSA Decryption (Key needed)\n");
    printf("4. Help\n");
    printf("5. Exit\n");

    printf("\nSelect: ");
    scanf("%d", &selection);

    return selection;
}

// Main function
int main() {
    printf("\n");
    printf("***************************************************\n");
    printf("*                                                 *\n");
    printf("*       RSA (Encryption/ Decryption) Program      *\n");
    printf("*                 by Fatalbullet                  *\n");
    printf("*       https://github.com/Ayoublee/RSA-algo      *\n");
    printf("*                                                 *\n");
    printf("***************************************************\n");

    int selection = 0;
    int p = 0; // Prime 1
    int q = 0; // Prime 2
    int n = 0; // n = p * q;
    int e = 0; // Public exponent
    int d = 0; // d = (1/e) mod (phi)
    int phi = 0; // (p - 1) * (q - 1)

    while(selection != 5) {
        selection = menu();

        if(selection == 1) {
            do {
                printf("\nEnter the first prime number: ");
                scanf("%d", &p);
                if(!check_prime(p)) {
                    printf("The number entered is not a prime number. Try again.\n");
                }
            } while(!check_prime(p));

            do {
                printf("\nEnter the second prime number: ");
                scanf("%d", &q);
                if(!check_prime(q)) {
                    printf("The number entered is not a prime number. Try again.\n");
                }
            } while(!check_prime(q));

            n = p * q;
            phi = (p - 1) * (q - 1);

            do {
                printf("\nEnter a value for public exponent 'e': ");
                scanf("%d", &e);
                if(!check_e(e, phi)) {
                    printf("The 'e' value is not compatible. Try again.\n");
                }
            } while(!check_e(e, phi));

            d = mod_inverse(e, phi);

            printf("\n*************************************\n");
            printf("  Public encryption key: \n");
            printf("    e = %d\n", e);
            printf("    n = %d\n", n);
            printf("\n");
            printf("  Private decryption key: \n");
            printf("    d = %d\n", d);
            printf("    n = %d", n);
            printf("\n*************************************\n");
        } else if(selection == 2) {
            printf("\nEnter the public encryption key: \n");
            printf("Enter 'e' value: ");
            scanf("%d", &e);
            printf("Enter 'n' value: ");
            scanf("%d", &n);

            encode(e, n);
        } else if(selection == 3) {
            printf("\nEnter the private decryption key: \n");
            printf("Enter 'd' value: ");
            scanf("%d", &d);
            printf("Enter 'n' value: ");
            scanf("%d", &n);

            decode(d, n);
        } else if(selection == 4) {
            printf("***************************************************\n");
            printf("Instructions:\n\n");
            printf("   Create encryption & decryption keys:\n");
            printf("   1. Enter two distinctive (greater than 10)\n");
            printf("      primes p and q\n");
            printf("   2. Enter an integer e so that gcd(e,phi(n)) = 1\n");
            printf("   3. Use the Public encryption key to encrypt\n");
            printf("      messages.\n");
            printf("   4. Use the Private decryption key to decrypt\n");
            printf("      messages.\n\n");
            printf("   RSA Encryption:\n");
            printf("   1. Enter e and n values in the Public encryption\n");
            printf("      key\n");
            printf("   2. Enter message to be encrypted (< 100000 chars)\n");
            printf("   3. Record the ciphered text info\n");
            printf("   4. Record the number of characters in the text\n\n");
            printf("   RSA Decryption:\n");
            printf("   1. Enter d and n values in the Private decryption\n");
            printf("      key\n");
            printf("   2. Enter the number of chars in the encrypted text\n");
            printf("   3. Enter the ciphered text (Base64) in one string\n\n");
            printf("***************************************************\n");
        } else if(selection == 5) {
            break;
        }
    }

    return 0;
}
