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
    pinMode(SS_PIN_2, INPUT);
    
    pinMode(SS_PIN_3, INPUT);

    // Inicialización del SRTank.
    tankData.highFloater = 55;
    tankData.lowFloater = 44;
    tankData.maxTemperature = 33;
    tankData.minTemperature = 22;
    tankData.volume = 58;
    tankData.temperature = 7;
}

void imprimirSRTank(){
    Serial.print("SRTank: ");
    Serial.print(tankData.highFloater);
    Serial.print(", ");
    Serial.print(tankData.lowFloater);
    Serial.print(", ");
    Serial.print(tankData.maxTemperature);
    Serial.print(", ");
    Serial.print(tankData.minTemperature);
    Serial.print(", ");
    Serial.print(tankData.volume);
    Serial.print(", ");
    Serial.println(tankData.temperature);
}

void loop(){

    if(digitalRead(SS_PIN_2) == LOW){
        Serial.println("Listening Master 1");
        if(mySlave_1.isSelected()){
            
            delay(10); // Espera un poco para recibir el comando.
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
    }

    if(digitalRead(SS_PIN_3) == LOW){
        if(mySlave_2.isSelected()){
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
}