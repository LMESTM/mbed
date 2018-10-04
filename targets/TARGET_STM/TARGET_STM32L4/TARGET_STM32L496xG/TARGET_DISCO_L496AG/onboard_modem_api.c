/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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

#if MBED_CONF_NSAPI_PRESENT

#include "onboard_modem_api.h"
#include "PinNames.h"

#if (MODEM_ON_BOARD && (TARGET_P_L496G_CELL01 || TARGET_P_L496G_CELL02))

/* C2C module Reset pin definitions */
#define C2C_RST_PIN                        GPIO_PIN_2
#define C2C_RST_GPIO_PORT                  GPIOB
#define C2C_RST_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOB_CLK_ENABLE()

/* C2C module PowerKey pin definitions */
#define C2C_PWRKEY_PIN                     GPIO_PIN_3
#define C2C_PWRKEY_GPIO_PORT               GPIOD
#define C2C_PWRKEY_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()

/* C2C SIM Reset pin definitions */
#define C2C_SIM_RST_PIN                    GPIO_PIN_7
#define C2C_SIM_RST_GPIO_PORT              GPIOC
#define C2C_SIM_RST_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

/* C2C SIM Clock pin definitions */
#define C2C_SIM_CLK_PIN                    GPIO_PIN_4
#define C2C_SIM_CLK_GPIO_PORT              GPIOA
#define C2C_SIM_CLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

/* C2C SIM DATA pin definitions */
#define C2C_SIM_DATA_PIN                   GPIO_PIN_12
#define C2C_SIM_DATA_GPIO_PORT             GPIOB
#define C2C_SIM_DATA_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()

/* C2C SIM Selection_0 pin definitions */
#define C2C_SIM_SEL0_PIN                   GPIO_PIN_2
#define C2C_SIM_SEL0_GPIO_PORT             GPIOC
#define C2C_SIM_SEL0_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()

/* C2C SIM Selection_1 pin definitions */
#define C2C_SIM_SEL1_PIN                   GPIO_PIN_3
#define C2C_SIM_SEL1_GPIO_PORT             GPIOI
#define C2C_SIM_SEL1_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOI_CLK_ENABLE()


typedef enum {
  SIM_EXT_SLOT,
  SIM_EMBEDDED
} C2C_SimSlot_t;


/**
  * @brief Set the pins related to the SIM to input no pull and enable the clocks
  * @retval None
  */
void C2C_ResetSIMConnection()
{
  GPIO_InitTypeDef  GPIO_InitStructSimInputs;

  C2C_SIM_RST_GPIO_CLK_ENABLE();
  C2C_SIM_CLK_GPIO_CLK_ENABLE();
  C2C_SIM_DATA_GPIO_CLK_ENABLE();
  
  /* SIM RESET setting (PC7)*/
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_RST_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_RST_GPIO_PORT, &GPIO_InitStructSimInputs);
  
  /* SIM CLK setting (PA4)*/
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_CLK_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_CLK_GPIO_PORT, &GPIO_InitStructSimInputs);
  
  /* SIM DATA setting (PB12) */
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_DATA_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_DATA_GPIO_PORT, &GPIO_InitStructSimInputs);
}

/**
  * @brief Select which SIM to use
  * @param sim: SIM_EXT_SLOT or SIM_EMBEDDED
  * @retval None
  */
void C2C_SimSelect(C2C_SimSlot_t sim)
{
  GPIO_InitTypeDef  GPIO_InitStructSimSelect;

  C2C_SIM_SEL0_GPIO_CLK_ENABLE();
  C2C_SIM_SEL1_GPIO_CLK_ENABLE();

  C2C_ResetSIMConnection();
  HAL_Delay(50);
  
  /* SIM selection */
  switch( sim )
  {
  case SIM_EXT_SLOT :
    
    /* Sim_select0 */
    /* S */
    /* LOW */
    /* STMOD+, pin 18 (PC2) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL0_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL0_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL0_GPIO_PORT, C2C_SIM_SEL0_PIN, GPIO_PIN_RESET);

    /* Sim_select1 */
    /* OE */
    /* LOW */
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL1_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL1_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL1_GPIO_PORT, C2C_SIM_SEL1_PIN, GPIO_PIN_RESET);
    break;
    
   case SIM_EMBEDDED :
  
    /* Sim_select0 */
    /* S */
    /* HIGH */
    /* STMOD+, pin 18 (PC2)*/
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL0_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL0_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL0_GPIO_PORT, C2C_SIM_SEL0_PIN, GPIO_PIN_SET);

    /* Sim_select1 */
    /* OE */
    /* LOW */
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL1_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL1_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL1_GPIO_PORT, C2C_SIM_SEL1_PIN, GPIO_PIN_RESET);
    break;
  }
}

void onboard_modem_init()
{
    printf("onboard_modem_init\r\n");
#if (MODEM_ON_BOARD_SIM_SLOT == 0)
	C2C_SimSelect(SIM_EXT_SLOT);
#else
	C2C_SimSelect(SIM_EMBEDDED);
#endif
}


void onboard_modem_deinit()
{
    printf("onboard_modem_deinit\r\n");
}

void onboard_modem_power_up()
{
 GPIO_InitTypeDef  GPIO_InitStructReset, GPIO_InitStructPwr;

  printf("onboard_modem_power_up\r\n");

  C2C_RST_GPIO_CLK_ENABLE();
  C2C_PWRKEY_GPIO_CLK_ENABLE();
  
  /* RESET sequence */
  /* STMOD+ IO12 */
  /* LOW */
  GPIO_InitStructReset.Pin       = C2C_RST_PIN;
  GPIO_InitStructReset.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructReset.Pull      = GPIO_PULLUP;
  GPIO_InitStructReset.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_RST_GPIO_PORT, &GPIO_InitStructReset);
  
  /* PWRKEY */
  /* STMOD+ IO9 */
  /* at least LOW during 100 ms */
  GPIO_InitStructPwr.Pin       = C2C_PWRKEY_PIN;
  GPIO_InitStructPwr.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPwr.Pull      = GPIO_PULLUP;
  GPIO_InitStructPwr.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_PWRKEY_GPIO_PORT,&GPIO_InitStructPwr);

#ifdef TARGET_P_L496G_CELL01
  /* default case is UG96 modem */
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(150);

  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_SET);
  HAL_Delay(150);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_RESET);
  /* Waits for Modem complete its booting procedure */
  HAL_Delay(2300);
#endif
#ifdef TARGET_P_L496G_CELL02
  /* BG96 case */
  /* POWER DOWN */
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(150);
  /* POWER UP */
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN,GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN,GPIO_PIN_RESET);
  /* Waits for Modem complete its booting procedure */
  HAL_Delay(5000);
#endif

}

void onboard_modem_power_down()
{
    printf("onboard_modem_power_down\r\n");

}
#endif //MODEM_ON_BOARD
#endif //MBED_CONF_NSAPI_PRESENT
