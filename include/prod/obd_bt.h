#pragma once

#include <Arduino.h>
#include "config.h"

bool obdBtBegin();
bool obdBtConnect();
bool obdBtIsConnected();
void obdBtDisconnect();
void obdBtEnd();

void obdBtSendLine(const char *line);

int obdBtAvailable();

int obdBtRead();
