# Mini Servidor FTP

## Descripcion

Servidor FTP simple implementado en Python que permite la transferencia de archivos entre computadoras en una red local mediante autenticacion de usuario y contraseña.

## Fundamentos Tecnicos

### Que es FTP

FTP (File Transfer Protocol) es un protocolo de transferencia de archivos que opera en la **Capa 7 (Aplicacion)** del modelo OSI. Utiliza el protocolo **TCP** de la Capa 4 (Transporte) para garantizar la entrega confiable de datos.

### Arquitectura FTP

FTP utiliza dos canales de comunicacion separados:

1. **Canal de Control (Puerto 21 o 2121 en esta implementacion)**
   - Transmite comandos del cliente al servidor
   - Recibe respuestas del servidor
   - Permanece abierto durante toda la sesion

2. **Canal de Datos (Puertos 60000-60100 en modo pasivo)**
   - Transfiere los archivos y listados de directorios
   - Se abre y cierra segun sea necesario

### Modos de Transferencia

- **Modo Pasivo (PASV)**: El servidor abre un puerto aleatorio y espera la conexion del cliente
- Esta implementacion usa modo pasivo para evitar problemas con firewalls

## Estructura del Proyecto

```
FTP/
├── src/
│   ├── server/
│   │   ├── ftp_server.py    # Servidor principal
│   │   └── auth.py          # Sistema de autenticacion
│   ├── config/
│   │   └── config.py        # Gestion de configuracion
│   └── utils/
│       └── logger.py        # Sistema de logs
├── data/
│   └── ftp_files/           # Directorio compartido
├── logs/                    # Archivos de registro
├── config.json              # Configuracion del servidor
├── requirements.txt         # Dependencias de Python
├── start_server.bat         # Script de inicio
└── README.md               # Este archivo
```

## Requisitos Previos

- Python 3.7 o superior
- Conexion a red local (Wi-Fi o Ethernet)
- Acceso a la consola de Windows (cmd o PowerShell)

## Instalacion

1. Asegurate de tener Python instalado:
   ```
   python --version
   ```

2. El script `start_server.bat` instalara automaticamente las dependencias necesarias

## Configuracion

El archivo `config.json` contiene la configuracion del servidor:

```json
{
    "server": {
        "host": "0.0.0.0",           // Escucha en todas las interfaces
        "port": 2121,                 // Puerto del servidor
        "max_connections": 10,        // Conexiones simultaneas maximas
        "max_connections_per_ip": 5   // Conexiones por IP
    },
    "directories": {
        "ftp_root": "data/ftp_files", // Directorio compartido
        "logs": "logs"                // Directorio de logs
    },
    "users": [
        {
            "username": "admin",
            "password": "admin123",
            "permissions": "elradfmwMT"  // Permisos completos
        },
        {
            "username": "user1",
            "password": "pass123",
            "permissions": "elr"         // Solo lectura y navegacion
        }
    ]
}
```

### Permisos de Usuario

- `e` = cambiar directorio (CWD, CDUP)
- `l` = listar archivos (LIST, NLST, STAT, MLSD)
- `r` = descargar archivos (RETR)
- `a` = subir archivos (APPE)
- `d` = eliminar archivos (DELE, RMD)
- `f` = renombrar archivos (RNFR, RNTO)
- `m` = crear directorios (MKD)
- `w` = escribir archivos (STOR, STOU)
- `M` = cambiar permisos (SITE CHMOD)
- `T` = cambiar fecha de modificacion

## Uso

### Iniciar el Servidor

1. Ejecuta el script de inicio:
   ```
   start_server.bat
   ```

2. El servidor mostrara:
   - IP local de tu computadora
   - Puerto en el que esta escuchando
   - Usuarios configurados
   - Instrucciones de conexion

### Conectarse desde la Misma Computadora

1. Abre otra consola (cmd o PowerShell)
2. Ejecuta:
   ```
   ftp localhost 2121
   ```
3. Ingresa usuario y contraseña cuando se solicite

### Conectarse desde Otra Computadora en la Red Local

#### Paso 1: Obtener la IP del Servidor

En la computadora que ejecuta el servidor, la IP se muestra al iniciar. Tambien puedes obtenerla con:
```
ipconfig
```
Busca la linea "Direccion IPv4" en tu adaptador de red activo.

#### Paso 2: Verificar Conectividad

Desde la computadora cliente, verifica que puedes alcanzar el servidor:
```
ping <IP_DEL_SERVIDOR>
```

#### Paso 3: Conectarse al Servidor FTP

En la consola de la computadora cliente:
```
ftp <IP_DEL_SERVIDOR> 2121
```

Ejemplo:
```
ftp 192.168.1.100 2121
```

#### Paso 4: Autenticarse

Ingresa un usuario y contraseña configurados en `config.json`:
```
User: admin
Password: admin123
```

### Comandos FTP Basicos

Una vez conectado, puedes usar estos comandos en la consola:

#### Navegacion
```
ls                  # Listar archivos en el servidor
dir                 # Listar archivos (detallado)
pwd                 # Mostrar directorio actual
cd <directorio>     # Cambiar directorio
cd ..               # Subir un nivel
```

#### Transferencia de Archivos

```
get <archivo>       # Descargar archivo del servidor
put <archivo>       # Subir archivo al servidor
mget *.txt          # Descargar multiples archivos
mput *.txt          # Subir multiples archivos
```

#### Configuracion Local

```
lcd <directorio>    # Cambiar directorio local
!dir                # Ver archivos locales (Windows)
!cd                 # Ver directorio local actual
```

#### Otras Operaciones

```
delete <archivo>    # Eliminar archivo
rmdir <directorio>  # Eliminar directorio
mkdir <directorio>  # Crear directorio
rename <old> <new>  # Renombrar archivo
```

#### Gestion de Sesion

```
binary              # Modo binario (recomendado para archivos no-texto)
ascii               # Modo ASCII (para archivos de texto)
status              # Ver estado de la conexion
bye                 # Cerrar sesion y salir
quit                # Cerrar sesion y salir
```

## Ejemplo de Sesion Completa

```
C:\> ftp 192.168.1.100 2121
Conectado a 192.168.1.100.
220 pyftpdlib ready.
User (192.168.1.100:(none)): admin
331 Username ok, send password.
Password: ********
230 Login successful.
ftp> binary
200 Type set to: Binary.
ftp> ls
200 Active data connection established.
125 Data connection already open. Transfer starting.
archivo1.txt
documento.pdf
imagen.jpg
226 Transfer complete.
ftp> get documento.pdf
200 Active data connection established.
125 Data connection already open. Transfer starting.
226 Transfer complete.
ftp> lcd C:\Descargas
Local directory now C:\Descargas
ftp> put mi_archivo.txt
200 Active data connection established.
125 Data connection already open. Transfer starting.
226 Transfer complete.
ftp> bye
221 Goodbye.
```

## Solucion de Problemas

### El servidor no inicia

1. Verifica que Python este instalado
2. Asegurate de que el puerto 2121 no este en uso:
   ```
   netstat -an | findstr 2121
   ```
3. Revisa los logs en la carpeta `logs/`

### No puedo conectarme desde otra computadora

1. Verifica que ambas computadoras esten en la misma red
2. Desactiva temporalmente el firewall de Windows para probar
3. Si usas firewall, permite el puerto 2121 y el rango 60000-60100:
   ```
   netsh advfirewall firewall add rule name="FTP Server" dir=in action=allow protocol=TCP localport=2121
   netsh advfirewall firewall add rule name="FTP Passive" dir=in action=allow protocol=TCP localport=60000-60100
   ```

### Los archivos no se transfieren correctamente

1. Usa el modo `binary` antes de transferir archivos
2. Verifica que tengas permisos suficientes (usuario con permiso `r` para descargar, `w` para subir)
3. Revisa que el directorio local tenga permisos de escritura

### Error de autenticacion

1. Verifica que el usuario y contraseña esten correctamente configurados en `config.json`
2. Reinicia el servidor despues de cambiar la configuracion

## Logs

Todos los eventos del servidor se registran en:
- Consola (salida en tiempo real)
- Archivo de log diario en `logs/ftp_server_YYYYMMDD.log`

Los logs incluyen:
- Conexiones y desconexiones de clientes
- Autenticaciones exitosas y fallidas
- Transferencias de archivos
- Errores del sistema

## Seguridad

ADVERTENCIA: Esta es una implementacion educativa. Para uso en produccion considera:

1. Usar FTPS (FTP sobre TLS/SSL) en lugar de FTP plano
2. Implementar contraseñas fuertes
3. Limitar los permisos de usuarios
4. Configurar reglas de firewall apropiadas
5. Usar redes privadas virtuales (VPN) para acceso remoto

## Notas Tecnicas

- El servidor usa `pyftpdlib`, una implementacion madura y completa del protocolo FTP
- TCP garantiza la entrega ordenada y sin errores de los datos
- El modo pasivo evita problemas comun con NAT y firewalls
- Los puertos pasivos (60000-60100) pueden configurarse segun tus necesidades

## Licencia

Este proyecto es de codigo abierto para fines educativos.
