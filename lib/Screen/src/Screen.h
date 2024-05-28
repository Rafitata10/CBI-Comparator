/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#ifndef SCREEN_H
#define SCREEN_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SRTank.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128 // Ancho en píxeles.
#define SCREEN_HEIGHT 64 // Altura en píxeles.

#define BLUE_LED_4  4 // Pin para el LED azul.
#define NUMLEDS 8 // Número de LEDs del anillo.

class Screen {
public:
    Screen();
    void initialize();
    void showStep(unsigned int step);
    void turnOnRing();
    void turnOffRing();
private:
    Adafruit_SSD1306 display;
};

#endif

// Path: lib/Screen/src/Screen.h