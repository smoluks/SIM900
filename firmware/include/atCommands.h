/*
 * atCommands.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

#define MASTERPHONE "+79372389951"

#define callCommand "ATD"MASTERPHONE";\r\n"
#define stopCallCommand "ATH"MASTERPHONE"\r\n"
#define smscommand "AT+CMGS=\""MASTERPHONE"\"\r\n"
