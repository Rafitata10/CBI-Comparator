// Rafael Ramírez Salas
// Ingeniería de Computadores, Universidad de Málaga
// Trabajo de Fin de Grado 2024: Fail Tolerant DualNano

#ifndef SRTank_h
#define SRTank_h

#include <Arduino.h>

// Digital inputs.
#define _INPUT_VALVE  0x00 // Inlet valve
#define _OUTPUT_VALVE 0x01 // Outlet valve
#define _HEATER       0x02 // Heater
#define _COOLER       0x03 // Cooler
#define _AGITATOR     0x04 // Stirrer
// Digital outputs.
#define _HIGH_FLOATER 0x10 // High floater
#define _LOW_FLOATER  0x11 // Low floater
#define _MIN_TEMP     0x12 // Low temperature
#define _MAX_TEMP     0x13 // High temperature
#define _REVOLUTIONS  0x14 // Revolutions
// Analog inputs.
#define _TEMP_IN_MIN  0x20 // Minimal temperature alarm trigger level
#define _TEMP_IN_MAX  0x21 // Maximal temperature alarm trigger level
// Analog outputs.
#define _VOLUME       0x30 // Volume
#define _TEMP         0x31 // Temperature
// Actions.
#define _TURN_ON      0x01 // Turn on
#define _TURN_OFF     0x00 // Turn off
// Commands.
#define _OPEN_INLET    40 // Open inlet valve
#define _CLOSE_INLET   41 // Close inlet valve
#define _OPEN_INLET2   50 // Open inlet valve (for second time)
#define _CLOSE_INLET2  51 // Close inlet valve (for second time)
#define _OPEN_OUTLET   42 // Open outlet valve
#define _CLOSE_OUTLET  43 // Close outlet valve
#define _TURN_ON_HEAT  44 // Turn on heater
#define _TURN_OFF_HEAT 45 // Turn off heater
#define _TURN_ON_COOL  46 // Turn on cooler
#define _TURN_OFF_COOL 47 // Turn off cooler
#define _TURN_ON_STIR  48 // Turn on stirrer
#define _TURN_OFF_STIR 49 // Turn off stirrer
#define _STAY_CHILL    52 // Stay Chill
#define _AUXILIAR      59 // Auxiliar
#define _ABORT         69 // ABORT
// Steps:
#define _STEP0 0 // Llenar tanque hasta los 1500 litros con una solución ya mezclada de agua con sosa caústica.
#define _STEP1 1 // Calentar la solución a una temperatura de 35ºC.
#define _STEP2 2 // Rellenar con aceites hasta los 3000 litros.
#define _STEP3 3 // Mezclar con el agitador el contenido del tanque, (se removerá hasta que se alcancen los 50ºC).
#define _STEP4 4 // Enfriar la mezcla hasta los 25ºC ya que al tratarse de una reacción exotérmica, se calentará.
#define _STEP5 5 // Abrir la válvula de vaciado del tanque para procesar el resultado en una maquinaria que dividirá y empaquetará el jabón.
#define _STEP6 6 // Finish. (Wait for a button push).
#define _STEP7 7 // ERROR State.
// Define el intervalo de tiempo.
#define _INVERVAL 500 // 1000

struct SRTankData {
    unsigned char highFloater;
    unsigned char lowFloater;
    unsigned char minTemperature;
    unsigned char maxTemperature;
    unsigned char revolutions;
}; extern SRTankData tankData;

struct SRTankData2 {
    unsigned int popurri;
    unsigned int volume;
    unsigned int temperature;
}; extern SRTankData2 tankData2;

class SRTank {
public:
    // Constructor.
    SRTank();
    // Initialize the tank.
    void initSRTank();
    // Refresh for the tank.
    void refreshData(SRTankData& tankData, SRTankData2& tankData2);
    void refreshTank(unsigned long& previousMillis);
    // Getters para los outputs digitales.
    unsigned char getHighFloater();
    unsigned char getLowFloater();
    unsigned char getMaxTemperature();
    unsigned char getMinTemperature();
    unsigned char getRevolutions();
    // Getters para los outputs analógicos.
    unsigned int getVolume();
    unsigned int getTemperature();
    // Setters para los outputs digitales.
    void setVolume(void);
    void setTemperature(void);
    void setHighFloater(void);
    void setLowFloater(void);
    void setMaxTemperature(void);
    void setMinTemperature(void);
    void setRevolutions(void);
    // Functions with digital outputs.
    void openInletValve(void);
    void closeInletValve(void);
    void openOutletValve(void);
    void closeOutletValve(void);
    void turnOnHeater(void);
    void turnOffHeater(void);
    void turnOnCooler(void);
    void turnOffCooler(void);
    void turnOnAgitator(void);
    void turnOffAgitator(void);
    // Setters para los outputs analógicos.
    void setMaxTemperatureAlarm(unsigned int temperature);
    void setMinTemperatureAlarm(unsigned int temperature);
    // New functions.
    void fillTank(void);
    void fillTank(unsigned int volume);
    void emptyTank(void);
    void emptyTank(unsigned int volume);
    void heatTank(void);
    void heatTank(unsigned int temperature);
    void coolTank(void);
    void coolTank(unsigned int temperature);
    void stirTank(void);
    void stirTank(unsigned int time);
    // Functions with the Tank.
    SRTankData getInit(void);
    SRTankData2 getInit2(void);
    SRTankData getStructure(void);
    SRTankData2 getStructure2(void);
    void setStructure(void);

public:
    // Digital outputs.
    bool highFloater;
    bool lowFloater;
    bool minTemperature;
    bool maxTemperature;
    bool revolutions;
    // Analog outputs.
    unsigned int volume;
    unsigned int temperature;

private:
    // Auxiliar values.
    unsigned char valueH, valueL;
    unsigned int maxVolume = 3000, minVolume = 0, maxTemp = 150, 
    minTemp = 1, maxTempAlarm = 125, minTempAlarm = 10, stirTime = 1000;
    // Auxiliar functions.
    void packTank();

}; extern SRTank tank; // Declaración externa para uso global.

#endif

// Path: lib/SRTank/SRTank.h