# 🎤 Guía de Presentación: Protocolo UDP-PONG

## 📋 División para 4 Personas

Esta guía se enfoca **exclusivamente en el protocolo UDP** y cómo fue implementado.

---

## 📌 PERSONA 1: Introducción y Fundamentos de UDP

**Tiempo:** 5 minutos

### Temas a Cubrir

#### 1. ¿Qué es UDP? (2 min)

**Definición:**
- UDP = User Datagram Protocol
- Protocolo de capa de transporte (Capa 4 OSI)
- Sin conexión, no confiable, rápido

**Comparación UDP vs TCP:**

| Característica | TCP | UDP |
|----------------|-----|-----|
| **Conexión** | Requiere handshake (SYN, SYN-ACK, ACK) | Sin conexión |
| **Confiabilidad** | Garantiza entrega | No garantiza |
| **Orden** | Garantiza orden | No garantiza |
| **Velocidad** | Más lento (overhead) | Más rápido |
| **Cabecera** | 20 bytes mínimo | 8 bytes |
| **Control de flujo** | Sí (ventana deslizante) | No |
| **Uso típico** | HTTP, FTP, SSH | DNS, streaming, juegos |

#### 2. ¿Por qué UDP para Juegos? (2 min)

**Ventajas:**
1. **Baja latencia**: No espera confirmaciones (ACKs)
2. **Velocidad > Confiabilidad**: Mejor perder un paquete que esperar retransmisión
3. **Overhead mínimo**: Solo 8 bytes de cabecera
4. **Datos obsoletos**: Si un paquete llega tarde, ya no importa

**Ejemplo visual:**
```
TCP: [Enviar paquete] → [Esperar ACK] → [Enviar siguiente]  ❌ Lento
UDP: [Enviar] [Enviar] [Enviar] [Enviar]                    ✅ Rápido
```

**Casos de uso:**
- Juegos en tiempo real (FPS, MOBA)
- Streaming de video/audio
- VoIP (llamadas)
- DNS queries

#### 3. Desafíos de UDP (1 min)

**Problemas que debemos resolver:**
- ❌ Pérdida de paquetes
- ❌ Paquetes desordenados
- ❌ Sin control de flujo
- ❌ Sin detección de errores automática

**Soluciones a nivel de aplicación:**
- ✅ Timestamps para orden
- ✅ Estado completo en cada paquete
- ✅ Checksums para integridad
- ✅ Estadísticas para monitoreo

---

## 📌 PERSONA 2: Diseño del Protocolo UDP-PONG

**Tiempo:** 5 minutos

### Temas a Cubrir

#### 1. Arquitectura del Protocolo (1 min)

**Capas del sistema:**
```
┌─────────────────────────────────┐
│  Aplicación: UDP-PONG           │  ← Capa 7 (Nuestro protocolo)
├─────────────────────────────────┤
│  Transporte: UDP                │  ← Capa 4
├─────────────────────────────────┤
│  Red: IP                        │  ← Capa 3
├─────────────────────────────────┤
│  Enlace: Ethernet               │  ← Capa 2
└─────────────────────────────────┘
```

**Nuestro trabajo:** Diseñar el protocolo de Capa 7 sobre UDP

#### 2. Estructura de Mensajes (3 min)

**Decisión de diseño: Mensajes binarios**

**¿Por qué binario y no JSON/texto?**
- Más eficiente (menos bytes)
- Más rápido de parsear
- Tamaño fijo y predecible

**Mensaje Cliente → Servidor (26 bytes):**
```c
struct client_message {
    uint8_t type;           // 1 byte:  Tipo de mensaje
    uint32_t timestamp;     // 4 bytes: Timestamp en ms
    uint8_t player_id;      // 1 byte:  ID del jugador (1 o 2)
    int8_t action;          // 1 byte:  Acción (-1, 0, 1)
    char player_name[16];   // 16 bytes: Nombre
} __attribute__((packed));  // Total: 26 bytes
```

**Tipos de mensaje:**
- `JOIN (1)`: Conectarse al servidor
- `INPUT (2)`: Enviar acción del jugador
- `LEAVE (4)`: Desconectarse

**Mensaje Servidor → Cliente (40 bytes):**
```c
struct server_message {
    uint8_t type;           // 1 byte:  Tipo de respuesta
    uint32_t timestamp;     // 4 bytes: Timestamp del servidor
    uint8_t player_id;      // 1 byte:  ID asignado
    
    // Estado del juego (24 bytes)
    float paddle1_y;        // 4 bytes
    float paddle2_y;        // 4 bytes
    float ball_x;           // 4 bytes
    float ball_y;           // 4 bytes
    uint8_t score1;         // 1 byte
    uint8_t score2;         // 1 byte
    
    // Estadísticas (10 bytes)
    uint16_t rtt_ms;        // 2 bytes
    uint8_t loss_percent;   // 1 byte
    uint32_t packets_sent;  // 4 bytes
    uint32_t packets_recv;  // 4 bytes
} __attribute__((packed));  // Total: 40 bytes
```

**Tipos de respuesta:**
- `STATE (1)`: Estado actual del juego
- `STATS (2)`: Estadísticas detalladas
- `ERROR (3)`: Error en el protocolo

#### 3. Ventajas del Diseño (1 min)

✅ **Eficiente**: Solo 26-40 bytes por paquete  
✅ **Rápido**: Binario es más rápido que texto  
✅ **Estado completo**: Cada paquete tiene toda la información  
✅ **Sin dependencias**: No importa si se pierde un paquete  

**Comparación con JSON:**
```json
// JSON (ejemplo): ~150 bytes
{
  "type": "STATE",
  "timestamp": 1234567890,
  "paddle1_y": 50.5,
  "paddle2_y": 45.2,
  ...
}

// Binario: 40 bytes
[01][4D 2C 61 D2][01][42 48 00 00][42 34 CC CD]...
```

**Ahorro:** ~73% menos bytes

---

## 📌 PERSONA 3: Flujo de Comunicación del Protocolo

**Tiempo:** 5 minutos

### Temas a Cubrir

#### 1. Secuencia de Conexión (2 min)

**Paso 1: Cliente se conecta**
```
Cliente                          Servidor
   |                                |
   |----JOIN "Player1"------------->|
   |    [26 bytes]                  |
   |                                | [Registra jugador]
   |                                | [Asigna ID = 1]
   |<---STATE (player_id=1)---------|
   |    [40 bytes]                  |
```

**Contenido del JOIN:**
```
type = 1 (JOIN)
timestamp = 1702598400000
player_id = 0 (aún no asignado)
action = 0
player_name = "Player1"
```

**Respuesta STATE:**
```
type = 1 (STATE)
timestamp = 1702598400010
player_id = 1 ← ID asignado por servidor
paddle1_y = 50.0
paddle2_y = 50.0
ball_x = 50.0
ball_y = 50.0
score1 = 0
score2 = 0
```

#### 2. Loop de Juego (2 min)

**Actualización a 60 FPS (cada 16ms):**

```
Cliente 1                Servidor                Cliente 2
   |                        |                        |
   |--INPUT (ARRIBA)------->|                        |
   |                        |<--INPUT (ABAJO)--------|
   |                        |                        |
   |                        | [Actualiza física]     |
   |                        | - Mueve paletas        |
   |                        | - Mueve pelota         |
   |                        | - Detecta colisiones   |
   |                        |                        |
   |<-----STATE-------------|------STATE------------>|
   |                        |                        |
   | [Renderiza]            |            [Renderiza] |
```

**Contenido del INPUT:**
```
type = 2 (INPUT)
timestamp = 1702598400026  ← 16ms después
player_id = 1
action = 1 (ARRIBA)
```

**Respuesta STATE (con nuevo estado):**
```
type = 1 (STATE)
timestamp = 1702598400027
player_id = 0 (no relevante en broadcast)
paddle1_y = 52.0  ← Se movió arriba
paddle2_y = 48.0  ← Se movió abajo
ball_x = 51.5     ← Pelota se movió
ball_y = 50.8
score1 = 0
score2 = 0
rtt_ms = 3        ← Estadísticas
loss_percent = 0
```

#### 3. Manejo de Problemas de UDP (1 min)

**Problema 1: Paquete perdido**
```
Cliente envía: INPUT#100, INPUT#101, INPUT#102
Servidor recibe: INPUT#100, [PERDIDO], INPUT#102

Solución: No importa, INPUT#102 tiene la acción actual
```

**Problema 2: Paquetes desordenados**
```
Servidor envía: STATE#100 (t=1000), STATE#101 (t=1016)
Cliente recibe: STATE#101 (t=1016), STATE#100 (t=1000)

Solución: Usar timestamps
if (received.timestamp > last_timestamp) {
    actualizar_estado();
} else {
    descartar();  // Paquete viejo
}
```

**Problema 3: Sin confirmación**
```
UDP no tiene ACKs, ¿cómo sabemos si llegó?

Solución: Estadísticas a nivel de aplicación
- Contar paquetes enviados
- Contar paquetes recibidos
- Calcular pérdida = (enviados - recibidos) / enviados
```

---

## 📌 PERSONA 4: Implementación y Validación del Protocolo

**Tiempo:** 5 minutos

### Temas a Cubrir

#### 1. Implementación en C (2 min)

**Socket UDP en el servidor:**
```c
// 1. Crear socket UDP
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                           //  ^^^^^^^^^ UDP

// 2. Configurar dirección
struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);
server_addr.sin_addr.s_addr = INADDR_ANY;

// 3. Bind (asociar socket con puerto)
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

// 4. Recibir mensajes (no hay listen/accept como en TCP)
recvfrom(sockfd, &msg, sizeof(msg), 0, 
         (struct sockaddr *)&client_addr, &len);

// 5. Enviar respuesta
sendto(sockfd, &response, sizeof(response), 0,
       (struct sockaddr *)&client_addr, len);
```

**Diferencias con TCP:**
- ❌ No hay `listen()`
- ❌ No hay `accept()`
- ❌ No hay `connect()` (cliente)
- ✅ Usa `sendto()` / `recvfrom()` directamente

**Socket UDP en el cliente:**
```c
// 1. Crear socket
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

// 2. Configurar dirección del servidor
struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);
server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

// 3. Enviar directamente (sin connect)
sendto(sockfd, &msg, sizeof(msg), 0,
       (struct sockaddr *)&server_addr, sizeof(server_addr));

// 4. Recibir respuesta
recvfrom(sockfd, &response, sizeof(response), 0, ...);
```

#### 2. Características del Protocolo Implementado (2 min)

**a) Timestamps para orden:**
```c
// Cliente
msg.timestamp = get_time_ms();  // Ej: 1702598400000
sendto(sockfd, &msg, ...);

// Servidor verifica
if (msg.timestamp > last_timestamp) {
    procesar_mensaje();
}
```

**b) Estado completo (no incremental):**
```c
// Cada STATE tiene TODA la información
struct server_message state;
state.paddle1_y = 52.0;  // Posición absoluta
state.paddle2_y = 48.0;  // No es "mover +2"
state.ball_x = 51.5;
state.ball_y = 50.8;
// ... todo el estado
```

**Ventaja:** Si se pierde un paquete, el siguiente tiene todo

**c) Estadísticas integradas:**
```c
// Cada STATE incluye métricas
state.rtt_ms = 3;           // Latencia
state.loss_percent = 0;     // Pérdida
state.packets_sent = 1523;
state.packets_recv = 1488;
```

**d) Checksums implícitos:**
- UDP ya incluye checksum en su cabecera
- Detecta corrupción de datos
- Si falla checksum, paquete se descarta

#### 3. Validación del Protocolo (1 min)

**Test 1: Condiciones normales**
```
Resultado:
- RTT: 2-5ms (localhost)
- Pérdida: 0%
- Throughput: ~15 Kbps
- Juego fluido a 60 FPS
```

**Test 2: Simulación de pérdida (10%)**
```bash
sudo tc qdisc add dev lo root netem loss 10%
```
```
Resultado:
- RTT: 2-5ms (sin cambio)
- Pérdida: ~10%
- Juego sigue funcionando
- Movimientos ocasionalmente entrecortados
```

**Conclusión:** El protocolo tolera pérdida de paquetes

**Test 3: Simulación de latencia (50ms)**
```bash
sudo tc qdisc add dev lo root netem delay 50ms
```
```
Resultado:
- RTT: ~50ms
- Pérdida: 0%
- Ligero retraso perceptible
- Juego sigue jugable
```

**Conclusión:** El protocolo funciona con latencia variable

---

## 🎯 Resumen Final (Todos juntos - 2 min)

### Conceptos Clave Demostrados

**1. Protocolo de Capa 7:**
- Diseñamos mensajes personalizados
- Definimos tipos, estructuras, y flujo
- Implementamos sobre UDP (Capa 4)

**2. Características de UDP:**
- Sin conexión (no handshake)
- No confiable (pérdida tolerada)
- Rápido (baja latencia)
- Ideal para tiempo real

**3. Soluciones Implementadas:**
- Timestamps → Orden de paquetes
- Estado completo → Tolerancia a pérdida
- Estadísticas → Monitoreo de QoS
- Mensajes binarios → Eficiencia

**4. Resultados:**
- ✅ Protocolo funcional
- ✅ 26-40 bytes por paquete
- ✅ 60 FPS constantes
- ✅ Tolera 10% de pérdida
- ✅ RTT < 5ms en localhost

---

## 📊 Distribución del Tiempo

| Persona | Tema | Tiempo |
|---------|------|--------|
| 1 | Fundamentos de UDP | 5 min |
| 2 | Diseño del protocolo | 5 min |
| 3 | Flujo de comunicación | 5 min |
| 4 | Implementación y validación | 5 min |
| **Total** | | **20 min** |

Dejar 5-10 min para preguntas.

---

## ❓ Preguntas Frecuentes

**P: ¿Por qué no usar TCP si es más confiable?**  
R: TCP es más lento. Para juegos, es mejor perder un paquete que esperar retransmisión. La velocidad es crítica.

**P: ¿Qué pasa si se pierden muchos paquetes?**  
R: El juego se vuelve entrecortado pero sigue funcionando. En redes reales, 1-2% de pérdida es normal.

**P: ¿Por qué mensajes binarios y no JSON?**  
R: Binario es más eficiente (73% menos bytes) y más rápido de parsear.

**P: ¿Cómo detectan paquetes viejos?**  
R: Cada paquete tiene un timestamp. Si llega un paquete con timestamp menor al último, se descarta.

**P: ¿Qué es el overhead de UDP?**  
R: UDP solo agrega 8 bytes de cabecera (vs 20 de TCP). Nuestro protocolo agrega 26-40 bytes de datos de aplicación.

---

## 🎓 Conclusión

Este proyecto demuestra:
- ✅ Diseño de un protocolo de capa de aplicación
- ✅ Implementación sobre UDP
- ✅ Manejo de desafíos de UDP (pérdida, orden)
- ✅ Medición de calidad de servicio (QoS)
- ✅ Aplicación práctica en juegos en tiempo real

**El protocolo UDP-PONG es eficiente, rápido, y tolerante a fallos de red.**
