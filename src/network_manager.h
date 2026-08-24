#pragma once

#include <Arduino.h>

void networkSetup();
void networkLoop();
bool networkIsConnected();
bool networkIsAccessPoint();
const String& networkOscHost();
uint16_t networkOscPort();

