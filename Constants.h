#ifndef CONSTANTS_H
#define CONSTANTS_H

// Hardware specifics
const int BAUD_RATE = 9600;
const int MAX_ANALOG = 1023;
const int DEBOUNCE_TIME = 160;

// Pins
const int buttonPin[] = {2, 9, 10, 11};
const int ledPin[] = {4, 5, 6, 7};
const int ledCount = sizeof(ledPin) / sizeof(ledPin[0]);
const int redLedPin = 3;
const int potPin = A2;
const int UNCONNECTED_ANALOG_PIN = A5;

// States
const int WAITING_FOR_START = 0;
const int TIME_OVER = 2;
const int GIVE_PENALTY = 3;
const int PLAYING_SHOW_PATTERN = 4;
const int PLAYING_GENERATE_PATTERN = 5;
const int PLAYING_INPUT_PATTERN = 6;

// Game times
const int T1_MIN = 1000;
const int T1_MAX = 4000;
const int T2_START = 4000;
const int T3_START = 6000;
const long SLEEP_TIMEOUT = 10000000;
const int GAME_OVER_TIMEOUT = 10000;

// Game settings
const double difficulties[] = {0.9, 0.8, 0.7, 0.6};

#endif // CONSTANTS_H