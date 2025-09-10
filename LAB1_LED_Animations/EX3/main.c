HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_RESET);
  
  while (1)
  {
    // -------- NS green 3s, EW red --------
    // NS: G ON, R/Y OFF
    HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_SET);
    // EW: R ON, Y/G OFF
    HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_RESET);
    HAL_Delay(3000);

    // -------- NS yellow 2s, EW red --------
    // NS: Y ON, R/G OFF
    HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_RESET);
    // EW: keep R ON
    HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_RESET);
    HAL_Delay(2000);

    // -------- EW green 3s, NS red --------
    // NS: R ON, Y/G OFF
    HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_RESET);
    // EW: G ON, R/Y OFF
    HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_SET);
    HAL_Delay(3000);

    // -------- EW yellow 2s, NS red --------
    // NS: keep R ON
    HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_RESET);
    // EW: Y ON, R/G OFF
    HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_RESET);
    HAL_Delay(2000);

    // tiny blanking to avoid any ghosting in sim
    HAL_GPIO_WritePin(NS_R_GPIO_Port, NS_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_Y_GPIO_Port, NS_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NS_G_GPIO_Port, NS_G_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_R_GPIO_Port, EW_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_Y_GPIO_Port, EW_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EW_G_GPIO_Port, EW_G_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
  }
