/*
ver ::cl 20120520
Configuracion basica para modulo transmisor RT 11
Utiliza libreria VirtualWire.h
pin 01 entrada desde Arduino pin digital 2
pin 02 Tierra
pin 07 tierra
pin 08 antena externa
pin 09 tierra
pin 10 5v
*/

#include <VirtualWire.h>

#define TRANSMIT_PIN 2

// Función para calcular CRC-5-USB
uint8_t calculateCRC5USB(uint8_t *data, uint8_t length) {
  uint8_t crc = 0x1F;  // Inicializar CRC con 0b11111
  uint8_t polynomial = 0x25; // Polinomio 0b00100101 para x^5 + x^2 + 1

  for (uint8_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ polynomial;
      } else {
        crc <<= 1;
      }
    }
  }
  return (crc >> 3) & 0x1F; // El CRC5 se toma de los 5 bits más significativos
}

void imprimirPaquete(const uint8_t* paquete, size_t longitud) {
    for (size_t i = 0; i < longitud; ++i) {
        if (paquete[i] < 0x10) {
            Serial.print("0"); // Aseguramos que cada byte se imprima con dos dígitos
        }
        Serial.print(paquete[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

String bytesToString(const uint8_t* bytes, size_t length) {
    String result = "";
    for (size_t i = 0; i < length; i++) {
        if (bytes[i] != 0) { // Ignorar los bytes nulos (relleno)
            result += (char)bytes[i];
        }
    }
    return result;
}

void setup() {
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_set_tx_pin(TRANSMIT_PIN);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Leer el mensaje desde la consola serial

    // Datos del paquete
    uint8_t origen[2] = {0x00, 0x07};   // Ejemplo origen
    uint8_t destino[2] = {0x00, 0x00};  // Ejemplo destino
    uint8_t secuencia = 0x01;           // Número de secuencia inicial
    uint8_t total = (input.length() + 7) / 8;  // Número total de paquetes (redondeo hacia arriba)

    for (uint8_t i = 0; i < total; i++) {
      uint8_t mensaje[8] = {0}; // Inicializar mensaje con ceros
      size_t start = i * 8;
      size_t length = (input.length() - start) >= 8 ? 8 : (input.length() - start);

      // Copiar el fragmento del mensaje al array mensaje
      for (size_t j = 0; j < length; j++) {
        mensaje[j] = input[start + j];
      }

      // Crear el paquete
      uint8_t paquete[16];
      memcpy(paquete, origen, 2);
      memcpy(paquete + 2, destino, 2);
      paquete[4] = 0x00; // Placeholder para CRC, lo llenamos después
      paquete[5] = 0x00; // Placeholder para CRC, lo llenamos después
      paquete[6] = secuencia + i;
      paquete[7] = total;
      memcpy(paquete + 8, mensaje, 8);

      // Calcular CRC
      uint8_t crc = calculateCRC5USB(paquete + 8, 8); // Calcular CRC solo en los primeros 12 bytes
      //paquete[4] = 0x00;
      paquete[5] = crc;

      Serial.print("[Enviando paquete ");
      Serial.print(i + 1);
      Serial.print(" de ");
      Serial.print(total);
      Serial.print(" - Destino: ");
      Serial.print(destino[1]);
      Serial.print("] > ");
      Serial.println(bytesToString(mensaje, 8)); // Imprimir el mensaje del paquete
      //Serial.print("Paquete: ");
      //imprimirPaquete(paquete, sizeof(paquete));
      // Enviar el paquete
      vw_send(paquete, sizeof(paquete));
      vw_wait_tx(); // Esperar a que se complete la transmisión

      delay(100); // Pequeña espera entre paquetes
    }
  }
}