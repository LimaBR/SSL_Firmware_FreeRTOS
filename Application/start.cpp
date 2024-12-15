/*
 * start.cpp
 *
 *  Created on: Oct 31, 2024
 *      Author: Gabriel
 */

#include "start.h"
#include <cstdio>
#include "FreeRTOS.h"
#include "GPIO_Pin_STM32.hpp"
#include "BinLeds.hpp"
#include "LogDriver.hpp"
#include "GPIO_Pin_STM32.hpp"
#include "PWM_Pin_STM32.hpp"
#include "UART_STM32.hpp"
#include "SPI_Master_STM32.hpp"
#include "Queue_STM32.hpp"

#include "Shared/SX1280Lib/sx1280-hal.h"

GPIO_Pin_STM32 ledOrange(GPIOD, GPIO_PIN_13);
GPIO_Pin_STM32 ledGreen(GPIOD, GPIO_PIN_12);
GPIO_Pin_STM32 ledRed(GPIOD, GPIO_PIN_14);
GPIO_Pin_STM32 ledBlue(GPIOD, GPIO_PIN_15);
BinLeds discoveryLeds((GPIO_Pin*[]){&ledOrange, &ledGreen, &ledRed, &ledBlue}, 4);

extern UART_HandleTypeDef huart2;
UART_STM32 uart2(&huart2);

extern SPI_HandleTypeDef hspi1;
SPI_Master_STM32 spi1(&hspi1, 100);

GPIO_Pin_STM32 sx1280_b_busy(SX1280_B_BUSY_GPIO_Port, SX1280_B_BUSY_Pin);
GPIO_Pin_STM32 sx1280_b_irq(SX1280_B_IRQ_GPIO_Port, SX1280_B_IRQ_Pin);
GPIO_Pin_STM32 sx1280_b_nss(SX1280_B_NSS_GPIO_Port, SX1280_B_NSS_Pin);
GPIO_Pin_STM32 sx1280_b_rst(SX1280_B_RST_GPIO_Port, SX1280_B_RST_Pin);
GPIO_Pin_STM32 sx1280_b_rxen(SX1280_B_RXEN_GPIO_Port, SX1280_B_RXEN_Pin);
GPIO_Pin_STM32 sx1280_b_txen(SX1280_B_TXEN_GPIO_Port, SX1280_B_TXEN_Pin);

SX1280Hal sx1280_b(&spi1, &sx1280_b_nss, &sx1280_b_busy, &sx1280_b_irq, &sx1280_b_rst, nullptr);

void executableDispatch(void* _executable){
	Executable* executable = static_cast<Executable*>(_executable);
	executable->execute();
	vTaskDelete(NULL);
}

void start(){
	discoveryLeds.set(1);
	printf("----------------INIT----------------\n");
	printf("X and Y: %lu\n", HAL_GetUIDw0());
	printf("Wafer number: %c\n", (uint8_t)HAL_GetUIDw1());
	printf("Lot number: %c%c%c%c%c%c%c\n",
			(uint8_t)(HAL_GetUIDw1()>>8),
			(uint8_t)(HAL_GetUIDw1()>>16),
			(uint8_t)(HAL_GetUIDw1()>>24),
			(uint8_t)(HAL_GetUIDw2()),
			(uint8_t)(HAL_GetUIDw2()>>8),
			(uint8_t)(HAL_GetUIDw2()>>16),
			(uint8_t)(HAL_GetUIDw2()>>24)
			);
	discoveryLeds.set(2);

	// Init log facility
	LogDriver::getInstance()->init();
	TaskHandle_t hTaskLogDriver;
	xTaskCreate(executableDispatch, "LogDriver", 256, LogDriver::getInstance(), 20, &hTaskLogDriver);

	// Init queues

	// Init shared resources
	spi1.init();

	//Init components

	//Init tasks
	discoveryLeds.set(3);
	vTaskDelete(nullptr);
}
