/* Defines the interrupt mode through the entire sketch, since we use input
pull-ups hence buttons are placed in inverted logic, we use falling interrupts*/
#define INTERRUPT_MODE FALLING

#define EI_ARDUINO_INTERRUPTED_PIN

/*Uncomment to show some debug prints on serial line*/
//#define DEBUG

#include <EnableInterrupt.h>
#include <avr/sleep.h>
#include "LedButtonUtils.h"
#include "PatternUtils.h"
#include "Constants.h"

double difficultyFactor;
volatile int status;
int score;
volatile int penalties;
long t2;
long t3;
volatile long time = 0;
volatile long timePress[ledCount];

/*Setup function called on game starting*/
void start_game()
{
    leds_off(ledPin, ledCount);
    digitalWrite(redLedPin, LOW);
    Serial.println("Go!");
    score = 0;
    penalties = 0;
    difficultyFactor = difficulties[map(analogRead(potPin), 0, MAX_ANALOG, 0, sizeof(difficulties) / sizeof(difficulties[0]) - 1)];
#ifdef DEBUG
    Serial.print("[DEBUG]: Difficulty factor F: ");
    Serial.println(difficultyFactor);
#endif
    t2 = T2_START;
    t3 = T3_START;
    status = PLAYING_GENERATE_PATTERN;
}

/*ISR called on arduino waking up from deep sleep*/
void wake_up_function_ISR()
{
    Serial.println("I'm waking up");
    time = millis();
    for (int i = 0; i < ledCount; i++) {
        detach_button_interrupt(i);
    }
    wait_for_button_release(arduinoInterruptedPin);
    enableInterrupt(buttonPin[0], start_game_ISR, INTERRUPT_MODE);
}

/*Turns LEDs off and go to sleep*/
void go_to_sleep()
{
    leds_off(ledPin, ledCount);
    digitalWrite(redLedPin, LOW);
    Serial.println("Sleeping...");
    Serial.flush();
    for (int i = 0; i < ledCount; i++) {
        attach_wake_up_interrupt(i);
    }
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    sleep_disable();
}

/*Attach the wake up interrupt to a button*/
void attach_wake_up_interrupt(const int index)
{
    enableInterrupt(buttonPin[index], wake_up_function_ISR, INTERRUPT_MODE);
}

/*ISR triggered on T1 press to start game from initial status*/
void start_game_ISR()
{
    if (millis() - time > DEBOUNCE_TIME) {
        start_game();
        wait_for_button_release(arduinoInterruptedPin);
        disableInterrupt(arduinoInterruptedPin);
    }
}

/*Setup the initial waiting status*/
void set_initial_status()
{
    Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
    enableInterrupt(buttonPin[0], start_game_ISR, INTERRUPT_MODE);
    time = millis();
    for (int i = 0; i < ledCount; i++) {
        timePress[i] = millis();
    }
    status = WAITING_FOR_START;
}

/*Global variable to store the input pattern*/
int inputPattern[ledCount];

/*ISR for handling button press during pattern input*/
void buttons_input_ISR()
{
    int index = findIndex(arduinoInterruptedPin, buttonPin, ledCount);
    if (millis() - timePress[index] > DEBOUNCE_TIME) {
        change_led_state(index, ledPin, inputPattern);
    }
#ifdef DEBUG
    else {
        Serial.print("[DEBUG]: Debounced pin no. ");
        Serial.println(index);
    }
#endif
    timePress[index] = millis();
}

/*Setup button interrupt for pattern input*/
void set_button_input_interrupt(int index)
{
    enableInterrupt(buttonPin[index], buttons_input_ISR, INTERRUPT_MODE);
}

/*Remove interrupt from the index button*/
void detach_button_interrupt(int index)
{
    disableInterrupt(buttonPin[index]);
}

/*Remove all interrupts from the buttons*/
void detach_penalty_interrupts()
{
    for (int i = 0; i < ledCount; i++) {
        detach_button_interrupt(i);
    }
}

/*ISR called on button press during pattern showing*/
void penalty_ISR()
{
    status = GIVE_PENALTY;
}

/*Attach interrupts that gives penalties to all the buttons*/
void attach_penalty_interrupts()
{
    for (int i = 0; i < ledCount; i++) {
        enableInterrupt(buttonPin[i], penalty_ISR, INTERRUPT_MODE);
    }
}


void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(redLedPin, OUTPUT);
    randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));
    for (int i = 0; i < ledCount; i++) {
        pinMode(ledPin[i], OUTPUT);
        pinMode(buttonPin[i], INPUT_PULLUP);
    }
    // Setting initial status
    set_initial_status();
}

void loop()
{
    switch (status) {
        case WAITING_FOR_START:
            leds_off(ledPin, ledCount);
            fade_next_step(redLedPin);
            if (millis() - time > 10000) {
                go_to_sleep();
            }
            break;
        case PLAYING_GENERATE_PATTERN:
            digitalWrite(redLedPin, LOW);
            // Led off for random time
            leds_off(ledPin, ledCount);
            delay(random(T1_MIN, T1_MAX));
            // Generate pattern
            int pattern[ledCount];
            randomize_pattern(pattern, ledCount);
            // Turn on leds according to pattern
            set_leds(pattern, ledPin, ledCount);
            attach_penalty_interrupts();
            time = millis();
            status = PLAYING_SHOW_PATTERN;
            break;
        case PLAYING_SHOW_PATTERN:
            if (millis() - time > t2) {
                detach_penalty_interrupts();
                leds_off(ledPin, ledCount);
                status = PLAYING_INPUT_PATTERN;
            }
            break;
        case PLAYING_INPUT_PATTERN:
            for (int i = 0; i < ledCount; i++) {
                inputPattern[i] = LOW;
            }
            // Attach interrupt to buttons
            for (int i = 0; i < ledCount; i++) {
                set_button_input_interrupt(i);
            }
            delay(t3);
            for (int i = 0; i < ledCount; i++) {
                detach_button_interrupt(i);
            }
            status = TIME_OVER;
            break;
        case TIME_OVER:
            if (patternCmp(pattern, inputPattern, ledCount)) {
                score++;
                t2 *= difficultyFactor;
                t3 *= difficultyFactor;
                Serial.print("New point! Score: ");
                Serial.println(score);
                status = PLAYING_GENERATE_PATTERN;
            }
            else {
                Serial.println("Time over! Wrong pattern!");
                status = GIVE_PENALTY;
            }
            break;
        case GIVE_PENALTY:
            leds_off(ledPin, ledCount);
            penalties++;
            Serial.println("Penalty!");
            digitalWrite(redLedPin, HIGH);
            delay(1000);
            digitalWrite(redLedPin, LOW);
            if (penalties >= 3) {
                Serial.print("Game Over. Final Score: ");
                Serial.println(score);
                set_initial_status();
            }
            else {
                status = PLAYING_GENERATE_PATTERN;
            }
            break;
        default:
            break;
    }
}
