// Copyright 2025 Rose Laird

#include <stdio.h>
#include <stdlib.h>
#include "film_library_plus.h"
#include <stdint.h>

void speed_up(FILE *inputFile, FILE *outputFile, int64_t numFrames,
        unsigned char height, unsigned char width,
        unsigned char channels, int speedFactor) {
    if (speedFactor <= 1) {
        fprintf(stderr, "Error: Speed factor must be greater than 1.\n");
        return;
    }

    size_t frameSize = height * width * channels;
    unsigned char *frameBuffer = malloc(frameSize);
    if (!frameBuffer) {
        perror("Error allocating memory");
        exit(1);
    }

    // Calculate the new frame count after fast forwarding
    int64_t newFrameCount = numFrames / speedFactor;

    // Write the updated metadata with the new frame count
    #pragma pack(1)  // No padding
    typedef struct {
        int64_t numFrames;       // 8 bytes
        unsigned char channels;  // 1 byte
        unsigned char height;    // 1 byte
        unsigned char width;     // 1 byte
    } VideoMetadata;
    #pragma pack()
    VideoMetadata metadata = {newFrameCount, channels, height, width};
    fseek(outputFile, 0, SEEK_SET);  // rewind to start of file
    fwrite(&metadata, sizeof(VideoMetadata), 1, outputFile);  // write header

    for (int64_t frame = 0; frame < numFrames; frame++) {
        size_t bytesRead = fread(frameBuffer, 1, frameSize, inputFile);
        if (bytesRead != frameSize) {
            perror("Error reading frame data");
            free(frameBuffer);
            exit(1);
        }

        if (frame % speedFactor == 0) {  // Finds one in N frames
            size_t bytesWritten = fwrite(frameBuffer, 1, frameSize, outputFile);
            if (bytesWritten != frameSize) {
                perror("Error writing frame data");
                free(frameBuffer);
                exit(1);
            }
        }
    }
    // Free the frame buffer
    free(frameBuffer);
    printf("Fast forward operation completed successfully.\n");
}

float parse_aspect_ratio(const char *aspectRatioStr) {
    int width, height;

    // Parse the string "16:9" into width and height
    if (sscanf(aspectRatioStr, "%d:%d", &width, &height) != 2
            || width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Invalid aspect ratio format."
            "Use WIDTH:HEIGHT (e.g., 16:9).\n");
        exit(1);
    }

    return (float)width / height;
}


void crop_aspect_ratio(FILE *inputFile, FILE *outputFile, int64_t numFrames,
                unsigned char originalWidth, unsigned char originalHeight,
                unsigned char channels, const char *aspectRatioStr) {
    // Parse the aspect ratio
    float targetAspectRatio = parse_aspect_ratio(aspectRatioStr);

    // Calculate original and target dimensions
    float originalAspectRatio = (float)originalWidth / originalHeight;
    unsigned char targetWidth, targetHeight;

    if (originalAspectRatio > targetAspectRatio) {
        // Crop width
        targetHeight = originalHeight;
        targetWidth = (unsigned char)(originalHeight * targetAspectRatio);
    } else {
        // Crop height
        targetWidth = originalWidth;
        targetHeight = (unsigned char)(originalWidth / targetAspectRatio);
    }

    // Allocate buffers for original and cropped frames
    size_t originalFrameSize = originalWidth * originalHeight * channels;
    size_t croppedFrameSize = targetWidth * targetHeight * channels;
    unsigned char *originalFrame = malloc(originalFrameSize);
    unsigned char *croppedFrame = malloc(croppedFrameSize);

    if (!originalFrame || !croppedFrame) {
        perror("Error allocating memory");
        free(originalFrame);
        free(croppedFrame);
        exit(1);
    }

    // Calculate crop bounds
    int cropTop = (originalHeight - targetHeight) / 2;
    int cropLeft = (originalWidth - targetWidth) / 2;

    // Write updated metadata
    #pragma pack(1)
    typedef struct {
        int64_t numFrames;
        unsigned char channels;
        unsigned char height;
        unsigned char width;
    } VideoMetadata;
    #pragma pack()
    VideoMetadata metadata = {numFrames, channels, targetHeight, targetWidth};
    fseek(outputFile, 0, SEEK_SET);
    if (fwrite(&metadata, sizeof(VideoMetadata), 1, outputFile) != 1) {
        perror("Error writing metadata");
        free(originalFrame);
        free(croppedFrame);
        exit(1);
    }

    // Process frames
    for (int64_t frame = 0; frame < numFrames; frame++) {
        // Read original frame
        size_t bytesRead = fread(originalFrame,
                1, originalFrameSize, inputFile);
        if (bytesRead != originalFrameSize) {
            perror("Error reading frame data");
            free(originalFrame);
            free(croppedFrame);
            exit(1);
        }

        // Crop the frame
        for (unsigned char ch = 0; ch < channels; ch++) {
            unsigned char *originalChannelStart = originalFrame +
                                ch * originalWidth * originalHeight;
            unsigned char *croppedChannelStart = croppedFrame +
                                ch * targetWidth * targetHeight;

            for (unsigned char row = 0; row < targetHeight; row++) {
                for (unsigned char col = 0; col < targetWidth; col++) {
                    croppedChannelStart[row * targetWidth + col] =
                        originalChannelStart[(row + cropTop) * originalWidth
                            + (col + cropLeft)];
                }
            }
        }

        // Write cropped frame
        size_t bytesWritten = fwrite(croppedFrame, 1,
                            croppedFrameSize, outputFile);
        if (bytesWritten != croppedFrameSize) {
            perror("Error writing cropped frame data");
            free(originalFrame);
            free(croppedFrame);
            exit(1);
        }
    }
    // Free memory
    free(originalFrame);
    free(croppedFrame);
    printf("Aspect ratio adjustment completed successfully."
        "Target aspect ratio: %.2f\n", targetAspectRatio);
}
