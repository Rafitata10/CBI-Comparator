/*
  * Rafael Ramírez Salas
  * Ingeniería de Computadores, Universidad de Málaga
  * Trabajo de Fin de Grado 2024: Fail Tolerant DualNano
*/

#include <SlaveSPI.h>

SlaveSPI::SlaveSPI(uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t ss){
    _mosi = mosi;
    _miso = miso;
    _sck = sck;
    _ss = ss;
    _delay = 2;
    _order = MSBFIRST;
}

// Inicializa la comunicación SPI del slave.
void SlaveSPI::beginSlave(){
    pinMode(_mosi, INPUT);
    pinMode(_miso, OUTPUT);
    pinMode(_sck, INPUT);
    pinMode(_ss, INPUT);
    digitalWrite(_miso, LOW); // Asegura que MISO está en estado bajo por defecto.

    setClockDivider(SPI_CLOCK_DIV32);  // Divide el reloj entre 32.
}

// Desactiva la comunicación SPI del slave.
void SlaveSPI::endSlave(){
    pinMode(_miso, INPUT); // Desactiva el pin MISO.
}

// Establece el orden de los bits.
void SlaveSPI::setBitOrder(uint8_t order){
    _order = order & 1;
}

// Establece el divisor de reloj.
void SlaveSPI::setClockDivider(uint32_t div){
    switch(div){
        case SPI_CLOCK_DIV2:
            _delay = 2;
            break;
        case SPI_CLOCK_DIV4:
            _delay = 4;
            break;
        case SPI_CLOCK_DIV8:
            _delay = 8;
            break;
        case SPI_CLOCK_DIV16:
            _delay = 16;
            break;
        case SPI_CLOCK_DIV32:
            _delay = 32;
            break;
        case SPI_CLOCK_DIV64:
            _delay = 64;
            break;
        case SPI_CLOCK_DIV128:
            _delay = 128;
            break;
        default:
            _delay = 128;
            break;
    }
}

// Devuelve true si el esclavo está seleccionado.
bool SlaveSPI::isSelected(){
    return digitalRead(_ss) == LOW; // Asumiendo activo en bajo.
}

// Espera un tiempo determinado.
void SlaveSPI::wait(uint_fast8_t del){
    for(uint_fast8_t i = 0; i < del; i++){
        asm volatile("nop");
    }
}

/*
// Transfiere un byte. (Versión con depuración)
uint8_t SlaveSPI::transfer(uint8_t valToSend){
    uint8_t received = 0;
    //String debugBitsSent = ""; // DEBUG-Para almacenar los bits enviados.
    String debugBitsReceived = ""; // DEBUG-Para almacenar los bits recibidos
    
    for(int i = 7; i >= 0; i--){
        while(digitalRead(_sck) == LOW); // Espera hasta que SCK suba.
        
        wait(_delay); // Espera un poco antes de leer el bit.

        // Recibe un bit del maestro.
        bool bitReceived = digitalRead(_mosi) == HIGH;
        if(bitReceived){
            received |= (1 << i);
        }
        debugBitsReceived += bitReceived ? "1" : "0"; // DEBUG-Almacena el bit recibido.
 
        // Envía un bit al maestro.
        //bool bitToSend = (valToSend & (1 << i)) ? HIGH : LOW;
        digitalWrite(_miso, (valToSend & (1 << i)) ? HIGH : LOW);
        // debugBitsSent += bitToSend ? "1" : "0"; // DEBUG-Almacena el bit enviado.

        while(digitalRead(_sck) == HIGH); // Espera hasta que SCK baje.
    }
    //Serial.println(debugBitsSent); // DEBUG-Imprime los bits enviados.
    Serial.println(debugBitsReceived); // DEBUG-Imprime los bits recibidos.
    return received; // Devuelve el byte recibido del master.
}
*/

// Transfiere un solo bit.
void SlaveSPI::transferBit(bool bitToSend){
    // Espera hasta que SCK suba, indicando el inicio de la transferencia del bit.
    while(digitalRead(_sck) == LOW);

    // Envía el bit al maestro.
    digitalWrite(_miso, bitToSend ? HIGH : LOW);

    // Espera hasta que SCK baje, indicando el fin de la transferencia del bit.
    while(digitalRead(_sck) == HIGH);
}

// Transfiere un byte.
void SlaveSPI::transferByte(uint8_t byteToSend){
    for(int i = 7; i >= 0; i--){
        while(digitalRead(_sck) == LOW); // Espera hasta que SCK suba.
        
        wait(1); // Espera un poco antes de escribir el bit.

        // Envía un bit al maestro.
        digitalWrite(_miso, (byteToSend & (1 << i)) ? HIGH : LOW);

        while(digitalRead(_sck) == HIGH); // Espera hasta que SCK baje.
    }
}

// Transfiere y recibe un byte.
uint8_t SlaveSPI::transfer(uint8_t valToSend){
    uint8_t received = 0;
    
    for(int i = 7; i >= 0; i--){
        while(digitalRead(_sck) == LOW); // Espera hasta que SCK suba.
        
        wait(_delay+8); // Espera un poco antes de leer el bit.

        if(digitalRead(_mosi) == HIGH){ // Recibe un bit del maestro.
            received |= (1 << i);
        }
        
        // Envía un bit al maestro.
        digitalWrite(_miso, (valToSend & (1 << i)) ? HIGH : LOW);

        while(digitalRead(_sck) == HIGH); // Espera hasta que SCK baje.
    }
    return received; // Devuelve el byte recibido del master.
}

// Transfiere un dato de 16 bits como dos bytes.
uint16_t SlaveSPI::transfer16(uint16_t data){
	union {
		uint16_t val;
		struct {
			uint8_t lsb;
			uint8_t msb;
		};
	} in, out;
  
	in.val = data;

	if(_order == MSBFIRST){
		out.msb = transfer(in.msb);
		out.lsb = transfer(in.lsb);
	} else {
		out.lsb = transfer(in.lsb);
		out.msb = transfer(in.msb);
	}

	return out.val;
}

// Path: lib/SlaveSPI/SlaveSPI.cpp