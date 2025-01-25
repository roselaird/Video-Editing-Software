// Copyright 2025 Rose Laird

#include <stdio.h>
#include <stdint.h>

void speed_up(FILE *inputFile, FILE *outputFile, int64_t numFrames,
        unsigned char height, unsigned char width,
        unsigned char channels, int speedFactor);

void crop_aspect_ratio(FILE *inputFile, FILE *outputFile, int64_t numFrames,
        unsigned char originalWidth, unsigned char originalHeight,
        unsigned char channels, const char *aspectRatioStr);
