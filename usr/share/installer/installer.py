import subprocess
import os
from tkinter import Tk, Label, Button, filedialog, messagebox, StringVar
from tkinter.ttk import Progressbar

def ejecutar_comando(cmd, progress_bar=None):
    """Ejecuta un comando en el sistema con una barra de progreso opcional."""
    try:
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        while process.poll() is None:
            if progress_bar:
                progress_bar.step(5)
        if process.returncode != 0:
            error = process.stderr.read().decode()
            raise RuntimeError(f"Error: {error}")
    except Exception as e:
        raise RuntimeError(f"Error ejecutando '{cmd}': {e}")

def listar_dispositivos_usb():
    """Obtiene una lista de dispositivos USB disponibles."""
    try:
        salida = subprocess.check_output("lsblk -lp | grep 'disk $' | awk '{print $1}'", shell=True)
        dispositivos = salida.decode().strip().split('\n')
        return dispositivos
    except subprocess.CalledProcessError:
        return []

def seleccionar_iso():
    """Permite seleccionar un archivo ISO mediante un cuadro de diálogo."""
    return filedialog.askopenfilename(filetypes=[("Archivos ISO", "*.iso")])

def particionar_usb(dispositivo, progress_bar):
    """Particiona la USB en tres particiones (EFI, ISO 1 e ISO 2)."""
    try:
        comandos = [
            f"parted {dispositivo} mklabel gpt",
            f"parted {dispositivo} mkpart primary fat32 1MiB 100MiB",
            f"parted {dispositivo} mkpart primary ext4 100MiB 50%",
            f"parted {dispositivo} mkpart primary ext4 50% 100%",
            f"mkfs.fat -F32 {dispositivo}1"
        ]
        for cmd in comandos:
            ejecutar_comando(cmd, progress_bar)
        messagebox.showinfo("Éxito", "¡Particionado completado!")
    except Exception as e:
        messagebox.showerror("Error", f"Fallo en el particionado: {e}")

def escribir_iso(iso_path, particion, progress_bar):
    """Escribe una ISO en una partición específica."""
    try:
        ejecutar_comando(f"sudo dd if={iso_path} of={particion} bs=4M status=progress oflag=sync", progress_bar)
        messagebox.showinfo("Éxito", f"¡ISO escrita en {particion}!")
    except Exception as e:
        messagebox.showerror("Error", f"No se pudo escribir la ISO: {e}")

def configurar_grub(dispositivo, progress_bar):
    """Configura GRUB en la partición EFI."""
    try:
        comandos = [
            f"mount {dispositivo}1 /mnt",
            "grub-install --target=x86_64-efi --efi-directory=/mnt --boot-directory=/mnt/boot --removable",
            "umount /mnt"
        ]
        for cmd in comandos:
            ejecutar_comando(cmd, progress_bar)
        messagebox.showinfo("Éxito", "¡GRUB instalado correctamente!")
    except Exception as e:
        messagebox.showerror("Error", f"No se pudo instalar GRUB: {e}")

def main():
    root = Tk()
    root.title("Creador de USB Multiboot")

    dispositivo_seleccionado = StringVar(value="")

    Label(root, text="1. Seleccione el dispositivo USB:").pack()
    dispositivos_usb = listar_dispositivos_usb()
    if dispositivos_usb:
        dispositivo_seleccionado.set(dispositivos_usb[0])
        Label(root, text=f"Dispositivo detectado: {dispositivos_usb[0]}").pack()
    else:
        messagebox.showerror("Error", "¡No se encontraron dispositivos USB!")
        root.quit()

    Label(root, text="2. Seleccione la primera ISO:").pack()
    ruta_iso1 = Button(root, text="Seleccionar ISO 1", command=lambda: seleccionar_iso()).pack()

    Label(root, text="3. Seleccione la segunda ISO:").pack()
    ruta_iso2 = Button(root, text="Seleccionar ISO 2", command=lambda: seleccionar_iso()).pack()

    progress_bar = Progressbar(root, orient="horizontal", mode="determinate", length=300)
    progress_bar.pack()

    Button(root, text="Particionar USB", command=lambda: particionar_usb(dispositivo_seleccionado.get(), progress_bar)).pack()
    Button(root, text="Escribir primera ISO", command=lambda: escribir_iso(ruta_iso1, dispositivo_seleccionado.get() + "2", progress_bar)).pack()
    Button(root, text="Escribir segunda ISO", command=lambda: escribir_iso(ruta_iso2, dispositivo_seleccionado.get() + "3", progress_bar)).pack()
    Button(root, text="Instalar GRUB", command=lambda: configurar_grub(dispositivo_seleccionado.get(), progress_bar)).pack()

    root.mainloop()

if __name__ == "__main__":
    main()
