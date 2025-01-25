def compare_binary_files(file1_path, file2_path):
    """
    Compare two binary files to check if they are identical.
    Flags any differences by displaying the position and differing byte values.
   
    Args:
        file1_path (str): Path to the first binary file.
        file2_path (str): Path to the second binary file.
   
    Returns:
        bool: True if files are identical, False otherwise.
    """
    try:
        with open(file1_path, 'rb') as file1, open(file2_path, 'rb') as file2:
            position = 0
            identical = True

            while True:
                byte1 = file1.read(1)
                byte2 = file2.read(1)

                # Check for end of files
                if not byte1 and not byte2:
                    break

                if byte1 != byte2:
                    identical = False
                    print(f"Difference at position {position}: {byte1} != {byte2}")

                position += 1

            # Check if one file is longer than the other
            if file1.read(1) or file2.read(1):
                identical = False
                print("Files differ in size.")

            return identical

    except FileNotFoundError as e:
        print(f"Error: {e}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return False

# Example usage
file1 = 'correct/swapped.bin'
file2 = 'rose_s.bin'

if compare_binary_files(file1, file2):
    print("The files are identical.")
else:
    print("The files are not identical.")