// Copyright 2025 Rose Laird

#include <stdio.h>
#include <stdlib.h>  // for malloc, free
#include <string.h>  // for memcpy
#include <omp.h>  // for OpenMP parallelization
#include <sys/sysinfo.h>
#include "film_library.h"  // for function declarations
#include <sys/mman.h>  // for memory mapping
#include <fcntl.h>  // for file control options
#include <unistd.h>  // for file I/O
#include <emmintrin.h>  // for SSE2 intrinsics SIMD
#include <stdbool.h>  // for boolean type
#include <stdint.h>  // for int64_t type


void reverse(FILE *inputFile, FILE *outputFile,
        int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    unsigned char *buffer;
    size_t frameSize = height * width * channels;
    size_t fileSize = numFrames * frameSize;

    buffer = malloc(fileSize);
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    size_t bytesRead = fread(buffer, 1, fileSize, inputFile);
    if (bytesRead != fileSize) {
        perror("Error reading file data");
        free(buffer);
        exit(1);
    }

    // Reverse the frames in the buffer
    for (int64_t start = 0, end = fileSize - frameSize; start < end;
            start += frameSize, end -= frameSize) {
        unsigned char temp[frameSize];
        memcpy(temp, &buffer[start], frameSize);
        memcpy(&buffer[start], &buffer[end], frameSize);
        memcpy(&buffer[end], temp, frameSize);
    }

    // Write the reversed data to the output file
    size_t bytesWritten = fwrite(buffer, 1, fileSize, outputFile);
    if (bytesWritten != fileSize) {
        perror("Error writing file data");
        free(buffer);
        exit(1);
    }

    printf("Successfully wrote %zu bytes to output file.\n", bytesWritten);
    free(buffer);
}

void reverse_fast(FILE *inputFile, FILE *outputFile,
        int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    size_t frameSize = height * width * channels;
    size_t fileSize = frameSize * numFrames;

    // Memory map the input file
    int inputFd = fileno(inputFile);
    if (inputFd == -1) {
        perror("Error getting file descriptor for input file");
        exit(EXIT_FAILURE);
    }

    unsigned char *mappedData = mmap(NULL, fileSize + 11,
            PROT_READ, MAP_PRIVATE, inputFd, 0);
    if (mappedData == MAP_FAILED) {
        perror("Error mapping input file");
        exit(EXIT_FAILURE);
    }

    // Skip metadata
    unsigned char *inputData = mappedData + 11;

    size_t batchSize = 1024;  // Number of frames per batch

    // No mapping for output file
    unsigned char *writeBuffer = malloc(batchSize * frameSize);
    if (writeBuffer == NULL) {
        perror("Error allocating write buffer");
        munmap(mappedData, fileSize + 11);
        exit(EXIT_FAILURE);
    }

    // Reads batches of frames from the end of the file
    for (int64_t batchStart = numFrames - 1; batchStart >= 0;
            batchStart -= batchSize) {
        int64_t batchEnd = batchStart - batchSize + 1;
        if (batchEnd < 0) batchEnd = 0;

        // Reverse the frames in the batch
        for (int64_t i = batchStart; i >= batchEnd; i--) {
            memcpy(writeBuffer + (batchStart - i) * frameSize,
                   inputData + i * frameSize, frameSize);
        }
        // Write the reversed batch to the output file
        fwrite(writeBuffer, 1, (batchStart - batchEnd + 1) * frameSize,
            outputFile);
    }
    free(writeBuffer);
    munmap(mappedData, fileSize + 11);
}

void reverse_small(FILE *inputFile, FILE *outputFile,
        int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    size_t frameSize = height * width * channels;
    unsigned char *buffer = malloc(frameSize);

    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    // Reads the frames in reverse order
    for (int64_t frame = numFrames - 1; frame >= 0; frame--) {
        // Finds position of ith last frame
        fseek(inputFile, (11 + frame * frameSize), SEEK_SET);
        size_t bytesRead = fread(buffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(buffer);
            exit(1);
        }
        // Writes one frame to output file
        if (fwrite(buffer, 1, frameSize, outputFile) != frameSize) {
            perror("Error writing frame data");
            free(buffer);
            exit(1);
        }
    }
    free(buffer);
    printf("Reverse operation completed successfully.\n");
}


void swap_channel(FILE *inputFile, FILE *outputFile, unsigned char ch1,
        unsigned char ch2, int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    if (ch1 >= channels || ch2 >= channels) {
        fprintf(stderr, "Error: Invalid channel indices.\n");
        return;
    }

    size_t frameSize = height * width * channels;  // Size of a single frame
    size_t numFramesBatch = 1024;                // Maximum batch size
    size_t totalSize = numFramesBatch * frameSize;

    // Buffer for reading/writing frames
    unsigned char *buffer = malloc(totalSize);
    if (!buffer) {
        perror("Memory allocation failed");
        exit(1);
    }

    int64_t framesProcessed = 0;

    while (framesProcessed < numFrames) {
        // Adjust the batch size for the last partial batch
        if (framesProcessed + numFramesBatch > numFrames) {
            numFramesBatch = numFrames - framesProcessed;
            totalSize = numFramesBatch * frameSize;
        }

        // read in a batch of frames
        size_t bytesRead = fread(buffer, 1, totalSize, inputFile);
        if (bytesRead != totalSize) {
            perror("Error reading input file");
            free(buffer);
            exit(1);
        }

        // Perform channel swapping on the batch
        #pragma omp parallel for
        for (size_t frame = 0; frame < numFramesBatch; frame++) {
            // Pointers to track positions of channels in the frame
            unsigned char *frameStart = buffer + (frame * frameSize);
            unsigned char *ch1_start = frameStart + (ch1 * height * width);
            unsigned char *ch2_start = frameStart + (ch2 * height * width);

            for (size_t pixel = 0; pixel < height * width; pixel++) {
                unsigned char temp = ch1_start[pixel];
                ch1_start[pixel] = ch2_start[pixel];
                ch2_start[pixel] = temp;
            }
        }
        // Write the modified batch back to the output file
        size_t bytesWritten = fwrite(buffer, 1, totalSize, outputFile);
        if (bytesWritten != totalSize) {
            perror("Error writing to output file");
            free(buffer);
            exit(1);
        }

        framesProcessed += numFramesBatch;
    }
    free(buffer);
    printf("Channel swapping completed successfully.\n");
}

void swap_channel_fast(FILE *inputFile, FILE *outputFile, unsigned char ch1,
        unsigned char ch2, int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    if (ch1 >= channels || ch2 >= channels) {
        fprintf(stderr, "Error: Invalid channel indices.\n");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;
    size_t totalSize = numFrames * frameSize;

    // Buffer for reading/writing frames
    unsigned char *buffer = malloc(totalSize);
    if (!buffer) {
        perror("Memory allocation failed");
        exit(1);
    }
    // Temporary buffer for swapping channels
    unsigned char *tempBuffer = malloc(channelSize);
    if (!buffer || !tempBuffer) {
        perror("Memory allocation failed");
        free(buffer);
        free(tempBuffer);
        exit(1);
    }

    // Read the entire file into memory
    size_t bytesRead = fread(buffer, 1, totalSize, inputFile);
    if (bytesRead != totalSize) {
        perror("Error reading input file");
        free(buffer);
        free(tempBuffer);
        exit(1);
    }

    // Process each frame
    for (int64_t frame = 0; frame < numFrames; frame++) {
        unsigned char *frameStart = buffer + (frame * frameSize);
        unsigned char *ch1_start = frameStart + (ch1 * channelSize);
        unsigned char *ch2_start = frameStart + (ch2 * channelSize);

        // Swap using memcpy - faster than a loop
        memcpy(tempBuffer, ch1_start, channelSize);
        memcpy(ch1_start, ch2_start, channelSize);
        memcpy(ch2_start, tempBuffer, channelSize);
    }

    // Write the modified data back to the output file
    size_t bytesWritten = fwrite(buffer, 1, totalSize, outputFile);
    if (bytesWritten != totalSize) {
        perror("Error writing to output file");
        free(buffer);
        free(tempBuffer);
        exit(1);
    }

    free(buffer);
    free(tempBuffer);

    printf("Channel swapping completed successfully using memcpy.\n");
}

void swap_channel_small(FILE *inputFile, FILE *outputFile, unsigned char ch1,
        unsigned char ch2, int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    if (ch1 >= channels || ch2 >= channels) {
        fprintf(stderr, "Error: Invalid channel indices.\n");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    // Allocate memory for reading/writing frames
    unsigned char *frameBuffer = malloc(frameSize);
    if (!frameBuffer) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Processes one frame at a time
    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }

        unsigned char *ch1_start = frameBuffer + (ch1 * channelSize);
        unsigned char *ch2_start = frameBuffer + (ch2 * channelSize);

        // Swap the channels using a temporary buffer
        for (size_t pixel = 0; pixel < channelSize; pixel++) {
            unsigned char temp = ch1_start[pixel];
            ch1_start[pixel] = ch2_start[pixel];
            ch2_start[pixel] = temp;
        }

        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(1);
        }
    }
    free(frameBuffer);
}


void clip_channel(FILE *inputFile, FILE *outputFile, unsigned char channel,
        unsigned char min, unsigned char max, int64_t numFrames,
        unsigned char height, unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    unsigned char *frameBuffer = malloc(frameSize);
    if (frameBuffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    // Process each frame
    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }

        // Clips each pixel in the channel
        for (size_t pixel = 0; pixel < channelSize; pixel++) {
            unsigned char *channelValue =
                &frameBuffer[channel * channelSize + pixel];
            if (*channelValue > max) {
                *channelValue = max;
            } else if (*channelValue < min) {
                *channelValue = min;
            }
        }

        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(1);
        }
    }

    free(frameBuffer);
}

void clip_channel_fast(FILE *inputFile, FILE *outputFile, unsigned char channel,
        unsigned char min, unsigned char max, int64_t numFrames,
        unsigned char height, unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index\n");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    // Allocate memory for the frame buffer
    unsigned char *frameBuffer = malloc(frameSize);
    if (frameBuffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Create a dynamic lookup table for pixel values
    unsigned char lookupTable[256];
    // Track which values have been processed
    bool isValueProcessed[256] = {false};

    // Process each frame
    for (int64_t frame = 0; frame < numFrames; frame++) {
        // Read the frame from the input file
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(EXIT_FAILURE);
        }

        // Get the pointer to the start of the specified channel
        unsigned char *channelStart = frameBuffer + channel * channelSize;

        // Apply dynamic lookup table to the channel
        for (int64_t pixel = 0; pixel < channelSize; pixel++) {
            unsigned char pixelValue = channelStart[pixel];
            if (!isValueProcessed[pixelValue]) {
                // Compute and store the clamped value in the lookup table
                if (pixelValue < min) {
                    lookupTable[pixelValue] = min;
                } else if (pixelValue > max) {
                    lookupTable[pixelValue] = max;
                } else {
                    lookupTable[pixelValue] = pixelValue;
                }
                isValueProcessed[pixelValue] = true;
            }

            // Replace the pixel value with its clamped counterpart
            channelStart[pixel] = lookupTable[pixelValue];
        }

        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(EXIT_FAILURE);
        }
    }
    // Clean up
    free(frameBuffer);
    printf("Clipping operation completed successfully with dynamic lookup.\n");
}

void clip_channel_small(FILE *inputFile, FILE *outputFile,
        unsigned char channel, unsigned char min, unsigned char max,
        int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    // Allocate memory for the frame buffer
    unsigned char *frameBuffer = malloc(frameSize);
    if (frameBuffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    // Process each frame
    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }
        for (size_t pixel = 0; pixel < channelSize; pixel++) {
            unsigned char *channelValue =
                &frameBuffer[channel * channelSize + pixel];
            // Clamp the pixel value
            if (*channelValue > max) {
                *channelValue = max;
            } else if (*channelValue < min) {
                *channelValue = min;
            }
        }
        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(1);
        }
    }
    // Clean up
    free(frameBuffer);
}


void scale_channel(FILE *inputFile, FILE *outputFile, unsigned char channel,
        float factor, int64_t numFrames, unsigned char height,
        unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    // Allocate memory for the frame buffer
    unsigned char *frameBuffer = malloc(frameSize);
    if (frameBuffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }
        // Get the pointer to the start of the specified channel
        unsigned char *channelStart = frameBuffer + channel * channelSize;

        #pragma omp parallel for

        // Apply the scaling factor to each pixel in the channel
        for (int64_t pixel = 0; pixel < channelSize; pixel++) {
            float scaledValue = channelStart[pixel] * factor;
            // Clamp the scaled value to the range [0, 255]
            if (scaledValue > 255) {
                channelStart[pixel] = 255;
            } else if (scaledValue < 0) {
                channelStart[pixel] = 0;
            } else {
                channelStart[pixel] = (unsigned char)scaledValue;
            }
        }
        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(1);
        }
    }

    free(frameBuffer);
}

void scale_channel_fast(FILE *inputFile, FILE *outputFile,
        unsigned char channel, float factor, int64_t numFrames,
        unsigned char height, unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index\n");
        return;
    }

    size_t frameSize = height * width * channels;
    size_t channelSize = height * width;

    // Allocate memory for the frame buffer
    unsigned char *frameBuffer = malloc(frameSize);
    if (frameBuffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    // Create an empty lookup table for pixel values
    unsigned char lookup_table[256];
    bool table_initialized[256] = { false };

    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }

        unsigned char *channelStart = frameBuffer + channel * channelSize;
        for (size_t pixel = 0; pixel < channelSize; pixel++) {
            unsigned char value = channelStart[pixel];

            // Add pixel to table if not already present
            if (!table_initialized[value]) {
                float scaled = value * factor;
                if (scaled > 255) scaled = 255;
                else if (scaled < 0) scaled = 0;
                lookup_table[value] = (unsigned char)scaled;
                table_initialized[value] = true;
            }
            // Replace pixel value with scaled value from table
            channelStart[pixel] = lookup_table[value];
        }
        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(frameBuffer);
            exit(1);
        }
    }
    // Clean up
    free(frameBuffer);
}

void scale_channel_small(FILE *inputFile, FILE *outputFile,
        unsigned char channel, float factor, int64_t numFrames,
        unsigned char height, unsigned char width, unsigned char channels) {
    if (channel >= channels) {
        fprintf(stderr, "Error: Invalid channel index");
        return;
    }

    size_t channelSize = height * width;
    size_t frameSize = channelSize * channels;
    // Buffer for a single channel
    unsigned char *channelBuffer = malloc(channelSize);
    // Buffer for a single frame
    unsigned char *frameBuffer = malloc(frameSize);

    if (channelBuffer == NULL || frameBuffer == NULL) {
        perror("Error allocating memory");
        free(channelBuffer);
        free(frameBuffer);
        exit(1);
    }

    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(channelBuffer);
            free(frameBuffer);
            exit(1);
        }
        // Get the pointer to the start of the specified channel
        size_t channelOffset = channel * channelSize;
        memcpy(channelBuffer, frameBuffer + channelOffset, channelSize);

        // Apply the scaling factor to each pixel in the channel
        for (size_t pixel = 0; pixel < channelSize; pixel++) {
            float scaledValue = channelBuffer[pixel] * factor;
            if (scaledValue > 255) {
                channelBuffer[pixel] = 255;
            } else if (scaledValue < 0) {
                channelBuffer[pixel] = 0;
            } else {
                channelBuffer[pixel] = (unsigned char)scaledValue;
            }
        }
        // Copy the modified channel back to the frame buffer
        memcpy(frameBuffer + channelOffset, channelBuffer, channelSize);

        // Write the modified frame to the output file
        size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
        if (bytesWritten != frameSize) {
            perror("Error writing frame data");
            free(channelBuffer);
            free(frameBuffer);
            exit(1);
        }
    }
    // Clean up
    free(channelBuffer);
    free(frameBuffer);
}
