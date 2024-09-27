#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

// Base64 encoding
char* base64_encode(const unsigned char *input, size_t length) {
    return g_base64_encode(input, length);
}

// Base64 decoding
gchar* base64_decode(const gchar *input, gsize *out_length) {
    return g_base64_decode(input, out_length);
}


// Global variables for RSA keys
int e, d, n, p, q;

// Function prototypes for RSA logic
void generate_keys(int p, int q);
void encrypt_text(const char *text, int e, int n);
void decrypt_text(const char *cipher_text, int d, int n);
void encrypt_file(const char *file_path, int e, int n);
void decrypt_file(const char *file_path, int d, int n);

// Button click handlers
void on_create_keys_clicked(GtkWidget *widget, gpointer data);
void on_encrypt_text_clicked(GtkWidget *widget, gpointer data);
void on_decrypt_text_clicked(GtkWidget *widget, gpointer data);
void on_encrypt_file_clicked(GtkWidget *widget, gpointer data);
void on_decrypt_file_clicked(GtkWidget *widget, gpointer data);

// Utility function for showing message dialogs
void show_message_dialog(const char *message);

// RSA logic functions
int gcd(int a, int b);
int mod_inverse(int a, int m);
int modular_exponentiation(int base, int exp, int mod);

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button_create_keys;
    GtkWidget *button_encrypt_text;
    GtkWidget *button_decrypt_text;
    GtkWidget *button_encrypt_file;
    GtkWidget *button_decrypt_file;

    gtk_init(&argc, &argv);

    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "RSA GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

 // Create and pack the label
    label = gtk_label_new("***************************************************\n"
                          "*                                                 *\n"
                          "*       RSA (Encryption/ Decryption) Program      *\n"
                          "*                 by Fatalbullet                  *\n"
                          "*       https://github.com/Ayoublee/RSA-algo      *\n"
                          "*                                                 *\n"
                          "***************************************************");
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    // Create buttons
    button_create_keys = gtk_button_new_with_label("Create Keys");
    g_signal_connect(button_create_keys, "clicked", G_CALLBACK(on_create_keys_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_create_keys, TRUE, TRUE, 0);

    button_encrypt_text = gtk_button_new_with_label("Encrypt Text");
    g_signal_connect(button_encrypt_text, "clicked", G_CALLBACK(on_encrypt_text_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_encrypt_text, TRUE, TRUE, 0);

    button_decrypt_text = gtk_button_new_with_label("Decrypt Text");
    g_signal_connect(button_decrypt_text, "clicked", G_CALLBACK(on_decrypt_text_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_decrypt_text, TRUE, TRUE, 0);

    button_encrypt_file = gtk_button_new_with_label("Encrypt File");
    g_signal_connect(button_encrypt_file, "clicked", G_CALLBACK(on_encrypt_file_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_encrypt_file, TRUE, TRUE, 0);

    button_decrypt_file = gtk_button_new_with_label("Decrypt File");
    g_signal_connect(button_decrypt_file, "clicked", G_CALLBACK(on_decrypt_file_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_decrypt_file, TRUE, TRUE, 0);

    // Show everything
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

// Handler for "Create Keys" button
void on_create_keys_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *entry_p, *entry_q;
    dialog = gtk_dialog_new_with_buttons("Create RSA Keys", NULL, GTK_DIALOG_MODAL, ("OK"), GTK_RESPONSE_OK, ("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry_p = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_p), "Enter prime number p");
    gtk_box_pack_start(GTK_BOX(content_area), entry_p, TRUE, TRUE, 0);

    entry_q = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_q), "Enter prime number q");
    gtk_box_pack_start(GTK_BOX(content_area), entry_q, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        p = atoi(gtk_entry_get_text(GTK_ENTRY(entry_p)));
        q = atoi(gtk_entry_get_text(GTK_ENTRY(entry_q)));
        generate_keys(p, q);
        char message[256];
        snprintf(message, sizeof(message), "Keys generated!\nPublic key (e, n): (%d, %d)\nPrivate key (d, n): (%d, %d)", e, n, d, n);
        show_message_dialog(message);
    }
    gtk_widget_destroy(dialog);
}

// Handler for "Encrypt Text" button
// Handler for "Encrypt Text" button
void on_encrypt_text_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry_text;
    GtkWidget *entry_e;
    GtkWidget *entry_n;
    GtkWidget *box;
    
    dialog = gtk_dialog_new_with_buttons("Encrypt Text", NULL, GTK_DIALOG_MODAL, 
        ("Encrypt"), GTK_RESPONSE_OK, 
        ("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Text to Encrypt:"), FALSE, FALSE, 0);
    entry_text = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_text, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Public Key e:"), FALSE, FALSE, 0);
    entry_e = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_e, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Modulus n:"), FALSE, FALSE, 0);
    entry_n = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_n, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text_to_encrypt = gtk_entry_get_text(GTK_ENTRY(entry_text));
        int e = atoi(gtk_entry_get_text(GTK_ENTRY(entry_e)));
        int n = atoi(gtk_entry_get_text(GTK_ENTRY(entry_n)));
        encrypt_text(text_to_encrypt, e, n);
    }
    gtk_widget_destroy(dialog);
}

// Handler for "Decrypt Text" button
// Handler for "Decrypt Text" button
void on_decrypt_text_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry_cipher_text;
    GtkWidget *entry_d;
    GtkWidget *entry_n;
    GtkWidget *box;
    
    dialog = gtk_dialog_new_with_buttons("Decrypt Text", NULL, GTK_DIALOG_MODAL, 
        ("Decrypt"), GTK_RESPONSE_OK, 
        ("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Cipher Text to Decrypt:"), FALSE, FALSE, 0);
    entry_cipher_text = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_cipher_text, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Private Key d:"), FALSE, FALSE, 0);
    entry_d = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_d, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Modulus n:"), FALSE, FALSE, 0);
    entry_n = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_n, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *cipher_text = gtk_entry_get_text(GTK_ENTRY(entry_cipher_text));
        int d = atoi(gtk_entry_get_text(GTK_ENTRY(entry_d)));
        int n = atoi(gtk_entry_get_text(GTK_ENTRY(entry_n)));
        decrypt_text(cipher_text, d, n);
    }
    gtk_widget_destroy(dialog);
}
// Handler for "Encrypt File" button
void on_encrypt_file_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *entry_file_path;
    GtkWidget *entry_e;
    GtkWidget *entry_n;
    GtkWidget *box;

    dialog = gtk_dialog_new_with_buttons("Encrypt File", NULL, GTK_DIALOG_MODAL,
        ("Encrypt"), GTK_RESPONSE_OK,
        ("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("File to Encrypt:"), FALSE, FALSE, 0);
    entry_file_path = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_file_path, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Public Key e:"), FALSE, FALSE, 0);
    entry_e = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_e, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Modulus n:"), FALSE, FALSE, 0);
    entry_n = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_n, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *file_path = gtk_entry_get_text(GTK_ENTRY(entry_file_path));
        int e = atoi(gtk_entry_get_text(GTK_ENTRY(entry_e)));
        int n = atoi(gtk_entry_get_text(GTK_ENTRY(entry_n)));
        encrypt_file(file_path, e, n);
    }
    gtk_widget_destroy(dialog);
}

// Handler for "Decrypt File" button
void on_decrypt_file_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *entry_file_path;
    GtkWidget *entry_d;
    GtkWidget *entry_n;
    GtkWidget *box;

    dialog = gtk_dialog_new_with_buttons("Decrypt File", NULL, GTK_DIALOG_MODAL,
        ("Decrypt"), GTK_RESPONSE_OK,
        ("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("File to Decrypt:"), FALSE, FALSE, 0);
    entry_file_path = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_file_path, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Private Key d:"), FALSE, FALSE, 0);
    entry_d = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_d, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Enter Modulus n:"), FALSE, FALSE, 0);
    entry_n = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_n, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *file_path = gtk_entry_get_text(GTK_ENTRY(entry_file_path));
        int d = atoi(gtk_entry_get_text(GTK_ENTRY(entry_d)));
        int n = atoi(gtk_entry_get_text(GTK_ENTRY(entry_n)));
        decrypt_file(file_path, d, n);
    }
    gtk_widget_destroy(dialog);
}

void show_message_dialog(const char *message) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// RSA logic functions
void generate_keys(int p, int q) {
    n = p * q;
    int phi_n = (p - 1) * (q - 1);

    // Simple public key selection
    e = 3;
    while (e < phi_n && gcd(e, phi_n) != 1) {
        e += 2;
    }

    // Compute private key 'd' as the modular inverse of 'e'
    d = mod_inverse(e, phi_n);

    printf("Public Key: (e=%d, n=%d)\n", e, n);
    printf("Private Key: (d=%d, n=%d)\n", d, n);
}

void encrypt_text(const char *text, int e, int n) {
    char encrypted_text[4096];
    int length = strlen(text);
    
    // Encrypt each character
    for (int i = 0; i < length; i++) {
        encrypted_text[i] = modular_exponentiation(text[i], e, n);
    }
    
    encrypted_text[length] = '\0';  // Null-terminate the string
    
    // Base64 encode the result
    char *base64_encoded = base64_encode((unsigned char *)encrypted_text, length);
    
    printf("Encrypted text (Base64 encoded): %s\n", base64_encoded);
    show_message_dialog(base64_encoded);  // Show the result in a dialog box
    
    g_free(base64_encoded);  // Free the allocated memory
}

void decrypt_text(const char *cipher_text, int d, int n) {
    gsize decoded_length;
    
    // Base64 decode the input
    gchar *base64_decoded = base64_decode(cipher_text, &decoded_length);
    
    char decrypted_text[4096];
    
    // Decrypt each character
    for (int i = 0; i < decoded_length; i++) {
        decrypted_text[i] = modular_exponentiation(base64_decoded[i], d, n);
    }
    
    decrypted_text[decoded_length] = '\0';  // Null-terminate the string
    
    printf("Decrypted text: %s\n", decrypted_text);
    show_message_dialog(decrypted_text);  // Show the result in a dialog box
    
    g_free(base64_decoded);  // Free the allocated memory
}
void encrypt_file(const char *file_path, int e, int n) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        show_message_dialog("Failed to open file for reading.");
        return;
    }

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer and read file content
    unsigned char *buffer = malloc(file_size);
    if (!buffer) {
        fclose(file);
        show_message_dialog("Failed to allocate memory.");
        return;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    // Encrypt the content
    unsigned char *encrypted = malloc(file_size);
    if (!encrypted) {
        free(buffer);
        show_message_dialog("Failed to allocate memory.");
        return;
    }

    for (long i = 0; i < file_size; i++) {
        encrypted[i] = modular_exponentiation(buffer[i], e, n);
    }

    // Base64 encode the encrypted content
    char *base64_encoded = base64_encode(encrypted, file_size);

    // Write the Base64 encoded content to a new file
    char encrypted_file_path[256];
    snprintf(encrypted_file_path, sizeof(encrypted_file_path), "%s.enc", file_path);
    file = fopen(encrypted_file_path, "w");
    if (!file) {
        free(buffer);
        free(encrypted);
        g_free(base64_encoded);
        show_message_dialog("Failed to open file for writing.");
        return;
    }

    fprintf(file, "%s", base64_encoded);
    fclose(file);

    // Clean up
    free(buffer);
    free(encrypted);
    g_free(base64_encoded);

    show_message_dialog("File encrypted successfully!");
}


void decrypt_file(const char *file_path, int d, int n) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        show_message_dialog("Failed to open file for reading.");
        return;
    }

    // Read the Base64 encoded content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *base64_encoded = malloc(file_size + 1);
    if (!base64_encoded) {
        fclose(file);
        show_message_dialog("Failed to allocate memory.");
        return;
    }

    fread(base64_encoded, 1, file_size, file);
    base64_encoded[file_size] = '\0';  // Null-terminate the string
    fclose(file);

    // Base64 decode the content
    gsize decoded_length;
    unsigned char *base64_decoded = base64_decode(base64_encoded, &decoded_length);

    // Decrypt the content
    unsigned char *decrypted = malloc(decoded_length);
    if (!decrypted) {
        free(base64_encoded);
        show_message_dialog("Failed to allocate memory.");
        return;
    }

    for (gsize i = 0; i < decoded_length; i++) {
        decrypted[i] = modular_exponentiation(base64_decoded[i], d, n);
    }

    // Write the decrypted content to a new file
    char decrypted_file_path[256];
    snprintf(decrypted_file_path, sizeof(decrypted_file_path), "%s.dec", file_path);
    file = fopen(decrypted_file_path, "wb");
    if (!file) {
        free(base64_encoded);
        free(base64_decoded);
        free(decrypted);
        show_message_dialog("Failed to open file for writing.");
        return;
    }

    fwrite(decrypted, 1, decoded_length, file);
    fclose(file);

    // Clean up
    free(base64_encoded);
    free(base64_decoded);
    free(decrypted);

    show_message_dialog("File decrypted successfully!");
}




// Helper functions for RSA
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int mod_inverse(int a, int m) {
    int m0 = m;
    int x0 = 0;
    int x1 = 1;

    if (m == 1) return 0;

    while (a > 1) {
        int q = a / m;
        int t = m;

        m = a % m;
        a = t;
        t = x0;

        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) x1 += m0;

    return x1;
}

int modular_exponentiation(int base, int exp, int mod) {
    int result = 1;
    base = base % mod;

    while (exp > 0) {
        if (exp % 2 == 1) result = (result * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }

    return result;
}
