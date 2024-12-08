#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void ejecutar_comando(const char *cmd) {
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error ejecutando: %s\n", cmd);
    }
}

void particionar_usb(const char *dispositivo) {
    char comando[256];

    // Crear tabla de particiones GPT
    snprintf(comando, sizeof(comando), "parted %s mklabel gpt", dispositivo);
    ejecutar_comando(comando);

    // Crear particiones
    snprintf(comando, sizeof(comando), "parted %s mkpart primary fat32 1MiB 100MiB", dispositivo);
    ejecutar_comando(comando);
    snprintf(comando, sizeof(comando), "parted %s mkpart primary ext4 100MiB 50%%", dispositivo);
    ejecutar_comando(comando);
    snprintf(comando, sizeof(comando), "parted %s mkpart primary ext4 50%% 100%%", dispositivo);
    ejecutar_comando(comando);

    // Formatear partición EFI
    snprintf(comando, sizeof(comando), "mkfs.fat -F32 %s1", dispositivo);
    ejecutar_comando(comando);

    printf("Particionado completado en el dispositivo: %s\n", dispositivo);
}

void escribir_iso(const char *iso_path, const char *particion) {
    char comando[256];

    snprintf(comando, sizeof(comando), "dd if=%s of=%s bs=4M status=progress oflag=sync", iso_path, particion);
    ejecutar_comando(comando);

    printf("ISO escrita en la partición: %s\n", particion);
}

void configurar_grub(const char *dispositivo) {
    char comando[256];

    snprintf(comando, sizeof(comando), "mount %s1 /mnt", dispositivo);
    ejecutar_comando(comando);

    ejecutar_comando("grub-install --target=x86_64-efi --efi-directory=/mnt --boot-directory=/mnt/boot --removable");

    ejecutar_comando("umount /mnt");

    printf("GRUB instalado correctamente en: %s\n", dispositivo);
}

void on_particionar_usb(GtkWidget *widget, gpointer data) {
    const char *dispositivo = (const char *)data;
    particionar_usb(dispositivo);
}

void on_escribir_iso(GtkWidget *widget, gpointer data) {
    const char **params = (const char **)data;
    escribir_iso(params[0], params[1]);
}

void on_configurar_grub(GtkWidget *widget, gpointer data) {
    const char *dispositivo = (const char *)data;
    configurar_grub(dispositivo);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Creador de USB Multiboot");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *btn_particionar = gtk_button_new_with_label("Particionar USB");
    GtkWidget *btn_escribir_iso1 = gtk_button_new_with_label("Escribir Primera ISO");
    GtkWidget *btn_escribir_iso2 = gtk_button_new_with_label("Escribir Segunda ISO");
    GtkWidget *btn_configurar_grub = gtk_button_new_with_label("Instalar GRUB");

    const char *dispositivo = "/dev/sdb"; // Modifica según tu dispositivo USB
    const char *iso1_path = "path/to/iso1.iso"; // Cambiar por la ruta real
    const char *iso2_path = "path/to/iso2.iso"; // Cambiar por la ruta real
    const char *particion1 = "/dev/sdb2";
    const char *particion2 = "/dev/sdb3";

    g_signal_connect(btn_particionar, "clicked", G_CALLBACK(on_particionar_usb), (gpointer)dispositivo);
    g_signal_connect(btn_escribir_iso1, "clicked", G_CALLBACK(on_escribir_iso), (gpointer)(const char *[]){iso1_path, particion1});
    g_signal_connect(btn_escribir_iso2, "clicked", G_CALLBACK(on_escribir_iso), (gpointer)(const char *[]){iso2_path, particion2});
    g_signal_connect(btn_configurar_grub, "clicked", G_CALLBACK(on_configurar_grub), (gpointer)dispositivo);

    gtk_box_pack_start(GTK_BOX(vbox), btn_particionar, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), btn_escribir_iso1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), btn_escribir_iso2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), btn_configurar_grub, FALSE, FALSE, 5);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
