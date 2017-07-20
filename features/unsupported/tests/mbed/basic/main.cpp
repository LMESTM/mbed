/**
 ******************************************************************************
 * @file    main.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    2-December-2016
 * @brief   Simple Example application for using the X_NUCLEO_IKS01A1 
 *          MEMS Inertial & Environmental Sensor Nucleo expansion board.
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
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
*/ 

/* Includes */
#include "mbed.h"
#include "LPS22HBSensor.h"
/* Instantiate the expansion board */

/* Simple main function */
int main() {

  uint8_t id;
  float value1, value2;
  DigitalOut SensorPower(A0);

  SensorPower = 0;
    wait(1);
  SensorPower = 1; 
    wait(1);

  printf ("\n\rRunning SPI3W_Test program\n\r");
  
#define SPI3W_TEST // undef to use SPI 4-wires
#ifdef SPI3W_TEST   
   SPI3W sens_intf(PB_15, NC, PB_13); // SPI2 on L476RG - 3-wires mosi and sclk   
#else //!SPI3W_TEST
   SPI sens_intf(PB_15, PB_14, PB_13); // 4-wires mosi, miso, sclk 
#endif
LPS22HBSensor press_temp(&sens_intf, PA_10);   // PA_10/D2 as CS
  
  printf("\r\n--- Starting new run ---\r\n");
    
  /* Enable  sensors */
  press_temp.enable();
  press_temp.read_id(&id);
  printf("LPS22HB  pressure & temperature   = 0x%X\r\n", id);
 
  while(1) {
    
    printf("\r\n");
    press_temp.get_temperature(&value1);
    press_temp.get_pressure(&value2);
    printf("LPS22HB: [temp] %7f C, [press] %f mbar\r\n", value1,value2);

    printf("---\r\n");
    wait(1.5);
  }
}