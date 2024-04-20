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

// Define los pines para mySlave_1.
#define MOSI_PIN_10 10
#define MISO_PIN_11 11
#define SCK_PIN_12 12
#define SS_PIN_3 3
// Define los pines para mySlave_2.
#define MOSI_PIN_7 7
#define MISO_PIN_8 8
#define SCK_PIN_9 9
#define SS_PIN_2 2
// Define los pines para los pines de sincronización.
#define SYNC_1 13 // Master 1.
#define SYNC_2 6  // Master 2.
// Define los pines para los LEDs.
#define BLUE_LED_4 4
#define GREEN_LED_5 5

// Instancias de la clase SoftSlave.
SlaveSPI slaveSPI_1(MOSI_PIN_10, MISO_PIN_11, SCK_PIN_12, SS_PIN_3);
unsigned char master1 = 0x1; // Id for the first master.
SlaveSPI slaveSPI_2(MOSI_PIN_7, MISO_PIN_8, SCK_PIN_9, SS_PIN_2);
unsigned char master2 = 0x2; // Id for the second master.
// Instancia de la clase SRTank.
SRTankData tankData = tank.getInit();
SRTankData2 tankData2 = tank.getInit2();
// Instancia de la clase Screen.
Screen myScreen;

// Variables para manejar los datos recibidos.
unsigned char receivedData1 = 0;
unsigned char receivedData2 = 0;
// Flags para saber si se han actualizado los datos.
unsigned char actualizado1 = 0;
unsigned char actualizado2 = 0;
// Respuestas para los masters.
unsigned char ack1 = 35;
unsigned char ack2 = 35;
// Flags para resetear los masters.
unsigned char reset1 = 0;
unsigned char reset2 = 0;
// Variable para el estado del tanque.
unsigned int step = _STEP6;
// Definir intervalo de tiempo para actualizar el tanque.
const unsigned long interval = 1000; // 500
unsigned long previousMillis = 0;
// Variables para manejar las comunicaciones SPI.
volatile bool isSPI1Active = false;
volatile bool isSPI2Active = false;

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
    // Inicializar los pines de los LEDs.
    pinMode(BLUE_LED_4, OUTPUT);
    digitalWrite(BLUE_LED_4, LOW);
    pinMode(GREEN_LED_5, OUTPUT);
    digitalWrite(GREEN_LED_5, LOW);    

    // Initialize the tank.
    tank.initSRTank();

    // Initialize the tank alarm temperature.
    tank.setMaxTemperatureAlarm(125); // Maximum temperature alarm.
    tank.setMinTemperatureAlarm(10); // Minimum temperature alarm.

    // Inicializar las comunicaciones SPI.
    slaveSPI_1.beginSlave();
    slaveSPI_1.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.
    slaveSPI_2.beginSlave();
    slaveSPI_2.setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.

    // Configuración de las interrupciones para los pines SS.
    pinMode(SS_PIN_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_2), SPI1_SS_ISR, FALLING);
    pinMode(SS_PIN_3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SS_PIN_3), SPI2_SS_ISR, FALLING);

    // Synchronization pins configuration.
    pinMode(SYNC_1, OUTPUT);
    pinMode(SYNC_2, OUTPUT);

    // Display setup.
    myScreen.initialize();
}

// Execute the command received.
void executeCommand(unsigned char command){
    switch(command){
        case _OPEN_INLET: // Llenar tanque hasta los 1500 litros con una solución ya mezclada de agua con sosa caústica.
            if(step == _STEP0 || step == _STEP6 || step == _STEP7){
                step = _STEP0;
                tank.openInletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _CLOSE_INLET:
            if(step == _STEP0 || step == _STEP7){
                step = _STEP0;
                tank.closeInletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _TURN_ON_HEAT: // Calentar la solución a una temperatura de 40ºC.
            if(step == _STEP0 || step == _STEP1 || step == _STEP6){
                step = _STEP1;
                tank.turnOnHeater();
            } else {
                step = _STEP7;
            }
            digitalWrite(BLUE_LED_4, LOW);
            digitalWrite(GREEN_LED_5, LOW);
            break;
        case _TURN_OFF_HEAT:
            if(step == _STEP1 || step == _STEP7){
                step = _STEP1;
                tank.turnOffHeater();
            } else {
                step = _STEP7;
            }
            break;
        case _OPEN_INLET2: // Rellenar con aceites hasta los 3000 litros.
            if(step == _STEP1 || step == _STEP2){
                step = _STEP2;
                tank.openInletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _CLOSE_INLET2:
            if(step == _STEP2 || step == _STEP7){
                step = _STEP2;
                tank.closeInletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _TURN_ON_STIR: // Mezclar con el agitador el contenido del tanque, (se removerá hasta que se alcancen los 60ºC).
            if(step == _STEP2 || step == _STEP3){
                step = _STEP3;
                tank.turnOnAgitator();
            } else {
                step = _STEP7;
            }
            break;
        case _TURN_OFF_STIR:
            if(step == _STEP3 || step == _STEP7){
                step = _STEP3;
                tank.turnOffAgitator();
            } else {
                step = _STEP7;
            }
            break;
        case _TURN_ON_COOL: // Enfriar la mezcla hasta los 35ºC ya que al tratarse de una reacción exotérmica, se calentará.
            if(step == _STEP3 || step == _STEP4){
                step = _STEP4;
                tank.turnOnCooler();
            } else {
                step = _STEP7;
            }
            break;
        case _TURN_OFF_COOL:
            if(step == _STEP4 || step == _STEP7){
                step = _STEP4;
                tank.turnOffCooler();
            } else {
                step = _STEP7;
            }
            break;
        case _OPEN_OUTLET: // Abrir la válvula de vaciado del tanque para procesar el resultado, (el jabón).
            if(step == _STEP4 || step == _STEP5){
                step = _STEP5;
                tank.openOutletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _CLOSE_OUTLET:
            if(step == _STEP5 || step == _STEP7){
                step = _STEP5;
                tank.closeOutletValve();
            } else {
                step = _STEP7;
            }
            break;
        case _STAY_CHILL: // Keep the tank in a chill state.
            // Nothing to do, unless de _STEP5 case.
            if(step == _STEP5){
                step = _STEP6;
            }
            break;
        case _ABORT: //  ERROR-Abort
            step = _STEP7;
            tank.openOutletValve();
            tank.closeInletValve();
            tank.turnOffHeater();
            tank.turnOffCooler();
            tank.turnOffAgitator();
            while(tankData2.volume > 0);
            tank.closeOutletValve();
            break;
        default: // Ignorar.
            step = _STEP7;
            break;
    }
}

// Refresh the tank data.
void refreshData(){
    tankData = tank.getStructure();
    tankData2 = tank.getStructure2();
}

// Reset a master.
void reset(unsigned char master){
    if(master == 0x1){
        ack1 = 69; // Reset the first master.
    } else {
        ack2 = 69; // Reset the second master.
    }
}

// Main loop.
void loop(){
    // Máquina de estados para manejar las comunicaciones SPI.
    if(isSPI1Active){ // Sería el master1 y tendría prioridad sobre el master2.
        delay(8); // Espera un poco para iniciar la comunicación.
        receivedData1 = slaveSPI_1.transfer(ack1); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al master.
        if(receivedData1 > 0){
            slaveSPI_1.WriteAnything(tankData2);
            actualizado1 = 1;
        }
        
        isSPI1Active = false; // Termina la comunicación SPI1.
    }
    if(isSPI2Active){ // Sería el master2.
        delay(8); // Espera un poco para iniciar la comunicación.
        receivedData2 = slaveSPI_2.transfer(ack2); // Envía confirmación.

        // Si se recibe el comando esperado, se envía el struct al master.
        if(receivedData2 > 0){
            slaveSPI_2.WriteAnything(tankData2);
            actualizado2 = 1;
        }
        
        isSPI2Active = false; // Termina la comunicación SPI2.
    }
    if(actualizado1 && actualizado2){
        if(receivedData1 == receivedData2){
            if(receivedData1 == 0 && receivedData2 == 0){
                executeCommand(_ABORT); // Se procede a abortar.
            } else if(receivedData1 == _ABORT && receivedData2 == _ABORT){
                executeCommand(_ABORT); // Se procede a abortar.
            } else {
                executeCommand(receivedData1); // Ejecuta el comando recibido.
            }
        } else {
            if(receivedData1 == 0){
                executeCommand(receivedData2); // Ejecuta el comando del master2.
                // master1 reseting...
            } else if(receivedData2 == 0){
                executeCommand(receivedData1); // Ejecuta el comando del master1.
                // master2 reseting...
            } else {
                // ERROR-Discrepancia.
                digitalWrite(BLUE_LED_4, HIGH);
                executeCommand(receivedData1); // Ejecuta el comando del master principal.
                reset(ack2); // Resetea el master2.
                reset2 = 1; // Flag reset.
            }
            executeCommand(receivedData1); // Ejecuta el comando del master principal.
            reset(ack2); // Resetea el master2.
        }
        receivedData1 = 0; // Limpia el dato recibido.
        receivedData2 = 0; // Limpia el dato recibido.        
    } else if(actualizado1 && reset2){  // master2 reseting...
        executeCommand(receivedData1); // Ejecuta el comando del master principal.
        receivedData1 = 0; // Limpia el dato recibido.
        receivedData2 = 0; // Limpia el dato recibido.
    } else if(actualizado2 && reset1){  // master1 reseting...
        executeCommand(receivedData2); // Ejecuta el comando del master principal.
        reset(ack1); // Resetea el master1.
        reset1 = 0; // Flag reset.
        receivedData1 = 0; // Limpia el dato recibido.
        receivedData2 = 0; // Limpia el dato recibido.
    }

    // Comprobar si debo actualizar el tanque.
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval){
        // Actualizar previousMillis con el tiempo actual.
        previousMillis = currentMillis;
        // Actualizar los datos del tanque.
        refreshData();
    }

    // Show the current step on the screen.
    myScreen.showStep(step);
}

// Path: src/comparator.cpp