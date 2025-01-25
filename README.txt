README for FilmMaster2000 Project

Overview
The FilmMaster2000 project is a high-performance and memory-efficient video processing library designed for editing video frames in uncompressed binary format. It includes a command-line tool (runme) to interact with the library and perform operations such as reversing, channel swapping, clipping, scaling, speeding up videos, and cropping frames to a specified aspect ratio.


Files and Structure
film_library.c: Contains core functions for video frame operations.
film_library.h: Header file for film_library.c with function declarations.
film_library_plus.c: Contains advanced video processing functions.
film_library_plus.h: Header file for film_library_plus.c.
runme.c: Command-line tool for executing library functions.
Makefile: Build system to compile the project and generate the executable (runme) and static library (libFilmMaster2000.a).


Compilation and Execution
Requirements
Operating System: Linux
Compiler: GCC or equivalent supporting C standard libraries.


Build Instructions
Run make all to compile the source files into the runme executable and libFilmMaster2000.a static library.

Optional:
Use make test to execute predefined tests.
Use make clean to remove compiled binaries and intermediate files.


Usage
The runme executable takes the following general format:

./runme [input file] [output file] [-S/-M] [function] [options]
-S or -M: Optimize for Speed (-S) or Memory (-M). Leave empty for balanced operation.
[function]: Specifies the operation to perform:
 - reverse: Reverses video frames.
 - swap_channel [ch1,ch2]: Swaps channels ch1 and ch2.
 - clip_channel [channel] [min,max]: Clips pixel values in channel to [min,max].
 - scale_channel [channel] [factor]: Scales pixel values in channel by factor.
 - speed_up [factor]: Reduces the video length by keeping 1 frame out of every factor frames.
 - crop_aspect [aspect_ratio]: Crops video frames to match the target aspect_ratio (e.g., 16:9).


Examples
Reverse frames in a video: ./runme input.bin output.bin reverse
Swap channels 0 and 1: ./runme input.bin output.bin swap_channel 0,1
Clip channel 1 to the range [50,200]: ./runme input.bin output.bin clip_channel 1 [50,200]
Scale channel 2 by a factor of 1.5: ./runme input.bin output.bin scale_channel 2 1.5
Speed up video by a factor of 2: ./runme input.bin output.bin speed_up 2
Crop video to 16:9 aspect ratio: ./runme input.bin output.bin crop_aspect 16:9


Features
Core Functions
Reverse: Efficiently reverse video frames.
Swap Channel: Swap two specified channels in each frame.
Clip Channel: Limit pixels to a defined range.
Scale Channel: Scale pixels with a multiplication factor.

Advanced Functions
Speed Up: Reduce video length by skipping frames.
Crop Aspect Ratio: Adjust frames to fit a specified aspect ratio.


Optimization Modes
-S: Prioritize speed using optimized algorithms (e.g., preloaded buffers).
-M: Prioritize memory efficiency with smaller buffers and frame-by-frame processing.


Error Handling
Handles invalid file paths, formats, and input parameters gracefully.
Outputs informative error messages to the user.


Notes
Ensure input files follow the expected uncompressed binary format.
Metadata structure: [No. Frames (int64)][Channels (uchar)][Height (uchar)][Width (uchar)]
[Pixel Data...]


Author
Rose Laird
Copyright 2025
For questions or contributions, please contact the author.