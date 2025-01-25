// Copyright 2025 Rose Laird

#include <stdio.h>  // for printf, fprintf, perror
#include <stdlib.h>  // for atoi, atof, malloc, free
#include <string.h>  // for strcmp, sscanf
#include <sys/time.h>  // for gettimeofday
#include <sys/resource.h>  // for getrusage
#include "film_library.h"  // for function declarations
#include "film_library_plus.h"  // for extra functions
#include <stdint.h>  // for int64_t type
#include <emmintrin.h>  // SSE2 intrinsics


void print_usage() {
    // Print usage information and ends program on incorrect input
    fprintf(stderr,
        "Usage: ./runme [input file] [output file] [-S/-M] [function] "
        "[options]\n");
    fprintf(stderr, "Functions and options:\n");
    fprintf(stderr, "  reverse\n");
    fprintf(stderr, "  swap_channel <channel1> <channel2>\n");
    fprintf(stderr, "  clip_channel <channel> <min,max>\n");
    fprintf(stderr, "  scale_channel <channel> <factor>\n");
    fprintf(stderr, "  speed_up <factor>\n");
    fprintf(stderr, "  crop_aspect <aspect ratio>\n");
}

int main(int argc, char *argv[]) {
    struct timeval start_time, end_time;
    struct rusage usage_start, usage_end;

    gettimeofday(&start_time, NULL);
    getrusage(RUSAGE_SELF, &usage_start);

    if (argc < 4) {
        // too few arguments
        print_usage();
        return 1;
    }

    // Parse command line arguments
    char *inputFilePath = argv[1];
    char *outputFilePath = argv[2];
    char *mode = NULL;
    char *function = NULL;
    char **params = NULL;
    int param_count = argc - 5;

    if (strcmp(argv[3], "-S") == 0 || strcmp(argv[3], "-M") == 0) {
        mode = argv[3];
        function = argv[4];
        params = &argv[5];  // Options start from the 5th index
    } else {
        function = argv[3];
        params = &argv[4];  // Options start from the 4th index
        param_count += 1;
    }

    FILE *inputFile = fopen(inputFilePath, "rb");
    if (!inputFile) {
        perror("Error opening input file");
        return 1;
    }

    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(inputFile);
        return 1;
    }

    // Reads video metadata
    VideoMetadata metadata;
    if (fread(&metadata, sizeof(VideoMetadata), 1, inputFile) != 1) {
        perror("Error reading video metadata");
        fclose(inputFile);
        fclose(outputFile);
        return 1;
    }

    // Writes video metadata
    if (fwrite(&metadata, sizeof(VideoMetadata), 1, outputFile) != 1) {
        perror("Error writing video metadata");
        fclose(inputFile);
        fclose(outputFile);
        return 1;
    }
    // Files are passed in as pointers and opened in binary mode

    if (strcmp(function, "reverse") == 0) {
        if (mode && strcmp(mode, "-S") == 0) {
            reverse_fast(inputFile, outputFile, metadata.numFrames,
                metadata.height, metadata.width, metadata.channels);
        } else if (mode && strcmp(mode, "-M") == 0) {
            reverse_small(inputFile, outputFile, metadata.numFrames,
                metadata.height, metadata.width, metadata.channels);
        } else {
            reverse(inputFile, outputFile, metadata.numFrames,
                metadata.height, metadata.width, metadata.channels);
        }
    } else if (strcmp(function, "swap_channel") == 0) {
        if (params == NULL) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        unsigned char ch1, ch2;
        // Parse channel numbers
        if (sscanf(params[0], "%hhu,%hhu", &ch1, &ch2) != 2) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        if (mode && strcmp(mode, "-S") == 0) {
            swap_channel_fast(inputFile, outputFile, ch1, ch2,
                metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else if (mode && strcmp(mode, "-M") == 0) {
            swap_channel_small(inputFile, outputFile, ch1, ch2,
                metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else {
            swap_channel(inputFile, outputFile, ch1, ch2, metadata.numFrames,
                metadata.height, metadata.width, metadata.channels);
        }

    } else if (strcmp(function, "clip_channel") == 0) {
        if (param_count != 2) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        // Parse channel number
        unsigned char channel = (unsigned char)atoi(params[0]);
        unsigned char min, max;
        // Parse min and max values
        if (sscanf(params[1], "[%hhu,%hhu]", &min, &max) != 2) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        if (mode && strcmp(mode, "-S") == 0) {
            clip_channel_fast(inputFile, outputFile, channel, min, max,
                metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else if (mode && strcmp(mode, "-M") == 0) {
            clip_channel_small(inputFile, outputFile, channel, min, max,
                metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else {
            clip_channel(inputFile, outputFile, channel, min, max,
                metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        }

    } else if (strcmp(function, "scale_channel") == 0) {
        if (param_count != 2) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        // Parse channel number and scaling factor
        unsigned char channel = (unsigned char)atoi(params[0]);
        float factor = atof(params[1]);
        if (mode && strcmp(mode, "-S") == 0) {
            scale_channel_fast(inputFile, outputFile, channel,
                factor, metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else if (mode && strcmp(mode, "-M") == 0) {
            scale_channel_small(inputFile, outputFile, channel,
                factor, metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        } else {
            scale_channel(inputFile, outputFile, channel,
                factor, metadata.numFrames, metadata.height,
                metadata.width, metadata.channels);
        }
    } else if (strcmp(function, "speed_up") == 0) {
        if (param_count != 1) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        // Parse speed factor
        int speedFactor = atoi(params[0]);
        speed_up(inputFile, outputFile, metadata.numFrames, metadata.height,
            metadata.width, metadata.channels, speedFactor);
    } else if (strcmp(function, "crop_aspect") == 0) {
        if (param_count != 1) {
            print_usage();
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }
        // Crop video frames to a target aspect ratio with extracted ratio
        crop_aspect_ratio(inputFile, outputFile, metadata.numFrames,
                metadata.width, metadata.height, metadata.channels, params[0]);
    } else {
        fprintf(stderr, "Invalid function: %s\n", function);
        print_usage();
        fclose(inputFile);
        fclose(outputFile);
        return 1;
    }

    fclose(inputFile);
    fclose(outputFile);

    gettimeofday(&end_time, NULL);         // End timing
    getrusage(RUSAGE_SELF, &usage_end);   // End resource tracking

    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1e6;
    int64_t memory_used = usage_end.ru_maxrss - usage_start.ru_maxrss;

    printf("Elapsed time: %.6f seconds\n", elapsed_time);
    printf("Memory used: %ld KB\n", memory_used);

    return 0;
}
