  while (1) {
      // RED ON (5 seconds)
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);   // RED
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // YELLOW
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); // GREEN
      HAL_Delay(5000);

      // GREEN ON (3 seconds)
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_Delay(3000);

      // YELLOW ON (2 seconds)
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
      HAL_Delay(2000);

  }
