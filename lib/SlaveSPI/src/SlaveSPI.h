/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#ifndef _SLAVESPI_H
#define _SLAVESPI_H

#include <Arduino.h>
#include <SPI.h>

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

class SlaveSPI {
    public:
        SlaveSPI(uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t ss);
        void beginSlave();
        void endSlave();
        void setBitOrder(uint8_t);
        void setClockDivider(uint32_t);
        bool isSelected();
        void transferBit(bool bitToSend);
        void transferByte(uint8_t byteToSend);
        uint8_t transfer(uint8_t val);
        uint16_t transfer16(uint16_t data);
        
        template <typename T> unsigned int WriteAnything(const T& value){
            const byte *p = (const byte*) &value;
            unsigned int i;
            for(i = 0; i < sizeof value; i++){
                this->transfer(*p++); // Usa el transfer de SoftSlave.
            }
            return i;
        }

        template <typename T> unsigned int ReadAnything(T& value){
            byte *p = (byte*) &value;
            unsigned int i;
            for(i = 0; i < sizeof value; i++){
                *p++ = this->transfer(0); // Usa el transfer de SoftSlave.
            }
            return i;
        }

    private:
        void wait(uint_fast8_t del);

    private:
        uint8_t _miso;
        uint8_t _mosi;
        uint8_t _sck;
        uint8_t _ss;
        uint8_t _delay;
        uint8_t _order;
};
#endif

// Path: lib/SlaveSPI/SlaveSPI.h