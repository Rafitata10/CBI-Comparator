/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#include <Screen.h>

Screen::Screen() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire){}
Adafruit_NeoPixel strip(NUMLEDS, BLUE_LED_4, NEO_GRB + NEO_KHZ800);

// Initialize the screen and the LEDs ring.
void Screen::initialize(){
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){ // Verifica la dirección I2C.
        for(;;); // Bucle infinito si la pantalla no se pudo inicializar.
    }
    display.display();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    showStep(_STEP6);

    // Initialize the LEDs ring.
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'.
}

// Show the current step on the screen.
void Screen::showStep(unsigned int step){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Step ");
    display.println(step);

    display.setCursor(0, 32);
    switch(step){
        case _STEP0: 
        case _STEP2: display.println("Filling");  break;
        case _STEP1: display.println("Heating");  break;
        case _STEP3: display.println("Mixing");   break;
        case _STEP4: display.println("Cooling");  break;
        case _STEP5: display.println("Emptying"); break;
        case _STEP6: display.println("Idle");     break;
        case _STEP7: display.println("ERROR!");   break;

        default: display.println("Unknown"); break;
    }
    display.display();
}

// Turn on the LEDs ring.
void Screen::turnOnRing(){
    for(int i = 0; i < NUMLEDS; i++){
        strip.setPixelColor(i, strip.Color(0, 0, 255));
    }
    strip.Color(0, 0, 0);
    strip.show();
}

// Turn off the LEDs ring.
void Screen::turnOffRing(){
    for(int i = 0; i < NUMLEDS; i++){
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

// Path: lib/Screen/src/Screen.cpp