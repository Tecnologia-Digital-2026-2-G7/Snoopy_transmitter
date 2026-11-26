# ESP32 Remote Control por ESP-NOW



Control remoto inalámbrico basado en dos ESP32. Un **emisor** con cinco botones detecta las pulsaciones y las transmite por **ESP-NOW** a un **receptor**, que ejecuta la acción correspondiente: siguiente, anterior, subir volumen, bajar volumen y reproducir/pausar.



## Características



- Cinco comandos: `NEXT`, `PREVIOUS`, `VOLUME UP`, `VOLUME DOWN`, `PLAY/STOP`.

- Detección de **flanco de bajada** con antirrebote por software.

- Arquitectura modular basada en tareas y colas de FreeRTOS.

- Comunicación directa entre placas, sin router ni WiFi.

- Paquete de solo 2 bytes con número de secuencia para detectar pérdidas.



## Cómo funciona



```

Botón -> [buttons_task] -> cola -> [espnow_task] -> radio -> [onReceive] -> acción

&#x20;         detecta flanco          arma el paquete             decodifica

&#x20;         y antirrebote           y lo envía                  y ejecuta

```



1. `buttons_task` muestrea los cinco pines cada 10 ms. Una pulsación se acepta solo si la lectura se mantiene estable en dos muestras consecutivas y el estado pasó de `HIGH` a `LOW`.

2. El comando detectado se deja en una cola de FreeRTOS.

3. `espnow_task` permanece bloqueada hasta que hay un comando en la cola, lo empaqueta y lo envía por ESP-NOW.

4. En el receptor, el callback `onReceive` valida el tamaño del paquete y ejecuta la acción según el comando.



## Estructura del proyecto



```

esp_remote/                  Emisor (ESP32 con botones)

├── esp_remote.ino           Punto de entrada: crea la cola e inicia los módulos

├── buttons.h / buttons.cpp  Lectura de botones, antirrebote y detección de flanco

├── espnow_link.h / .cpp     Inicialización de ESP-NOW y envío de comandos

└── remote_protocol.h        Comandos y formato del paquete (compartido)



esp_remote_rx/               Receptor

├── esp_remote_rx.ino        Recepción de paquetes y ejecución de acciones

└── remote_protocol.h        Copia idéntica del archivo del emisor

```



> Cada carpeta es un sketch independiente de Arduino y se carga en su propia placa. `remote_protocol.h` debe ser **idéntico** en ambas.



## Hardware



- 2 × ESP32 (cualquier variante con WiFi).

- 5 × pulsadores normalmente abiertos.

- Cables y protoboard.



### Conexión de los botones



Cada botón se conecta entre su pin y **GND**. No necesita resistencia externa porque se usa la resistencia pull-up interna (`INPUT_PULLUP`).



| Botón | Comando | Pin por defecto |

|---|---|---|

| Siguiente | `CMD_NEXT` | GPIO 32 |

| Anterior | `CMD_PREVIOUS` | GPIO 33 |

| Volumen + | `CMD_VOLUME_UP` | GPIO 25 |

| Volumen − | `CMD_VOLUME_DOWN` | GPIO 26 |

| Play / Stop | `CMD_PLAY_STOP` | GPIO 27 |



Los pines se cambian en la tabla `BUTTONS[]` de `buttons.cpp`.



## Requisitos de software



- Arduino IDE 2.x (o PlatformIO).

- Soporte para placas ESP32 (core de Espressif).

- El receptor usa la firma de callback del **core 3.x**. Con el core 2.x debe ser `(const uint8_t *mac, const uint8_t *data, int len)`.



## Puesta en marcha



1. **Receptor.** Abre la carpeta `esp_remote_rx/`, carga el sketch en la placa receptora y abre el monitor serie a 115200 baudios. Se imprimirá su dirección MAC, por ejemplo `A4:CF:12:34:56:78`.

2. **Emisor.** En `esp_remote.ino`, escribe esa MAC en `RECEIVER_MAC`:



&#x20;  ```cpp

&#x20;  static const uint8_t RECEIVER_MAC[6] = { 0xA4, 0xCF, 0x12, 0x34, 0x56, 0x78 };

&#x20;  ```



3. Abre la carpeta `esp_remote/` y carga el sketch en la placa emisora.

4. Presiona un botón. El monitor serie del receptor mostrará el comando recibido.



## Protocolo



ESP-NOW solo transporta bytes, por lo que el proyecto define su propio paquete de 2 bytes:



```cpp

typedef struct __attribute__((packed)) {

&#x20; uint8_t cmd;

&#x20; uint8_t seq;

} RemotePacket;

```



| Byte | Campo | Descripción |

|---|---|---|

| 0 | `cmd` | Código del comando |

| 1 | `seq` | Contador de envíos (0 a 255, luego reinicia) |



### Comandos



| Valor | Nombre | Acción |

|---|---|---|

| 1 | `CMD_NEXT` | Siguiente |

| 2 | `CMD_PREVIOUS` | Anterior |

| 3 | `CMD_VOLUME_UP` | Subir volumen |

| 4 | `CMD_VOLUME_DOWN` | Bajar volumen |

| 5 | `CMD_PLAY_STOP` | Reproducir / detener |



El valor `0` queda reservado como inválido.



## Personalización



**Agregar un botón**



1. Añade un valor al enum `RemoteCmd` en `remote_protocol.h` (en ambas copias).

2. Añade una fila a `BUTTONS[]` en `buttons.cpp`.

3. Añade un `case` en el `switch` de `esp_remote_rx.ino`.



**Cambiar la acción en el receptor.** Reemplaza los `Serial.println` del `switch` por la lógica real (control de un reproductor, de un amplificador, etc.). Como `onReceive` se ejecuta dentro de la tarea interna de WiFi, si la acción es pesada conviene pasar el comando a una cola y procesarlo en otra tarea.



**Ajustar la sensibilidad.** `SAMPLE_MS` en `buttons.cpp` define el periodo de muestreo y, con él, el tiempo de antirrebote.



## Limitaciones conocidas



- `esp_now_send` devuelve `ESP_OK` cuando el paquete se entrega a la radio, no cuando el receptor lo confirma. Para conocer el resultado real hay que registrar un callback de envío.

- El cifrado está desactivado (`encrypt = false`). Cualquier dispositivo que conozca la MAC y el formato podría enviar comandos al receptor.

- El número de secuencia se transmite, pero el receptor aún no lo utiliza para detectar pérdidas.

- Emisor y receptor deben usar el mismo canal WiFi. Con `channel = 0` ambos usan el canal por defecto.



## Posibles mejoras



- Repetición automática al mantener presionados los botones de volumen.

- Callback de confirmación de envío.

- Cifrado de los paquetes con clave compartida.

- Uso del campo `seq` para detectar y registrar paquetes perdidos.

- Librería propia para compartir `remote_protocol.h` entre ambos proyectos.



## Licencia



Completar con la licencia del proyecto.

