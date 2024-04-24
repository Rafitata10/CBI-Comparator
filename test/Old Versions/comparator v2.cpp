/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#include <Arduino.h>
#include "SoftSlave.h"

// Variables para manejar las comunicaciones SPI.
volatile bool isSPI1Active = false;
volatile bool isSPI2Active = false;

// Define los pines para mySlave_1.
#define MOSI_PIN_7 7
#define MISO_PIN_8 8
#define SCK_PIN_9 9
#define SS_PIN_2 2
// Define los pines para mySlave_2.
#define MOSI_PIN_10 10
#define MISO_PIN_11 11
#define SCK_PIN_12 12
#define SS_PIN_3 3
// LEDs pins.
#define LED_PIN_4 4
#define LED_PIN_5 5

struct SRTankData {
    unsigned char highFloater;
    unsigned char lowFloater;
    unsigned char maxTemperature;
    unsigned char minTemperature;
    unsigned int volume;
    unsigned int temperature;
}; SRTankData tankData;

// Instancias de la clase SoftSlave.
SoftSlave mySlave_1(MOSI_PIN_7, MISO_PIN_8, SCK_PIN_9, SS_PIN_2);
SoftSlave mySlave_2(MOSI_PIN_10, MISO_PIN_11, SCK_PIN_12, SS_PIN_3);

unsigned char receivedData1;
unsigned char receivedData2;
unsigned char actualizado = 0;

void setup(){
    Serial.begin(9600);

    mySlave_1.beginSlave();
    mySlave_1.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.
    mySlave_2.beginSlave();

    pinMode(LED_PIN_4, OUTPUT);
    pinMode(LED_PIN_5, OUTPUT);
    receivedData1 = 0;
    receivedData2 = 0;

    // Configuración de las interrupciones para los pines SS.
    // pinMode(SS_PIN_2, INPUT);
    // pinMode(SS_PIN_3, INPUT);

    // Inicialización del SRTank.
    tankData.highFloater = 55;
    tankData.lowFloater = 44;
    tankData.maxTemperature = 33;
    tankData.minTemperature = 22;
    tankData.volume = 58;
    tankData.temperature = 7;
}

void loop(){
    // Revisar si el Chip Select 1 ha sido activado.
    if(digitalRead(SS_PIN_2) == LOW){
        isSPI1Active = true; // Indica que hay una solicitud de comunicación SPI1
    }

    // Revisar si el Chip Select 2 ha sido activado.
    if(digitalRead(SS_PIN_3) == LOW){
        isSPI2Active = true; // Indica que hay una solicitud de comunicación SPI2
    }

    // Máquina de estados para manejar las comunicaciones SPI.
    if(isSPI1Active){ // Sería el master1 y tendría prioridad sobre el master2.
        receivedData1 = mySlave_1.transfer(23); // Envía confirmación.

        delay(10); // Espera un poco para enviar el struct.
        // Si se recibe el comando esperado, prepara el struct para enviar
        if(receivedData1 > 0){ // '1' es el comando esperado
            mySlave_1.WriteAnything(tankData);
            actualizado = 1;
        }
        
        isSPI1Active = false; // Termina la comunicación SPI1.
        Serial.print("Respuesta_1: ");
        Serial.println(receivedData1); 
        receivedData1 = 0; // Limpia el dato recibido.
    }

    if(isSPI2Active){
        receivedData2 = mySlave_2.transfer(35); // Envía confirmación.
        
        delay(10); // Espera un poco para enviar el struct.

        // Si se recibe el comando esperado, prepara el struct para enviar
        if(receivedData2 > 0){ // '1' es el comando esperado
            mySlave_2.WriteAnything(tankData);
            actualizado = 1;
        }
        
        isSPI2Active = false; // Termina la comunicación SPI2.
        Serial.print("Respuesta_2: ");
        Serial.println(receivedData2);
        receivedData2 = 0; // Limpia el dato recibido.
    }
}