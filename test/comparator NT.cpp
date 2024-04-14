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

// Instancias de la clase SoftSlave.
SoftSlave mySlave_1(MOSI_PIN_7, MISO_PIN_8, SCK_PIN_9, SS_PIN_2);
SoftSlave mySlave_2(MOSI_PIN_10, MISO_PIN_11, SCK_PIN_12, SS_PIN_3);

unsigned char receivedData1 = 0;
unsigned char receivedData2 = 0;
unsigned char actualizado1 = 0;
unsigned char actualizado2 = 0;
unsigned char ack = 35;

struct SRTankData {
    bool highFloater;
    bool lowFloater;
    bool maxTemperature;
    bool minTemperature;
}; SRTankData tankData;

struct SRTankData2 {
    unsigned int volume;
    unsigned int temperature;
}; SRTankData2 tankData2;

// ISR para SS de SPI_1.
void SPI1_SS_ISR(){
    isSPI1Active = true; // Inmediatamente inicia la comunicación SPI1 si es posible.
}

// ISR para SS de SPI_2.
void SPI2_SS_ISR(){
    isSPI2Active = true; // Inmediatamente inicia la comunicación SPI2 si es posible.
}

void initSRTank(){
    // Initialize serial communication.
    Serial.begin(19200);
    
    // Synchronize communication with the simulator.
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
}

void setup(){
    Serial.begin(9600);

    mySlave_1.beginSlave();
    mySlave_1.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.
    mySlave_2.beginSlave();

    // Configuración de las interrupciones para los pines SS.
    pinMode(SS_PIN_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_2), SPI1_SS_ISR, FALLING);
    
    pinMode(SS_PIN_3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_3), SPI2_SS_ISR, FALLING);

    // Initialize the SRTank.
    tankData.highFloater = true;
    tankData.lowFloater = false;
    tankData.maxTemperature = true;
    tankData.minTemperature = false;
    tankData2.volume = 58;
    tankData2.temperature = 7;
}

void loop(){
    // Máquina de estados para manejar las comunicaciones SPI.
    if(isSPI1Active){ // Sería el master1 y tendría prioridad sobre el master2.
        delay(5); // Espera un poco para iniciar la comunicación.
        receivedData1 = mySlave_1.transfer(ack-12); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al slave.
        if(receivedData1 > 0){
            mySlave_1.transferBit(tankData.highFloater);
            mySlave_1.transferBit(tankData.lowFloater);
            mySlave_1.transferBit(tankData.maxTemperature);
            mySlave_1.transferBit(tankData.minTemperature);
            mySlave_1.WriteAnything(tankData2);
        }
        actualizado1 = 1;
        isSPI1Active = false; // Termina la comunicación SPI1.
    }
    if(isSPI2Active){ // Sería el master2.
        delay(8); // Espera un poco para iniciar la comunicación.
        receivedData2 = mySlave_2.transfer(ack); // Envía confirmación.
        
        // Si se recibe el comando esperado, se envía el struct al slave.
        if(receivedData2 > 0){
            mySlave_2.transferBit(tankData.highFloater);
            mySlave_2.transferBit(tankData.lowFloater);
            mySlave_2.transferBit(tankData.maxTemperature);
            mySlave_2.transferBit(tankData.minTemperature);
            mySlave_2.WriteAnything(tankData2);
        }
        actualizado2 = 1;
        isSPI2Active = false; // Termina la comunicación SPI2.
    }

    if(actualizado1){
        Serial.print("Respuesta_1: ");
        Serial.println(receivedData1);
        receivedData1 = 0; // Limpia el dato recibido.
        actualizado1 = 0; // Flag reset.
    }
    if(actualizado2){
        Serial.print("Respuesta_2: ");
        Serial.println(receivedData2);
        receivedData2 = 0; // Limpia el dato recibido.
        actualizado2 = 0; // Flag reset.
    }
}