import os

# Define the folder path
folder_path = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Data\Mod Downloads"

# Walk through the directory recursively
for root, _, files in os.walk(folder_path):
    # Filter files that match the required suffixes
    blend_file = None
    position_file = None

    for file_name in files:
        if file_name.endswith("Blend.buf"):
            blend_file = os.path.join(root, file_name)
        elif file_name.endswith("Texcoord.buf"):
            position_file = os.path.join(root, file_name)

    # If the folder contains at least one of the required files
    if blend_file or position_file:
        print(f"Subfolder: {root}")

        # Process Blend.buf file
        if blend_file:
            blend_size = os.path.getsize(blend_file)
            blend_size_div_32 = blend_size / 32
            print(f"Blend.buf Size (bytes): {blend_size}")
            print(f"Blend.buf Size / 32: {blend_size_div_32}")
        else:
            blend_size_div_32 = None
            print("Blend.buf not found.")

        # Process Texcoord.buf file
        if position_file:
            position_size = os.path.getsize(position_file)
            if blend_size_div_32:
                position_div_blend = position_size / blend_size_div_32
                print(f"Texcoord.buf Size / (Blend.buf Size / 32): {position_div_blend}")
            else:
                print("Cannot calculate Texcoord.buf ratio (Blend.buf not found).")
        else:
            print("Texcoord.buf not found.")

        print("-" * 40)