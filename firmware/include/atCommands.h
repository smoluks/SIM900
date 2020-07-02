/*
 * atCommands.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

char* getCallCommand();
char* getSmsCommand();

#define stopCallCommand "ATH\r\n"
#define pickUpCommand "ATA\r\n"
#define hangUpCommand "ATH\r\n"
