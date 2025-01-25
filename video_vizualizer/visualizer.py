import struct
import os
from PIL import Image

def read_binary_video_file(input_file):
    with open('rose_s.bin', 'rb') as f:
        # Read the header
        num_frames = struct.unpack('Q', f.read(8))[0]
        channels = struct.unpack('B', f.read(1))[0]
        height = struct.unpack('B', f.read(1))[0]
        width = struct.unpack('B', f.read(1))[0]
        print(f"Number of frames: {num_frames}, Channels: {channels}, Height: {height}, Width: {width}")

        # Verify the data format
        if channels != 3:
            raise ValueError("Unsupported format: only 3 channels (RGB) are supported")

        # Read each frame
        frame_size = height * width * channels
        frames = []

        for _ in range(num_frames-1):
            # Read and split the frame data into R, G, B channels
            red_channel = f.read(height * width)
            green_channel = f.read(height * width)
            blue_channel = f.read(height * width)

            if len(red_channel) != height * width or len(green_channel) != height * width or len(blue_channel) != height * width:
                raise ValueError("Unexpected EOF while reading frame data")

            # Combine the channels into interleaved RGB format
            frame_data = bytearray(height * width * channels)
            for i in range(height * width):
                frame_data[i * 3] = red_channel[i]
                frame_data[i * 3 + 1] = green_channel[i]
                frame_data[i * 3 + 2] = blue_channel[i]

            frames.append((channels, height, width, frame_data))

        return frames

def save_frames_as_jpeg(frames, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    for i, (channels, height, width, frame_data) in enumerate(frames):
        if channels == 3:
            mode = 'RGB'
        else:
            raise ValueError("Unsupported number of channels")

        # Create an image object
        img = Image.frombytes(mode, (width, height), bytes(frame_data))

        # Save the image
        output_path = os.path.join(output_dir, f'frame_{i:04d}.jpg')
        img.save(output_path)
        print(f"Saved frame {i} to {output_path}")

def main():
    input_file = "rose_s.bin"  # Replace with your binary file name
    output_dir = "output_frames2"  # Replace with your desired output directory

    print("Reading binary video file...")
    frames = read_binary_video_file(input_file)

    print("Saving frames as JPEGs...")
    save_frames_as_jpeg(frames, output_dir)

    print("Done!")

if __name__ == "__main__":
    main()
