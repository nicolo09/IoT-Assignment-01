#define DEBUG

#define BAUD_RATE 9600
#define MAX_FADE 255
#define MAX_ANALOG 1023
#define SLEEP_TIMEOUT 10000000
#define INTERRUPT_MODE FALLING
#define EI_ARDUINO_INTERRUPTED_PIN


// FIXME: Debounce button on selecting pattern
// TODO: Change go to sleep to timerone
// TODO: Separate in more files
// TODO: Change penalty blink on press button during pattern showing
// FIXME: Fix bug game immediately starts after wake up

// Status
#define WAITING_FOR_START 0
#define PLAYING 1
#define TIME_OVER 2

#define UNCONNECTED_ANALOG_PIN A5

// Times
#define T1_MIN 1000
#define T1_MAX 4000
#define T2_START 4000
#define T3_START 6000
#define DEBOUNCE_TIME 150

#include <EnableInterrupt.h>
#include <avr/sleep.h>

const int buttonPin[] = {2, 9, 10, 11};
const int ledPin[] = {7, 6, 5, 4};
const int ledCount = sizeof(ledPin) / sizeof(ledPin[0]);
const int redLedPin = 3;
const int potPin = A2;
const int pinToWakeUp = buttonPin[0];
const double difficulties[] = {0.9, 0.8, 0.7, 0.6};
double difficultyFactor;

int badPattern[ledCount];
volatile int status;
int score;
volatile int penalties;
long t2;
long t3;
volatile long time;
volatile bool penalizedDuringPattern;
volatile long timePress[ledCount];

/*Turns off the green LEDs*/
void leds_off(const int *ledPins, const int ledCount)
{
    for (int i = 0; i < ledCount; i++) {
        digitalWrite(ledPins[i], LOW);
    }
}

/*Does a fade-in-fade-out cycle step*/
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

/*Setup function called on game starting*/
void start_game()
{
    // ISR for starting the game
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
    status = PLAYING;
}

/*ISR called on arduino waking up*/
void wake_up_function_ISR()
{
    Serial.println("Waking up");
    time = millis();
    for(int i = 0; i < ledCount; i++){
      detach_button_interrupt(i);
    }
    wait_for_button_release(findIndex(arduinoInterruptedPin, buttonPin, ledCount));
    enableInterrupt(buttonPin[0], start_game_ISR, FALLING);
}

/*Turns LEDs off and go to sleep*/
void go_to_sleep()
{
    leds_off(ledPin, ledCount);
    digitalWrite(redLedPin, LOW);
    Serial.println("Sleeping...");
    Serial.flush();
    for(int i = 0; i < ledCount; i++){
      attach_wake_up_interrupt(i);
    }
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    sleep_disable();
}

void attach_wake_up_interrupt(const int index){
  enableInterrupt(buttonPin[index],wake_up_function_ISR,INTERRUPT_MODE);
}

void start_game_ISR()
{
    start_game();
    wait_for_button_release(findIndex(arduinoInterruptedPin, buttonPin, ledCount));
    disableInterrupt(arduinoInterruptedPin);
}

/*Setup the initial waiting status*/
void set_initial_status()
{
    Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
    enableInterrupt(buttonPin[0], start_game_ISR, FALLING);
    time = millis();
    for (int i = 0; i < ledCount; i++) {
        timePress[i] = millis();
    }
    status = WAITING_FOR_START;
}

void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(redLedPin, OUTPUT);
    randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));
    for (int i = 0; i < ledCount; i++) {
        pinMode(ledPin[i], OUTPUT);
        pinMode(buttonPin[i], INPUT_PULLUP);
        badPattern[i] = LOW;
    }
    // Setting initial status
    set_initial_status();
}

// Functions to handle pattern input
/*Global variable to store the input pattern*/
int inputPattern[ledCount];

/*Changes an LED state (changes are reflected in inputPattern array)*/
void change_led_state(const int ledIndex, int *inputPattern)
{
    inputPattern[ledIndex] == LOW ? inputPattern[ledIndex] = HIGH : inputPattern[ledIndex] = LOW;
    digitalWrite(ledPin[ledIndex], inputPattern[ledIndex]);
}

void wait_for_button_release(int buttonIndex)
{
    while (digitalRead(buttonPin[buttonIndex]) == LOW) {
    }
}

/*Finds zero-based index of an element in an array*/
int findIndex(const int pin, const int *array, const int length)
{
    for (int i = 0; i < length; i++) {
        if (pin == array[i]) {
            return i;
        }
    }
    return -1;
}

/*ISR for handling button press during pattern input*/
void buttons_input_ISR()
{
    int index = findIndex(arduinoInterruptedPin, buttonPin, ledCount);
    if (millis() - timePress[index] > DEBOUNCE_TIME) {
        change_led_state(index, inputPattern);
        //wait_for_button_release(index);
    }
#ifdef DEBUG
    else {
        Serial.print("[DEBUG]: Debounced pin no. ");
        Serial.println(index);
    }
#endif
    timePress[index] = millis();
}

/*Sets each button interrupt to the corresponding ISR during pattern input*/
void set_button_interrupt(int index)
{
    enableInterrupt(buttonPin[index], buttons_input_ISR, FALLING);
}

/*Remove interrupt from the index button*/
void detach_button_interrupt(int index)
{
    disableInterrupt(buttonPin[index]);
}

// Functions to setup the game
/*Generates a random HIGH-LOW values pattern and saves it in pattern*/
void randomize_pattern(int *pattern)
{
    do {
        for (int i = 0; i < ledCount; i++) {
            pattern[i] = random(0, 2) == 0 ? LOW : HIGH;
        }
    } while (0 == memcmp(pattern, badPattern, ledCount * sizeof(pattern[0])));
}

/*Sets LEDs HIGH or LOW according to values in pattern*/
void set_leds(const int *pattern, const int *ledPins, const int ledCount)
{
    for (int i = 0; i < ledCount; i++) {
        digitalWrite(ledPins[i], pattern[i]);
    }
}

void give_penalty()
{
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
        status = PLAYING;
    }
}

void penalty_ISR()
{
    penalizedDuringPattern = true;
    detachPenaltyInterrupts();
    give_penalty();
    wait_for_button_release(findIndex(arduinoInterruptedPin, buttonPin, ledCount));
}

void attachPenaltyInterrupts()
{
    for (int i = 0; i < ledCount; i++) {
        enableInterrupt(buttonPin[i], penalty_ISR, FALLING);
    }
}

void detachPenaltyInterrupts()
{
    for (int i = 0; i < ledCount; i++) {
        disableInterrupt(buttonPin[i]);
    }
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
        case PLAYING:
            digitalWrite(redLedPin, LOW);
            // Led off for random time
            leds_off(ledPin, ledCount);
            delay(random(T1_MIN, T1_MAX));
            // Generate pattern
            int pattern[ledCount];
            randomize_pattern(pattern);
            // Turn on leds according to pattern
            set_leds(pattern, ledPin, ledCount);
            penalizedDuringPattern = false;
            attachPenaltyInterrupts();
            delay(t2);
            detachPenaltyInterrupts();
            // Turn off leds
            leds_off(ledPin, ledCount);
            if (penalizedDuringPattern) {
            }
            else {
                for (int i = 0; i < ledCount; i++) {
                    inputPattern[i] = LOW;
                }
                // Attach interrupt to buttons
                for (int i = 0; i < ledCount; i++) {
                    set_button_interrupt(i);
                }
                delay(t3);
                for (int i = 0; i < ledCount; i++) {
                    detach_button_interrupt(i);
                }
                status = TIME_OVER;
            }
            break;
        case TIME_OVER:
            if (0 == memcmp(pattern, inputPattern, ledCount * sizeof(pattern[0]))) {
                score++;
                t2 *= difficultyFactor;
                t3 *= difficultyFactor;
                Serial.print("New point! Score: ");
                Serial.println(score);
                status = PLAYING;
            }
            else {
                Serial.println("Time over! Wrong pattern!");
                give_penalty();
            }
            break;
        default:
            break;
    }
}
