#ifndef PATTERN_UTILS_H
#define PATTERN_UTILS_H

/*Finds zero-based index of an element in an array*/
int findIndex(const int element, const int *array, const int length);

/*Generates a random HIGH-LOW values pattern and saves it in pattern, it will never return an all LOW pattern*/
void randomize_pattern(int *pattern, const int ledCount);

/*Returns true if patterns are equal to each other*/
bool patternCmp(int* pattern1, int* pattern2, int length);

#endif // PATTERN_UTILS_H