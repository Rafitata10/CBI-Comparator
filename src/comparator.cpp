/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#include <Arduino.h>
#include <SlaveSPI.h>
#include <SRTank.h>

// Variables para manejar las comunicaciones SPI.
volatile bool isSPI1Active = false;
volatile bool isSPI2Active = false;

// Define los pines para mySlave_1.
#define MOSI_PIN_13 13
#define MISO_PIN_12 12
#define SCK_PIN_11 11
#define SS_PIN_3 3
// Define los pines para mySlave_2.
#define MOSI_PIN_7 7
#define MISO_PIN_8 8
#define SCK_PIN_9 9
#define SS_PIN_2 2
// Define los pines para los LEDs.
#define LED_PIN_4 4
#define LED_PIN_5 5

// Instancias de la clase SoftSlave.
SlaveSPI slaveSPI_1(MOSI_PIN_13, MISO_PIN_12, SCK_PIN_11, SS_PIN_3);
SlaveSPI slaveSPI_2(MOSI_PIN_7, MISO_PIN_8, SCK_PIN_9, SS_PIN_2);

unsigned char receivedData1 = 0;
unsigned char receivedData2 = 0;
unsigned char actualizado1 = 0;
unsigned char actualizado2 = 0;
unsigned char ack = 35;
SRTankData tankData = tank.getInit();
SRTankData2 tankData2 = tank.getInit2();

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
    // Serial.begin(9600);
    pinMode(LED_PIN_5, OUTPUT);
    digitalWrite(LED_PIN_5, LOW);
    pinMode(LED_PIN_4, OUTPUT);
    digitalWrite(LED_PIN_4, LOW);

    // Initialize the tank.
    initSRTank();

    // Initialize the tank alarm temperature.
    tank.setMaxTemperatureAlarm(125); // Maximum temperature alarm.
    tank.setMinTemperatureAlarm(10); // Minimum temperature alarm.

    slaveSPI_1.beginSlave();
    slaveSPI_1.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.
    slaveSPI_2.beginSlave();
    slaveSPI_2.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.

    // Configuración de las interrupciones para los pines SS.
    pinMode(SS_PIN_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_2), SPI1_SS_ISR, FALLING);
    
    pinMode(SS_PIN_3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_3), SPI2_SS_ISR, FALLING);

    // Initialize the SRTank.
    tank.fillTank(350); // Fill the tank to 350 liters.
    tank.heatTank(33); // Heat the tank to 33ºC.
}

void loop(){
    // Máquina de estados para manejar las comunicaciones SPI.
    if(isSPI1Active){ // Sería el master1 y tendría prioridad sobre el master2.
        delay(8); // Espera un poco para iniciar la comunicación.
        receivedData1 = slaveSPI_1.transfer(ack); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al slave.
        if(receivedData1 > 0){
            slaveSPI_1.WriteAnything(tankData2);
            actualizado1 = 1;
        }
        
        isSPI1Active = false; // Termina la comunicación SPI1.
    }
    if(isSPI2Active){ // Sería el master2.
        delay(8); // Espera un poco para iniciar la comunicación.
        receivedData2 = slaveSPI_2.transfer(ack); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al slave.
        if(receivedData2 > 0){
            slaveSPI_2.WriteAnything(tankData2);
            actualizado2 = 1;
        }
        
        isSPI2Active = false; // Termina la comunicación SPI2.
    }
    /*
    if(actualizado1){
        Serial.print("Respuesta_1: ");
        Serial.println(receivedData1);
        receivedData1 = 0; // Limpia el dato recibido.
    }
    if(actualizado2){
        Serial.print("Respuesta_2: ");
        Serial.println(receivedData2);
        receivedData2 = 0; // Limpia el dato recibido.
    }
    */
    if(actualizado1){
        tankData = tank.getStructure();
        tankData2 = tank.getStructure2();

        if(receivedData1 == 43){ // 40 <= receivedData1 && receivedData1 < 45
            digitalWrite(LED_PIN_4, HIGH);
        } else {
            digitalWrite(LED_PIN_4, LOW);
        }
        if(receivedData2 == 32){ // 40 <= receivedData2 && receivedData2 < 45
            digitalWrite(LED_PIN_5, HIGH);
        } else {
            digitalWrite(LED_PIN_5, LOW);
        }
        
        receivedData1 = 0; // Limpia el dato recibido.
        receivedData2 = 0; // Limpia el dato recibido.
        actualizado1 = 0; // Flag reset.
        actualizado2 = 0; // Flag reset.
    }
}

// Path: src/comparator.cpp