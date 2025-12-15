# 🎮 UDP-PONG: Juego Multijugador sobre Protocolo UDP

**Proyecto Final - Redes de Computadoras**

Implementación de un juego Pong multijugador que demuestra el diseño e implementación de un **protocolo de capa de aplicación (Capa 7)** sobre **UDP**, con estadísticas de red en tiempo real.

---

## 📋 Tabla de Contenidos

1. [Descripción General](#-descripción-general)
2. [Estructura del Proyecto](#-estructura-del-proyecto)
3. [Compilación y Ejecución](#-compilación-y-ejecución)
4. [Protocolo UDP-PONG](#-protocolo-udp-pong)
5. [Arquitectura del Sistema](#-arquitectura-del-sistema)
6. [División de Presentación](#-división-de-presentación-4-personas)

---

## 🎯 Descripción General

Este proyecto implementa un juego Pong clásico para 2 jugadores utilizando el protocolo **UDP (User Datagram Protocol)** como capa de transporte, con un protocolo personalizado de aplicación diseñado específicamente para juegos en tiempo real.

### Características Principales

✅ **Protocolo UDP personalizado** con mensajes binarios eficientes (26-40 bytes)  
✅ **Juego funcional** con física de colisiones y sistema de puntuación  
✅ **Estadísticas en tiempo real** (RTT, pérdida de paquetes, throughput)  
✅ **Interfaz visual** con ncurses (terminal)  
✅ **Arquitectura cliente-servidor** escalable  
✅ **60 FPS** de actualización constante  

### ¿Por qué UDP?

UDP es ideal para aplicaciones en tiempo real como juegos porque:
- **Baja latencia**: No espera confirmaciones (ACKs)
- **Velocidad > Confiabilidad**: Es mejor perder un paquete que esperar retransmisión
- **Overhead mínimo**: Menos bytes de cabecera que TCP
- **Datos obsoletos**: Si un paquete llega tarde, ya no importa (el juego avanzó)

---

## 📁 Estructura del Proyecto

```
/home/fernando/universidad/redes/
├── src/                    # Código fuente
│   ├── pong_server.c      # Servidor del juego
│   ├── pong_client.c      # Cliente del juego
│   ├── utils.c            # Funciones utilitarias
│   └── stats.c            # Sistema de estadísticas
├── include/               # Archivos de cabecera
│   ├── protocol.h         # Definición del protocolo UDP
│   ├── utils.h            # Utilidades
│   └── stats.h            # Estadísticas
├── bin/                   # Binarios compilados
│   ├── pong_server        # Ejecutable del servidor
│   └── pong_client        # Ejecutable del cliente
├── build/                 # Archivos objeto (.o)
├── docs/                  # Documentación adicional
├── Makefile              # Sistema de compilación
└── README.md             # Este archivo
```

### Archivos Antiguos (Referencia)
- `client.c` / `server.c`: Implementación UDP simple original
- `client` / `server`: Binarios de la versión simple

---

## 🔨 Compilación y Ejecución

### Requisitos del Sistema

#### Linux (Nativo)

```bash
# Ubuntu/Debian
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev

# RedHat/CentOS/Fedora
sudo yum install gcc ncurses-devel
```

#### Windows (WSL - Recomendado)

**Opción 1: WSL2 (Recomendado)**

1. **Instalar WSL2:**
```powershell
# En PowerShell como Administrador
wsl --install
```

2. **Instalar Ubuntu desde Microsoft Store**

3. **Dentro de WSL, instalar dependencias:**
```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```

4. **Navegar al proyecto:**
```bash
cd /mnt/c/Users/TuUsuario/ruta/al/proyecto
```

**Opción 2: Windows Nativo (MinGW)**

⚠️ **Nota:** ncurses no está disponible nativamente en Windows. Se requiere PDCurses o compilar sin interfaz gráfica.

1. **Instalar MinGW-w64:**
   - Descargar de: https://www.mingw-w64.org/
   - O usar MSYS2: https://www.msys2.org/

2. **Instalar PDCurses (alternativa a ncurses):**
```bash
# En MSYS2
pacman -S mingw-w64-x86_64-pdcurses
```

3. **Modificar Makefile:**
```makefile
# Cambiar -lncurses por -lpdcurses
LIBS = -lpdcurses -lm
```

**Recomendación:** Usar WSL2 para compatibilidad completa.

---

### Compilar

#### Linux / WSL
```bash
make clean    # Limpiar archivos anteriores
make all      # Compilar servidor y cliente
```

#### Windows (MinGW/MSYS2)
```bash
mingw32-make clean
mingw32-make all
```

---

### Ejecutar

#### Linux / WSL

**Terminal 1 - Servidor:**
```bash
bin/pong_server
```

**Terminal 2 - Cliente 1:**
```bash
bin/pong_client
# Ingresa tu nombre: Player1
```

**Terminal 3 - Cliente 2:**
```bash
bin/pong_client
# Ingresa tu nombre: Player2
```

**Controles:**
- `W` = Mover paleta ARRIBA
- `S` = Mover paleta ABAJO
- `Q` = SALIR

#### Windows (WSL)

Mismo procedimiento que Linux, ejecutar dentro de WSL:
```bash
bin/pong_server  # Terminal WSL 1
bin/pong_client  # Terminal WSL 2 y 3
```

**Nota:** Abrir múltiples terminales WSL:
- Windows Terminal: `Ctrl+Shift+T` para nueva pestaña
- O abrir múltiples ventanas de Ubuntu desde el menú inicio

#### Windows (Nativo - MinGW)

Si compilaste con PDCurses:
```cmd
bin\pong_server.exe  # CMD 1
bin\pong_client.exe  # CMD 2 y 3
```

---

## 🌐 Protocolo UDP-PONG

### Diseño del Protocolo

El protocolo UDP-PONG utiliza **mensajes binarios estructurados** para eficiencia máxima.

#### Mensaje Cliente → Servidor (26 bytes)

```c
struct client_message {
    uint8_t type;              // Tipo: JOIN(1), INPUT(2), LEAVE(4)
    uint32_t timestamp;        // Timestamp en milisegundos
    uint8_t player_id;         // ID del jugador (1 o 2)
    int8_t action;             // -1=ABAJO, 0=QUIETO, 1=ARRIBA
    char player_name[16];      // Nombre del jugador
} __attribute__((packed));
```

#### Mensaje Servidor → Cliente (40 bytes)

```c
struct server_message {
    uint8_t type;              // Tipo: STATE(1), STATS(2), ERROR(3)
    uint32_t timestamp;        // Timestamp del servidor
    uint8_t player_id;         // ID asignado al jugador
    
    // Estado del juego
    float paddle1_y;           // Posición paleta 1 (0-100)
    float paddle2_y;           // Posición paleta 2 (0-100)
    float ball_x;              // Posición X pelota (0-100)
    float ball_y;              // Posición Y pelota (0-100)
    uint8_t score1;            // Puntos jugador 1
    uint8_t score2;            // Puntos jugador 2
    
    // Estadísticas de red
    uint16_t rtt_ms;           // Round Trip Time
    uint8_t loss_percent;      // Pérdida de paquetes (%)
    uint32_t packets_sent;     // Total enviados
    uint32_t packets_recv;     // Total recibidos
} __attribute__((packed));
```

### Flujo de Comunicación

```
1. CONEXIÓN
   Cliente → Servidor: JOIN "Player1"
   Servidor → Cliente: STATE (con player_id=1)

2. LOOP DE JUEGO (cada 16ms = 60 FPS)
   Cliente → Servidor: INPUT (acción: ARRIBA/ABAJO/QUIETO)
   Servidor: Actualiza física del juego
   Servidor → Todos: STATE (nuevo estado + estadísticas)

3. DESCONEXIÓN
   Cliente → Servidor: LEAVE
   Servidor: Libera recursos del jugador
```

### Ventajas del Diseño

✅ **Eficiente**: Solo 26-40 bytes por paquete  
✅ **Binario**: Más rápido que JSON o texto  
✅ **Estado completo**: Cada paquete tiene todo el estado (no incremental)  
✅ **Tolerante a pérdidas**: Datos viejos se descartan usando timestamps  
✅ **Estadísticas integradas**: Monitoreo sin overhead adicional  

---

## 🏗️ Arquitectura del Sistema

### Componentes Principales

#### 1. **Servidor (`pong_server.c`)**

**Responsabilidades:**
- Gestión de hasta 2 jugadores simultáneos
- Física del juego (movimiento, colisiones, puntuación)
- Broadcast de estado a 60 FPS
- Tracking de estadísticas de red

**Lógica Principal:**
```c
while (1) {
    // 1. Recibir inputs de jugadores (non-blocking)
    recvfrom(sockfd, &msg, ...);
    
    // 2. Actualizar física del juego
    update_physics();  // Paletas, pelota, colisiones
    
    // 3. Enviar estado a todos los jugadores
    broadcast_state(sockfd);
    
    // 4. Mantener 60 FPS
    usleep(16ms);
}
```

#### 2. **Cliente (`pong_client.c`)**

**Responsabilidades:**
- Interfaz visual con ncurses
- Captura de inputs del teclado
- Envío de acciones al servidor
- Renderizado del juego y estadísticas

**Lógica Principal:**
```c
while (running) {
    // 1. Leer tecla presionada
    int ch = getch();
    
    // 2. Enviar INPUT al servidor
    sendto(sockfd, &msg, ...);
    
    // 3. Recibir STATE del servidor
    recvfrom(sockfd, &state, ...);
    
    // 4. Renderizar juego + estadísticas
    render_game();
    render_stats();
}
```

#### 3. **Sistema de Estadísticas (`stats.c`)**

**Métricas Calculadas:**
- **RTT (Round Trip Time)**: Latencia de red
- **Pérdida de paquetes**: Porcentaje de paquetes perdidos
- **Throughput**: Bytes por segundo
- **Contadores**: Paquetes enviados/recibidos

**Algoritmos:**
- **EWMA** (Exponentially Weighted Moving Average) para RTT promedio
- Detección de pérdida por timeout
- Cálculo de throughput en tiempo real

---

## 👥 División de Presentación (4 Personas)

### 📌 PERSONA 1: Introducción y Protocolo UDP

**Tiempo estimado:** 5-7 minutos

**Temas a cubrir:**

1. **Introducción al Proyecto** (2 min)
   - ¿Qué es UDP-PONG?
   - Objetivos del proyecto
   - Demostración rápida del juego funcionando

2. **Protocolo UDP vs TCP** (3 min)
   - Diferencias fundamentales
   - ¿Por qué UDP para juegos?
   - Ventajas y desventajas de UDP

3. **Diseño del Protocolo UDP-PONG** (2 min)
   - Estructura de mensajes binarios
   - Tipos de mensajes (JOIN, INPUT, STATE)
   - Tamaño de paquetes (26-40 bytes)

**Archivos a revisar:**
- `include/protocol.h` (líneas 1-75)
- Sección "Protocolo UDP-PONG" de este README

**Puntos clave:**
- UDP no garantiza entrega ni orden
- Mensajes binarios son más eficientes que texto/JSON
- Estado completo en cada paquete (no incremental)

---

### 📌 PERSONA 2: Arquitectura del Servidor

**Tiempo estimado:** 5-7 minutos

**Temas a cubrir:**

1. **Arquitectura del Servidor** (2 min)
   - Componentes principales
   - Socket UDP non-blocking
   - Gestión de múltiples clientes

2. **Física del Juego** (3 min)
   - Movimiento de paletas
   - Física de la pelota
   - Detección de colisiones
   - Sistema de puntuación

3. **Loop Principal** (2 min)
   - Recepción de inputs
   - Actualización a 60 FPS
   - Broadcast de estado

**Archivos a revisar:**
- `src/pong_server.c` (líneas 95-159: `update_physics()`)
- `src/pong_server.c` (líneas 280-303: loop principal)

**Puntos clave:**
- Servidor mantiene estado autoritativo del juego
- Física simple pero efectiva
- 60 FPS constantes para fluidez

**Código a explicar:**
```c
// Colisión con paleta
if (game.ball_x <= PADDLE_WIDTH + BALL_SIZE / 2) {
    if (fabs(game.ball_y - game.paddle1_y) <= PADDLE_HEIGHT / 2) {
        game.ball_vx = fabs(game.ball_vx);  // Rebote
        // Efecto según punto de impacto
        float hit_pos = (game.ball_y - game.paddle1_y) / (PADDLE_HEIGHT / 2);
        game.ball_vy += hit_pos * 0.5f;
    }
}
```

---

### 📌 PERSONA 3: Arquitectura del Cliente e Interfaz

**Tiempo estimado:** 5-7 minutos

**Temas a cubrir:**

1. **Arquitectura del Cliente** (2 min)
   - Conexión al servidor
   - Asignación de ID de jugador
   - Loop principal del cliente

2. **Interfaz con ncurses** (3 min)
   - Ventana de juego
   - Ventana de estadísticas
   - Renderizado a 60 FPS
   - Uso de colores

3. **Manejo de Inputs** (2 min)
   - Captura de teclas (W/S/Q)
   - Envío al servidor
   - Non-blocking input

**Archivos a revisar:**
- `src/pong_client.c` (líneas 28-63: `init_ncurses()`)
- `src/pong_client.c` (líneas 68-132: `render_game()`)
- `src/pong_client.c` (líneas 137-186: `render_stats()`)

**Puntos clave:**
- ncurses permite UI avanzada en terminal
- Renderizado eficiente con ventanas separadas
- Input non-blocking para no bloquear el juego

**Layout a mostrar:**
```
┌─────────────────────────┬──────────────────┐
│      PONG GAME          │   ESTADÍSTICAS   │
│  ║            ●      ║   │  RTT: 3.2 ms     │
│  ║                   ║   │  Pérdida: 0%     │
│  P1: 3        P2: 2     │  Enviados: 1523  │
└─────────────────────────┴──────────────────┘
```

---

### 📌 PERSONA 4: Estadísticas y Demostración

**Tiempo estimado:** 5-7 minutos

**Temas a cubrir:**

1. **Sistema de Estadísticas** (3 min)
   - RTT (Round Trip Time)
   - Pérdida de paquetes
   - Throughput
   - Algoritmo EWMA para promedios

2. **Manejo de Condiciones de Red** (2 min)
   - Tolerancia a pérdida de paquetes
   - Uso de timestamps
   - Descarte de datos viejos

3. **Demostración en Vivo** (2 min)
   - Juego en condiciones normales
   - Simulación de pérdida de paquetes con `tc`
   - Mostrar cómo cambian las estadísticas

**Archivos a revisar:**
- `src/stats.c` (líneas 18-30: `stats_update_rtt()`)
- `src/stats.c` (líneas 60-70: `stats_get_loss_percent()`)

**Puntos clave:**
- Estadísticas en tiempo real sin overhead
- EWMA suaviza variaciones de RTT
- El juego tolera pérdida de paquetes

**Demostración:**
```bash
# 1. Mostrar juego normal (0% pérdida, RTT < 5ms)

# 2. Simular 10% pérdida
sudo tc qdisc add dev lo root netem loss 10%

# 3. Observar estadísticas (pérdida ~10%, juego sigue funcionando)

# 4. Limpiar
sudo tc qdisc del dev lo root
```

**Código a explicar:**
```c
// EWMA para RTT promedio
if (stats->rtt_avg == 0) {
    stats->rtt_avg = rtt_ms;
} else {
    stats->rtt_avg = 0.9 * stats->rtt_avg + 0.1 * rtt_ms;
}
```

---

## 🎓 Conceptos Demostrados

### Redes de Computadoras
- ✅ Protocolo de capa de aplicación (Capa 7 OSI)
- ✅ Sockets UDP en Linux
- ✅ Arquitectura cliente-servidor
- ✅ Mensajes binarios vs texto

### Aplicaciones en Tiempo Real
- ✅ Baja latencia (< 5ms en localhost)
- ✅ Tolerancia a pérdida de paquetes
- ✅ Actualización a 60 FPS
- ✅ Prioridad: velocidad > confiabilidad

### Medición de QoS
- ✅ RTT (latencia)
- ✅ Pérdida de paquetes
- ✅ Throughput
- ✅ Jitter (variación de latencia)

---

## 🧪 Pruebas y Validación

### Test 1: Condiciones Normales
```bash
bin/pong_server  # Terminal 1
bin/pong_client  # Terminal 2 y 3
```
**Resultado esperado:** RTT < 5ms, pérdida 0%, juego fluido

### Test 2: Pérdida de Paquetes
```bash
sudo tc qdisc add dev lo root netem loss 10%
```
**Resultado esperado:** Pérdida ~10%, juego sigue funcionando

### Test 3: Latencia Alta
```bash
sudo tc qdisc add dev lo root netem delay 50ms
```
**Resultado esperado:** RTT ~50ms, ligero retraso perceptible

### Limpiar Simulación
```bash
sudo tc qdisc del dev lo root
```

---

## 📚 Referencias

- **RFC 768**: User Datagram Protocol (UDP)
- **Beej's Guide to Network Programming**: Sockets en C
- **ncurses Programming HOWTO**: Interfaz de terminal
- **Game Networking**: Gaffer on Games

---

## 👨‍💻 Autores

**Equipo de 4 personas** - Redes de Computadoras  
Universidad Nacional de San Agustín (UNSA)

---

## 📄 Licencia

Proyecto académico - Uso educativo

---

## 🚀 Trabajo Futuro

Posibles mejoras:
- [ ] Soporte para más de 2 jugadores
- [ ] Reconexión automática
- [ ] Encriptación de mensajes
- [ ] Modo espectador
- [ ] Replay de partidas
- [ ] Matchmaking automático

---

**¡Disfruta jugando y aprendiendo sobre UDP!** 🎮🌐
