/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include <SRTank.h>
#include <Screen.h>

// Define los comandos para los resets.
#define _RESET0 10 // Reset to step 0.
#define _RESET1 11 // Reset to step 1.
#define _RESET2 12 // Reset to step 2.
#define _RESET3 13 // Reset to step 3.
#define _RESET4 14 // Reset to step 4.
#define _RESET5 15 // Reset to step 5.
#define _RESET6 16 // Reset to step 6.
#define _RESET7 17 // Reset to step 7.
// Define los pines para los LEDs.
#define BLUE_LED_4  4
#define GREEN_LED_5 5
// Define la respuesta para los masters.
#define _RESPONSE 35
// Define los masters.
#define MASTER_1 1 // Master 1.
#define MASTER_2 2 // Master 2.
#define PIN_RESET 13 // Pin para el reset.

class STMachine {
  public:
    STMachine();
    void initLeds();
    void executeCommand(unsigned char command, unsigned int& step, Screen& myScreen, SRTankData& tankData, SRTankData2& tankData2);
    void refreshData(SRTankData& tankData, SRTankData2& tankData2);
    void reset(unsigned char master, unsigned int step, unsigned int& ack1, unsigned int& ack2);
    void flagReset(unsigned int& receivedData1, unsigned int& receivedData2, unsigned char& actualizado1, unsigned char& actualizado2);
    void resetAcks(unsigned int& ack1, unsigned int& ack2);

  private:
    unsigned int convertir(unsigned int code);
};

#endif

// Path: lib/STMachine/src/STMachine.h