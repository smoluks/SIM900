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

#define stopCallCommand "ATH\r\n"
#define pickUpCommand "ATA\r\n"
#define hangUpCommand "ATH\r\n"
#define pinCommand "AT+CPIN="pin"\r\n"
