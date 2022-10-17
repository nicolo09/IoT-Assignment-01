#ifndef LED_BUTTON_UTILS_H
#define LED_BUTTON_UTILS_H

/*Turns off all the LEDs in the ledPins array*/
void leds_off(const int *ledPins, const int ledCount);

/*Does a fade-in-fade-out cycle step*/
void fade_next_step(const int redLedPin);

/*Changes an LED state in the ledPin array (changes are reflected into inputPattern)*/
void change_led_state(const int ledIndex, const int* ledPin, int *inputPattern);

/*Sets LEDs HIGH or LOW according to values in pattern*/
void set_leds(const int *pattern, const int *ledPins, const int ledCount);

void wait_for_button_release(int pin);

#endif // LED_BUTTON_UTILS_H