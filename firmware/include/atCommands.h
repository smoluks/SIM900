/*
 * atCommands.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

#define MASTERPHONE "+79018031475"

#define callCommand "ATD"MASTERPHONE";\r\n"
#define stopCallCommand "ATH;\r\n"
#define pickUpCommand "ATA\r\n"
#define hangUpCommand "ATH\r\n"
#define smscommand "AT+CMGS=\""MASTERPHONE"\"\r\n"
