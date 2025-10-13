/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define R1_W_Pin GPIO_PIN_0
#define R1_W_GPIO_Port GPIOA
#define R2_N_Pin GPIO_PIN_1
#define R2_N_GPIO_Port GPIOA
#define R3_E_Pin GPIO_PIN_2
#define R3_E_GPIO_Port GPIOA
#define R4_S_Pin GPIO_PIN_3
#define R4_S_GPIO_Port GPIOA
#define Y1_W_Pin GPIO_PIN_4
#define Y1_W_GPIO_Port GPIOA
#define Y2_N_Pin GPIO_PIN_5
#define Y2_N_GPIO_Port GPIOA
#define Y3_E_Pin GPIO_PIN_6
#define Y3_E_GPIO_Port GPIOA
#define Y4_S_Pin GPIO_PIN_7
#define Y4_S_GPIO_Port GPIOA
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOB
#define BTN3_Pin GPIO_PIN_2
#define BTN3_GPIO_Port GPIOB
#define SEG1_Pin GPIO_PIN_10
#define SEG1_GPIO_Port GPIOB
#define SEG2_Pin GPIO_PIN_11
#define SEG2_GPIO_Port GPIOB
#define SEG3_Pin GPIO_PIN_12
#define SEG3_GPIO_Port GPIOB
#define SEG4_Pin GPIO_PIN_13
#define SEG4_GPIO_Port GPIOB
#define SEG5_Pin GPIO_PIN_14
#define SEG5_GPIO_Port GPIOB
#define SEG6_Pin GPIO_PIN_15
#define SEG6_GPIO_Port GPIOB
#define G1_W_Pin GPIO_PIN_8
#define G1_W_GPIO_Port GPIOA
#define G2_N_Pin GPIO_PIN_9
#define G2_N_GPIO_Port GPIOA
#define G3_E_Pin GPIO_PIN_10
#define G3_E_GPIO_Port GPIOA
#define G4_S_Pin GPIO_PIN_11
#define G4_S_GPIO_Port GPIOA
#define EN0_Pin GPIO_PIN_5
#define EN0_GPIO_Port GPIOB
#define EN1_Pin GPIO_PIN_6
#define EN1_GPIO_Port GPIOB
#define EN2_Pin GPIO_PIN_7
#define EN2_GPIO_Port GPIOB
#define EN3_Pin GPIO_PIN_8
#define EN3_GPIO_Port GPIOB
#define SEG0_Pin GPIO_PIN_9
#define SEG0_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
