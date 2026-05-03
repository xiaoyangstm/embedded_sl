/**
 * @file stm32f1xx_hal_i2c.c
 * @brief HAL I2C Implementation
 */

#include "stm32f1xx_hal_i2c.h"

static void I2C_ClearADDRFlag(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
  uint32_t pclk1;
  uint32_t freqrange;

  if (hi2c == NULL) {
    return HAL_ERROR;
  }

  /* Enable I2C clock */
  if (hi2c->Instance == I2C1) {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    pclk1 = 36000000;  /* PCLK1 at 36MHz */
  } else if (hi2c->Instance == I2C2) {
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    pclk1 = 36000000;
  }

  /* Software reset */
  hi2c->Instance->CR1 |= I2C_CR1_SWRST;
  hi2c->Instance->CR1 &= ~I2C_CR1_SWRST;

  /* Configure timing */
  freqrange = pclk1 / 1000000;
  hi2c->Instance->CR2 = freqrange;

  /* Configure clock speed */
  if (hi2c->Init.ClockSpeed <= 100000) {
    /* Standard mode */
    hi2c->Instance->CCR = (pclk1 / (hi2c->Init.ClockSpeed * 2)) & 0xFFF;
    hi2c->Instance->TRISE = freqrange + 1;
  } else {
    /* Fast mode */
    hi2c->Instance->CCR = ((pclk1 / (hi2c->Init.ClockSpeed * 3)) & 0xFFF) | 0x8000;
    hi2c->Instance->TRISE = (freqrange * 300 / 1000) + 1;
  }

  /* Configure addressing mode */
  hi2c->Instance->OAR1 = hi2c->Init.OwnAddress1 | hi2c->Init.AddressingMode | 0x4000;

  /* Enable I2C */
  hi2c->Instance->CR1 |= I2C_CR1_PE;

  /* Call MSP init */
  HAL_I2C_MspInit(hi2c);

  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
  if (hi2c == NULL) {
    return HAL_ERROR;
  }

  /* Disable I2C */
  hi2c->Instance->CR1 &= ~I2C_CR1_PE;

  /* Disable I2C clock */
  if (hi2c->Instance == I2C1) {
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
  } else if (hi2c->Instance == I2C2) {
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN;
  }

  HAL_I2C_MspDeInit(hi2c);

  hi2c->State = HAL_I2C_STATE_RESET;

  return HAL_OK;
}

__weak void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
}

__weak void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (hi2c->State != HAL_I2C_STATE_READY) {
    return HAL_BUSY;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Wait until BUSY flag is reset */
  while ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Generate start condition */
  hi2c->Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Send slave address */
  hi2c->Instance->DR = DevAddress;

  /* Wait until ADDR flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  I2C_ClearADDRFlag(hi2c);

  /* Send data */
  while (Size > 0) {
    /* Wait until TXE flag is set */
    while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
        return HAL_TIMEOUT;
      }
    }

    hi2c->Instance->DR = *pData++;
    Size--;

    if (Size == 0) {
      /* Wait until BTF flag is set */
      while ((hi2c->Instance->SR1 & I2C_SR1_BTF) == 0) {
        if ((HAL_GetTick() - tickstart) > Timeout) {
          hi2c->Instance->CR1 |= I2C_CR1_STOP;
          return HAL_TIMEOUT;
        }
      }

      /* Generate stop condition */
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
    }
  }

  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (hi2c->State != HAL_I2C_STATE_READY) {
    return HAL_BUSY;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Wait until BUSY flag is reset */
  while ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Enable ACK */
  hi2c->Instance->CR1 |= I2C_CR1_ACK;

  /* Generate start condition */
  hi2c->Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Send slave address with read bit */
  hi2c->Instance->DR = DevAddress | 0x01;

  /* Wait until ADDR flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  if (Size == 1) {
    /* Disable ACK */
    hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
  }

  I2C_ClearADDRFlag(hi2c);

  if (Size == 1) {
    /* Generate stop condition */
    hi2c->Instance->CR1 |= I2C_CR1_STOP;

    /* Wait until RXNE flag is set */
    while ((hi2c->Instance->SR1 & I2C_SR1_RXNE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        return HAL_TIMEOUT;
      }
    }

    *pData = hi2c->Instance->DR;
  } else {
    while (Size > 0) {
      /* Wait until RXNE flag is set */
      while ((hi2c->Instance->SR1 & I2C_SR1_RXNE) == 0) {
        if ((HAL_GetTick() - tickstart) > Timeout) {
          hi2c->Instance->CR1 |= I2C_CR1_STOP;
          return HAL_TIMEOUT;
        }
      }

      *pData++ = hi2c->Instance->DR;
      Size--;

      if (Size == 1) {
        /* Disable ACK and generate stop */
        hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
      }
    }
  }

  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (hi2c->State != HAL_I2C_STATE_READY) {
    return HAL_BUSY;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Wait until BUSY flag is reset */
  while ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Generate start condition */
  hi2c->Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Send slave address */
  hi2c->Instance->DR = DevAddress;

  /* Wait until ADDR flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  I2C_ClearADDRFlag(hi2c);

  /* Wait until TXE flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  /* Send memory address */
  if (MemAddSize == I2C_MEMADD_SIZE_16BIT) {
    hi2c->Instance->DR = (uint8_t)(MemAddress >> 8);

    while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
        return HAL_TIMEOUT;
      }
    }
  }

  hi2c->Instance->DR = (uint8_t)MemAddress;

  /* Send data */
  while (Size > 0) {
    while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
        return HAL_TIMEOUT;
      }
    }

    hi2c->Instance->DR = *pData++;
    Size--;
  }

  /* Wait until BTF flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_BTF) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  /* Generate stop condition */
  hi2c->Instance->CR1 |= I2C_CR1_STOP;

  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (hi2c->State != HAL_I2C_STATE_READY) {
    return HAL_BUSY;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Wait until BUSY flag is reset */
  while ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Generate start condition */
  hi2c->Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Send slave address */
  hi2c->Instance->DR = DevAddress;

  /* Wait until ADDR flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  I2C_ClearADDRFlag(hi2c);

  /* Wait until TXE flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  /* Send memory address */
  if (MemAddSize == I2C_MEMADD_SIZE_16BIT) {
    hi2c->Instance->DR = (uint8_t)(MemAddress >> 8);

    while ((hi2c->Instance->SR1 & I2C_SR1_TXE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
        return HAL_TIMEOUT;
      }
    }
  }

  hi2c->Instance->DR = (uint8_t)MemAddress;

  /* Wait until BTF flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_BTF) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_TIMEOUT;
    }
  }

  /* Generate start condition */
  hi2c->Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  /* Enable ACK */
  hi2c->Instance->CR1 |= I2C_CR1_ACK;

  /* Send slave address with read bit */
  hi2c->Instance->DR = DevAddress | 0x01;

  /* Wait until ADDR flag is set */
  while ((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) {
    if ((HAL_GetTick() - tickstart) > Timeout) {
      return HAL_TIMEOUT;
    }
  }

  if (Size == 1) {
    /* Disable ACK */
    hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
  }

  I2C_ClearADDRFlag(hi2c);

  if (Size == 1) {
    /* Generate stop condition */
    hi2c->Instance->CR1 |= I2C_CR1_STOP;

    /* Wait until RXNE flag is set */
    while ((hi2c->Instance->SR1 & I2C_SR1_RXNE) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        return HAL_TIMEOUT;
      }
    }

    *pData = hi2c->Instance->DR;
  } else {
    while (Size > 0) {
      /* Wait until RXNE flag is set */
      while ((hi2c->Instance->SR1 & I2C_SR1_RXNE) == 0) {
        if ((HAL_GetTick() - tickstart) > Timeout) {
          hi2c->Instance->CR1 |= I2C_CR1_STOP;
          return HAL_TIMEOUT;
        }
      }

      *pData++ = hi2c->Instance->DR;
      Size--;

      if (Size == 1) {
        /* Disable ACK and generate stop */
        hi2c->Instance->CR1 &= ~I2C_CR1_ACK;
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
      }
    }
  }

  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t trial = 0;

  while (trial < Trials) {
    /* Wait until BUSY flag is reset */
    while ((hi2c->Instance->SR2 & I2C_SR2_BUSY) != 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        return HAL_TIMEOUT;
      }
    }

    /* Generate start condition */
    hi2c->Instance->CR1 |= I2C_CR1_START;

    /* Wait until SB flag is set */
    while ((hi2c->Instance->SR1 & I2C_SR1_SB) == 0) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        return HAL_TIMEOUT;
      }
    }

    /* Send slave address */
    hi2c->Instance->DR = DevAddress;

    /* Wait until ADDR flag is set or AF flag is set */
    while (((hi2c->Instance->SR1 & I2C_SR1_ADDR) == 0) && ((hi2c->Instance->SR1 & 0x10) == 0)) {
      if ((HAL_GetTick() - tickstart) > Timeout) {
        hi2c->Instance->CR1 |= I2C_CR1_STOP;
        return HAL_TIMEOUT;
      }
    }

    /* Check if ADDR flag is set */
    if ((hi2c->Instance->SR1 & I2C_SR1_ADDR) != 0) {
      /* Device is ready */
      I2C_ClearADDRFlag(hi2c);
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
      return HAL_OK;
    } else {
      /* Clear AF flag */
      hi2c->Instance->SR1 &= ~0x10;
      hi2c->Instance->CR1 |= I2C_CR1_STOP;
    }

    trial++;
  }

  return HAL_ERROR;
}

static void I2C_ClearADDRFlag(I2C_HandleTypeDef *hi2c)
{
  /* Clear ADDR flag by reading SR1 and SR2 */
  volatile uint32_t tmpreg;
  tmpreg = hi2c->Instance->SR1;
  tmpreg = hi2c->Instance->SR2;
}