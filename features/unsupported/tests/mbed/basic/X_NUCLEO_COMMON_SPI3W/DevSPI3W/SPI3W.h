/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_SPI3W_H
#define MBED_SPI3W_H

#include "mbed.h"
#include "Component.h"
#include "SPI.h"

/** A SPI3W Master, used for communicating with SPI3W slave devices
 *
 * The default format is set to 8-bits, mode 0, and a clock frequency of 1MHz
 *
 * Most SPI devices will also require Chip Select and Reset signals. These
 * can be controlled using <DigitalOut> pins
 *
 * @Note Synchronization level: Thread safe
 *
 * Example:
 * @code
 * // Send a byte to a SPI slave, and record the response
 *
 * #include "mbed.h"
 *
 * // hardware ssel (where applicable)
 * //SPI device(p5, p7, p8); // mosi, sclk, ssel
 *
 * // software ssel
 * SPI3W device(p5, p7); // mosi, sclk
 * DigitalOut cs(p8); // ssel
 *
 * int main() {
 *     // hardware ssel (where applicable)
 *     //int response = device.write(0xFF);
 *
 *     device.lock();
 *     // software ssel
 *     cs = 0;
 *     int response = device.spi3w_write(0xFF);
 *     cs = 1;
 *     device.unlock();
 *
 * }
 * @endcode
 */
 
class SPI3W : public SPI {

public:
       SPI3W (PinName spi_sda, PinName spi_sdi, PinName spi_clk);

     uint8_t spi3w_read (uint8_t * pBuffer, DigitalOut * _cs_pin, uint8_t RegisterAddr, uint16_t NumByteToRead);
     uint8_t spi3w_write(uint8_t * pBuffer, DigitalOut * _cs_pin, uint8_t RegisterAddr, uint16_t NumByteToWrite);

private:
     uint8_t Sensor_IO_Read( DigitalOut * _cs_pin, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );
     uint8_t Sensor_IO_SPI_Read( DigitalOut * _cs_pin, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );

     uint8_t Sensor_IO_Write( DigitalOut * _cs_pin, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );
     uint8_t Sensor_IO_SPI_Write( DigitalOut * _cs_pin, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );

     uint8_t Sensor_IO_SPI_CS_Disable(DigitalOut * _cs_pin);
     uint8_t Sensor_IO_SPI_CS_Enable(DigitalOut * _cs_pin);

public:
    virtual ~SPI3W() {
    }

};

#endif

/** @}*/
