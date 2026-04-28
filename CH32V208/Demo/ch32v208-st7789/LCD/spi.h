/******************************************************************************
 * File Name          : spi.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/8/15
 * Description        : Hardware SPI driver header.
 ******************************************************************************
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef __SPI_H__
#define __SPI_H__

#include "ch32v20x.h"

void SPI2_Init(void);
void SPI2_Write(uint8_t data);

#endif
