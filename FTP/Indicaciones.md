# Guía Rápida de Pruebas - Servidor FTP

Este documento resume de forma breve y precisa cómo probar el servidor FTP tanto en la misma computadora como desde otra en la red local.

## 1. Inicio del Servidor

1.  Abre la carpeta del proyecto.
2.  Haz doble clic en **`start_server.bat`**.
3.  Se abrirá una consola negra. **No la cierres**.
4.  Anota la **IP** que aparece en la consola (ej. `192.168.1.50`) y el puerto (`2121`).

## 2. Prueba en la Misma Laptop (Localhost)

Esta prueba verifica que el servidor funciona correctamente sin depender de la red.

1.  Abre una **nueva consola** (CMD o PowerShell).
2.  Escribe el siguiente comando y presiona Enter:
    ```cmd
    ftp localhost 2121
    ```
3.  Cuando pida usuario, escribe: `admin`
4.  Cuando pida contraseña, escribe: `admin123`
5.  Si ves el mensaje **"230 Login successful"**, el servidor funciona.

## 3. Prueba desde Otra Laptop (Red Local / LAN)

Esta prueba verifica la conexión entre computadoras. **Requisito:** Ambas deben estar conectadas al mismo Wi-Fi o cable.

1.  Ve a la **otra computadora**.
2.  Abre una consola (CMD o PowerShell).
3.  Prueba conexión con ping (opcional pero recomendado):
    ```cmd
    ping <IP_DEL_SERVIDOR>
    ```
    *(Reemplaza `<IP_DEL_SERVIDOR>` por la IP anotada en el paso 1, ej: `192.168.1.50`)*.
4.  Conéctate al FTP:
    ```cmd
    ftp <IP_DEL_SERVIDOR> 2121
    ```
5.  Usa las credenciales:
    *   **User**: `admin`
    *   **Pass**: `admin123`

> **Nota:** Si no conecta, **desactiva temporalmente el Firewall** en la computadora que tiene el servidor o permite el puerto `2121`.

## 4. Cómo Pasar Archivos (Ejemplos)

Una vez conectado (cuando ves `ftp>`), usa estos comandos.

### Preparación (Importante)
Antes de enviar archivos que no sean texto (como imágenes, PDF, zip), activa el modo binario:
```ftp
binary
```

### Ver archivos del servidor
```ftp
ls
```

### Descargar archivo del Servidor a tu PC
Comando: `get <nombre_archivo>`
```ftp
get documento.txt
```
*El archivo se guardará en la carpeta donde abriste la consola.*

### Subir archivo de tu PC al Servidor
1.  Asegúrate de saber en qué carpeta estás en tu PC local. Puedes cambiar con `lcd`:
    ```ftp
    lcd C:\Users\TuUsuario\Documents
    ```
2.  Sube el archivo con `put`:
    ```ftp
    put mi_tarea.pdf
    ```

### Cerrar conexión
```ftp
bye
```

## Resumen de Credenciales
*   **Admin (Lectura y Escritura):**
    *   User: `admin`
    *   Pass: `admin123`
*   **Usuario (Solo Lectura):**
    *   User: `user1`
    *   Pass: `pass123`
