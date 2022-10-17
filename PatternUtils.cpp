#include <Arduino.h>

/*Finds zero-based index of an element in an array*/
int findIndex(const int element, const int *array, const int length)
{
    for (int i = 0; i < length; i++) {
        if (element == array[i]) {
            return i;
        }
    }
    return -1;
}

/*Returns true if patterns are equal to each other*/
bool patternCmp(int* pattern1, int* pattern2, int length)
{
    if (0 == memcmp(pattern1, pattern2, length * sizeof(pattern1[0]))){
        return true;
    }
    return false;
}

/*Generates a random HIGH-LOW values pattern and saves it in pattern, it will never return an all LOW pattern*/
void randomize_pattern(int *pattern, const int ledCount)
{
    int badPattern[ledCount];
    do {
        for (int i = 0; i < ledCount; i++) {
            badPattern[i] = LOW;
            pattern[i] = random(0, 2) == 0 ? LOW : HIGH;
        }
    } while (patternCmp(pattern, badPattern, ledCount));
}
