/*
ver ::cl 20120520
Configuracion basica para modulo receptor  RR 10
Utiliza libreria VirtualWire.h
pin 01 5v
pin 02 Tierra
pin 03 antena externa
pin 07 tierra
pin 10 5v
pin 11 tierra
pin 12 5v
pin 14 Arduino pin digital 2
pin 15 5v
*/
#include <VirtualWire.h>
#define RECEIVE_PIN 2

int recibidos = 0;

uint16_t calculateCRC5USB(uint8_t *data, uint8_t length) {
  uint16_t crc = 0x1F;
  uint16_t polynomial = 0x25;

  for (uint8_t i = 0; i < length; i++) {
    crc ^= data[i];

    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x10) {
        crc = (crc << 1) ^ polynomial;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc & 0x1F;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Configurando Recepción");
  vw_set_ptt_inverted(true); 
  vw_setup(2000);
  vw_set_rx_pin(RECEIVE_PIN);
  vw_rx_start();
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  uint8_t destinoEsperado[2] = {0x00, 0x07}; 

  if (vw_get_message(buf, &buflen)) {
    uint8_t origen[2] = {buf[0], buf[1]};
    uint8_t destino[2] = {buf[2], buf[3]};

    uint8_t secuencia = buf[6];
    uint8_t total = buf[7];
    char mensaje[9];
    memcpy(mensaje, buf + 8, 8);
    mensaje[8] = '\0';
       
    uint8_t receivedCRC = buf[5];
    uint8_t calculatedCRC = calculateCRC5USB(&buf[8], 8);

    if ((destino[0] == 0x00 && destino[1] == destinoEsperado[0]) || (destino[0] == 0x00 && destino[1] == destinoEsperado[1])) {
      
      Serial.print("[Origen: ");
      Serial.print(origen[0], HEX);
      Serial.print(origen[1], HEX);
      if(destino[1] == 0x00) Serial.print(" - Broadcast ");
      else Serial.print(" - Unicast ");

      Serial.print(destino[0], HEX);
      Serial.print(destino[1], HEX);

      Serial.print(" P(");
      Serial.print(secuencia, HEX);
      Serial.print(":");
      Serial.print(total, HEX);
      Serial.print(")");
      Serial.print("] > ");
      //Serial.print("Destino: ");
      //Serial.print(destino[0], HEX);
      //Serial.print(destino[1], HEX);
      //Serial.print("  Secuencia: ");
      //Serial.print(total, HEX);
      //Serial.print("  Mensaje: ");
      Serial.println(mensaje);

      // Verificar el CRC
      if (receivedCRC == calculatedCRC) {
        Serial.println("CRC correcto");
        //Serial.print(receivedCRC, BIN);
        //Serial.print(", Calculado: ");
        //Serial.println(calculatedCRC, BIN);
      } else {
        Serial.print("CRC incorrecto. Recibido: ");
        Serial.print(receivedCRC, BIN);
        Serial.print(", Calculado: ");
        Serial.println(calculatedCRC, BIN);
      }
    };
    digitalWrite(13, true); // Indicar recepción
  };
}
