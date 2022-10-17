static const int MAX_FADE = 255;

void leds_off(const int *ledPins, const int ledCount)
{
    for (int i = 0; i < ledCount; i++) {
        digitalWrite(ledPins[i], LOW);
    }
}

void fade_next_step(const int redLedPin)
{
    static bool increasing = true;
    static int fadeValue = 0;
    if (increasing) {
        analogWrite(redLedPin, fadeValue++);
    }
    else {
        analogWrite(redLedPin, fadeValue--);
    }
    if (fadeValue == MAX_FADE) {
        increasing = false;
    }
    else if (fadeValue == 0) {
        increasing = true;
    }
    delay(10);
}

/*Changes an LED state (changes are reflected in inputPattern array)*/
void change_led_state(const int ledIndex, const int* ledPin, int *inputPattern)
{
    inputPattern[ledIndex] == LOW ? inputPattern[ledIndex] = HIGH : inputPattern[ledIndex] = LOW;
    digitalWrite(ledPin[ledIndex], inputPattern[ledIndex]);
}

/*Sets LEDs HIGH or LOW according to values in pattern*/
void set_leds(const int *pattern, const int *ledPins, const int ledCount)
{
    for (int i = 0; i < ledCount; i++) {
        digitalWrite(ledPins[i], pattern[i]);
    }
}

void wait_for_button_release(int pin)
{
    while (digitalRead(pin) == LOW) {
    }
}