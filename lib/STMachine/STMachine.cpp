/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#include <STMachine.h>
#include <Screen.h>

STMachine::STMachine(){}

void STMachine::initLeds(){
    // Inicializar los pines de los LEDs.
    pinMode(BLUE_LED_4, OUTPUT);
    digitalWrite(BLUE_LED_4, LOW);
    pinMode(GREEN_LED_5, OUTPUT);
    digitalWrite(GREEN_LED_5, LOW);
    pinMode(PIN_RESET, OUTPUT);
}

// Execute the command received.
void STMachine::executeCommand(unsigned char command, unsigned int& step, Screen& myScreen, SRTankData& tankData, SRTankData2& tankData2){
    switch(command){
        case _OPEN_INLET: // Llenar tanque hasta los 1500 litros con una solución ya mezclada de agua con sosa caústica.
            if((tankData2.volume <= 1515) && (step == _STEP0 || step == _STEP6 || step == _STEP7)){
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
        case _TURN_ON_HEAT: // Calentar la solución a una temperatura de 35ºC.
            if((tankData2.temperature <= 37) && (step == _STEP0 || step == _STEP1 || step == _STEP6)){
                step = _STEP1;
                tank.turnOnHeater();
            } else {
                step = _STEP7;
            }
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
        case _TURN_ON_STIR: // Mezclar con el agitador el contenido del tanque, (se removerá hasta que se alcancen los 50ºC).
            if((tankData2.temperature <= 52) && (step == _STEP2 || step == _STEP3)){
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
        case _TURN_ON_COOL: // Enfriar la mezcla hasta los 25ºC ya que al tratarse de una reacción exotérmica, se calentará.
            if((tankData2.temperature >= 23) && (step == _STEP3 || step == _STEP4)){
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
                digitalWrite(PIN_RESET, LOW); // Reset the reset pin.
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
            myScreen.showStep(step);
            tank.openOutletValve();
            tank.closeInletValve();
            tank.turnOffHeater();
            tank.turnOffCooler();
            tank.turnOffAgitator();
            while(tankData2.volume >= 100){
                tank.refreshData(tankData, tankData2);
                delay(1000);
            }
            tank.closeOutletValve();
            break;
        default: // Ignorar.
            step = _STEP0;
            break;
    }
}

// Reset the crazy master.
void STMachine::reset(unsigned char master, unsigned int step, unsigned int& ack1, unsigned int& ack2){
    // digitalWrite(PIN_RESET, HIGH);
    if(master == 1){
        ack1 = convertir(step); // Reset the first master.
    } else {
        ack2 = convertir(step); // Reset the second master.
    }
}

// Define the function "convertir".
unsigned int STMachine::convertir(unsigned int code){
    unsigned int resp = _RESET0;
    switch(code){
        case _STEP0:
            resp = _RESET0; break;
        case _STEP1:
            resp = _RESET1; break;
        case _STEP2:
            resp = _RESET2; break;
        case _STEP3:
            resp = _RESET3; break;
        case _STEP4:
            resp = _RESET4; break;
        case _STEP5:
            resp = _RESET5; break;
        case _STEP6:
            resp = _RESET6; break;
        case _STEP7:
            resp = _RESET7; break;    
        default:
            break;
    }
    return resp;
}

// Reset the flags.
void STMachine::flagReset(unsigned int& receivedData1, unsigned int& receivedData2, unsigned char& actualizado1, unsigned char& actualizado2){
    receivedData1 = 0; // Limpia el dato recibido.
    receivedData2 = 0; // Limpia el dato recibido.    
    actualizado1 = 0; // Limpia el flag de actualización.
    actualizado2 = 0; // Limpia el flag de actualización.
}

// Reset the acks after using them and before editing them.
void STMachine::resetAcks(unsigned int& ack1, unsigned int& ack2){
    ack1 = _RESPONSE; // Default value for the first master.
    ack2 = _RESPONSE; // Default value for the second master.
    digitalWrite(BLUE_LED_4, LOW); // Reset the blue led.
}

// Path: lib/STMachine/src/STMachine.h