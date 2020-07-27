/*
 * atCommands.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

char* getCallCommand();
char* getSmsCommand();

#define pin "0000"

#define pingCommand "AT\r\n"
#define echoOffCommand "ATE0\r\n"
#define shortResponceCommand "ATV0\r\n"
#define uartSpeedCommand "AT+IPR=115200\r\n"
#define getUartSpeedCommand "AT+IPR?\r\n"
#define uartModeCommand "AT+IFC=2,2\r\n"
#define saveCommand "AT&W\r\n"
//call
#define getRegistrationStatusCommand "AT+CPIN?\r\n"
#define stopCallCommand "ATH\r\n"
#define pickUpCommand "ATA\r\n"
#define hangUpCommand "ATH\r\n"
#define pinCommand "AT+CPIN="pin"\r\n"
