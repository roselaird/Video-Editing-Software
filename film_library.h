// Copyright 2025 Rose Laird
#ifndef LIB_FILMMASTER2000_H
#define LIB_FILMMASTER2000_H
#include <stdio.h>

#pragma pack(1)  // Disable padding
typedef struct {
    int64_t numFrames;
    unsigned char channels;
    unsigned char height;
    unsigned char width;
} VideoMetadata;
#pragma pack()

void reverse(FILE *inputFile, FILE *outputFile,
    int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void reverse_fast(FILE *inputFile, FILE *outputFile,
    int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void reverse_small(FILE *inputFile, FILE *outputFile,
    int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void swap_channel(FILE *inputFile, FILE *outputFile,
    unsigned char ch1, unsigned char ch2, int64_t numFrames,
    unsigned char height, unsigned char width, unsigned char channels);
void swap_channel_fast(FILE *inputFile, FILE *outputFile,
    unsigned char ch1, unsigned char ch2, int64_t numFrames,
    unsigned char height, unsigned char width, unsigned char channels);
void swap_channel_small(FILE *inputFile, FILE *outputFile,
    unsigned char ch1, unsigned char ch2, int64_t numFrames,
    unsigned char height, unsigned char width, unsigned char channels);
void clip_channel(FILE *inputFile, FILE *outputFile,
    unsigned char channel, unsigned char min,
    unsigned char max, int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void clip_channel_fast(FILE *inputFile, FILE *outputFile,
    unsigned char channel, unsigned char min,
    unsigned char max, int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void clip_channel_small(FILE *inputFile, FILE *outputFile,
    unsigned char channel, unsigned char min,
    unsigned char max, int64_t numFrames, unsigned char height,
    unsigned char width, unsigned char channels);
void scale_channel(FILE *inputFile, FILE *outputFile,
    unsigned char channel, float factor, int64_t numFrames,
    unsigned char height, unsigned char width,
    unsigned char channels);
void scale_channel_fast(FILE *inputFile, FILE *outputFile,
    unsigned char channel, float factor, int64_t numFrames,
    unsigned char height, unsigned char width,
    unsigned char channels);
void scale_channel_small(FILE *inputFile, FILE *outputFile,
    unsigned char channel, float factor, int64_t numFrames,
    unsigned char height, unsigned char width,
    unsigned char channels);
#endif
