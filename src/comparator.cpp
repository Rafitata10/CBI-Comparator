/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

// Incluye las librerías necesarias.
#include <Arduino.h>
#include <SlaveSPI.h>
#include <SRTank.h>
#include <Screen.h>
#include <STMachine.h>

// Instancias de la clase SoftSlave.
SlaveSPI slaveSPI_1(MOSI_PIN_10, MISO_PIN_11, SCK_PIN_12, SS_PIN_3);
SlaveSPI slaveSPI_2(MOSI_PIN_7, MISO_PIN_8, SCK_PIN_9, SS_PIN_2);
// Instancia de la clase SRTank.
SRTankData tankData = tank.getInit();
SRTankData2 tankData2 = tank.getInit2();
// Instancia de la clase Screen.
Screen myScreen;
// Instancia de la clase StateMachine.
STMachine mySTMachine;

// Variables para manejar los datos recibidos.
unsigned int receivedData1 = 0;
unsigned int receivedData2 = 0;
// Flags para saber si se han actualizado los datos.
unsigned char actualizado1 = 0;
unsigned char actualizado2 = 0;
// Respuestas para los masters.
unsigned int ack1 = _RESPONSE;
unsigned int ack2 = _RESPONSE;
// Variable para el estado del tanque.
unsigned int step = _STEP6;
unsigned int prevStep = _STEP6;
// Definir intervalo de tiempo para actualizar el tanque.
unsigned long previousMillis = 0;
// Variables para manejar las comunicaciones SPI.
volatile bool isSPI1Active = false;
volatile bool isSPI2Active = false;
// Auxiliar flag for rebooting.
unsigned int reboot = 0;

// ISR para SS de SPI_1.
void SPI1_SS_ISR(){
    isSPI1Active = true; // Inmediatamente inicia la comunicación SPI1 si es posible.
}
// ISR para SS de SPI_2.
void SPI2_SS_ISR(){
    isSPI2Active = true; // Inmediatamente inicia la comunicación SPI2 si es posible.
}

// Configuración inicial.
void setup(){
    // Inicializar las comunicaciones SPI.
    slaveSPI_1.beginSlave();
    slaveSPI_2.beginSlave();

    // Initialize the tank.
    tank.initSRTank();  
    
    //Initialize the LEDs pins.
    mySTMachine.initLeds();

    // Configuración de las interrupciones para los pines SS.
    pinMode(SS_PIN_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_2), SPI1_SS_ISR, FALLING);
    pinMode(SS_PIN_3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_3), SPI2_SS_ISR, FALLING);

    // Display setup.
    myScreen.initialize();
}

// Main loop.
void loop(){
    // Refresh Comparator Screen.
    if(step != prevStep){
        // Show the current step on the screen.
        myScreen.showStep(step);
        prevStep = step;
    }

    // Máquina de estados para manejar las comunicaciones SPI.
    if(isSPI1Active){ // Sería el master1 y tendría prioridad sobre el master2.
        receivedData1 = slaveSPI_1.transfer(ack1); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al master.
        if(receivedData1 > 0){
            slaveSPI_1.WriteAnything(tankData2);
            actualizado1 = 1;
        }
        isSPI1Active = false; // Termina la comunicación SPI1.
    }
    if(isSPI2Active){ // Sería el master2.
        receivedData2 = slaveSPI_2.transfer(ack2); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al master.
        if(receivedData2 > 0){
            slaveSPI_2.WriteAnything(tankData2);
            actualizado2 = 1;
        }
        isSPI2Active = false; // Termina la comunicación SPI2.
    }

    if(actualizado1 && actualizado2){
        // Reset the acks after using them and before editing them.
        mySTMachine.resetAcks(ack1, ack2, myScreen);
        if(receivedData1 == receivedData2){
            if(receivedData1 == 0){ // Case: 0 & 0
                mySTMachine.executeCommand(_ABORT, step, myScreen, tankData, tankData2); // Se procede a abortar.
            } else if(receivedData1 == _ABORT){ // Case: _ABORT & _ABORT
                mySTMachine.executeCommand(_ABORT, step, myScreen, tankData, tankData2); // Se procede a abortar.
            } else { // Case: Command & Command
                mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando recibido.
            }
            if(reboot == 1){
                reboot = 0;
                digitalWrite(PIN_RESET, LOW); // Reset the reset pin.
            }
        } else {
            if(receivedData1 == 0){ // Case: 0 & Command
                mySTMachine.executeCommand(receivedData2, step, myScreen, tankData, tankData2); // Ejecuta el comando del master2.
                // master1 reseting...
            } else if(receivedData2 == 0){ // Case: Command & 0
                mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando del master1.
                // master2 reseting...
            } else { // Case: Command-A & Command-B
                // Discrepancy detected.
                if(reboot == 0){
                    // myScreen.turnOnRing();
                    digitalWrite(BLUE_LED_4, HIGH);
                    digitalWrite(PIN_RESET, HIGH);
                    mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando del master principal.
                    mySTMachine.reset(MASTER_2, step, ack1, ack2); // Resetea el master2.
                    reboot++;
                } else {
                    mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando del master principal.
                    digitalWrite(PIN_RESET, LOW);
                    reboot = 0;
                }
            }
        }
    // Case: Command & Nothing
    } else if(actualizado1 && !actualizado2){  // master1 absent...
        mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando del master principal.
        // mySTMachine.reset(MASTER_2, step, ack1, ack2); // Resetea el master2.
    // Case: Nothing & Command
    } else if(!actualizado1 && actualizado2){  // master2 absent...
        mySTMachine.executeCommand(receivedData1, step, myScreen, tankData, tankData2); // Ejecuta el comando del master secundario.
        // mySTMachine.reset(MASTER_1, step, ack1, ack2); // Resetea el master1.
    }

    // Flag reset.
    mySTMachine.flagReset(receivedData1, receivedData2, actualizado1, actualizado2); // Reset the flags.

    // Comprobar si debo actualizar el tanque.
    tank.refreshTank(previousMillis);
}

// Path: src/comparator.cpp
