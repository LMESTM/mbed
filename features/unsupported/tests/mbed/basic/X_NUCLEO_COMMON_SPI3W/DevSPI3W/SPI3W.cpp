/**
 ******************************************************************************
 * @file    SPI3W.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    5 August 2016
 * @brief   Implementation of an SPI 3 wires driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
 
#include "mbed.h"
#include "SPI3W.h"
//#include "SensorTile.h"

         
/* Class Implementation ------------------------------------------------------*/

/** Constructor
 * @param spi3w object of an helper class which handles the spi peripheral
 * @param 
 */
 
 SPI3W::SPI3W (PinName spi_sda, PinName spi_sdi, PinName spi_clk) : SPI(spi_sda, spi_sdi, spi_clk, NC) 
{
}


uint8_t SPI3W::Sensor_IO_SPI_CS_Enable(DigitalOut * _cs_pin)
{
   *_cs_pin = 0;
     return 0;
}

uint8_t SPI3W::Sensor_IO_SPI_CS_Disable(DigitalOut * _cs_pin)
{
   *_cs_pin = 1;
     return 0;
}   


/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t SPI3W::Sensor_IO_SPI_Write( DigitalOut * _cs_pin, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
{
  uint8_t i;
    
// Select the correct device
  Sensor_IO_SPI_CS_Enable(_cs_pin);

    write(WriteAddr);
    
    write((const char *)pBuffer, (int) nBytesToWrite, NULL, 0);

// Deselect the device
  Sensor_IO_SPI_CS_Disable(_cs_pin);
  
  return 0;
}

/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t SPI3W::Sensor_IO_Write( DigitalOut * _cs_pin, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
{
   return Sensor_IO_SPI_Write( _cs_pin, WriteAddr, pBuffer, nBytesToWrite );
}

/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t SPI3W::Sensor_IO_SPI_Read( DigitalOut * _cs_pin, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{   
  /* Select the correct device */
  Sensor_IO_SPI_CS_Enable(_cs_pin);
  uint8_t TxByte = ReadAddr | 0x80;

  write((const char *)&TxByte, 1, (char *)pBuffer, (int) nBytesToRead);

  /* Deselect the device */
  Sensor_IO_SPI_CS_Disable(_cs_pin);  
  
  return 0;
}


/**
 * @brief  Reads from the sensor to a buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t SPI3W::Sensor_IO_Read( DigitalOut * _cs_pin, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
  return Sensor_IO_SPI_Read( _cs_pin, ReadAddr, pBuffer, nBytesToRead );
}


uint8_t SPI3W::spi3w_read(uint8_t * pBuffer, DigitalOut * _cs_pin, uint8_t RegisterAddr, uint16_t NumByteToRead)
{
     return Sensor_IO_Read( _cs_pin, RegisterAddr, pBuffer, NumByteToRead );        
}

uint8_t SPI3W::spi3w_write(uint8_t * pBuffer, DigitalOut * _cs_pin, uint8_t RegisterAddr, uint16_t NumByteToWrite)
{
     return Sensor_IO_Write( _cs_pin, RegisterAddr, pBuffer, NumByteToWrite );
}

