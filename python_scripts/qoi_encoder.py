#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt
from mpl_colormap_data import cmaps
import struct

QOIF = bytearray(b"qoif")
QOI_OP_RGB = int("fe", base=16)
QOI_OP_RGBA = int("ff", base=16)
QOI_OP_INDEX = int("00", base=16)
QOI_OP_DIFF = int("40", base=16)
QOI_OP_LUMA = int("80", base=16)
QOI_OP_RUN = int("c0", base=16)

QOI_MASK = int("c0", base=16)


def qoi_encode(
    data: list[int], width: int, height: int, num_channels: int, srgb: bool
) -> bytes:
    write_buffer = bytearray()

    # Magic
    write_buffer += QOIF
    # Size
    write_buffer += struct.pack(">I", width)
    write_buffer += struct.pack(">I", height)
    # Number of channels
    write_buffer += struct.pack(">B", num_channels)
    # Colorspace
    write_buffer += struct.pack(">B", 0 if srgb else 1)

    previous_pixel = (0, 0, 0, 255)
    pixel_cache = [[0, 0, 0, 0] for _ in range(64)]
    run_length = 0

    # Iterate over pixel data
    for idx in range(0, len(data), 3):
        red = data[idx]
        green = data[idx + 1]
        blue = data[idx + 2]
        alpha = data[idx + 3] if num_channels == 4 else 255

        # Equal to previous pixel
        if (red, green, blue, alpha) == previous_pixel:
            # Increment run length
            run_length += 1

            # At max run length or end of pixel data
            if run_length == 62 or (idx == len(data) - num_channels):
                write_buffer += (QOI_OP_RUN | run_length).to_bytes(1, "big")
                # Reset run
                run_length = 0
        else:
            # Run has ended. Encode the previous pixels data.
            if run_length > 0:
                write_buffer += (QOI_OP_RUN | (run_length - 1)).to_bytes(1, "big")
                run_length = 0

            hash_index = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64

            # Index previous pixel
            if pixel_cache[hash_index] == (red, green, blue, alpha):
                write_buffer += (QOI_OP_INDEX | hash_index).to_bytes(1, "big")
            else:
                pixel_cache[hash_index] = [red, green, blue, alpha]

                # Constant alpha
                if previous_pixel[3] == alpha:
                    # Pixel differences
                    red_diff = red - previous_pixel[0]
                    green_diff = green - previous_pixel[1]
                    blue_diff = blue - previous_pixel[2]

                    # Channel differences
                    red_green_diff = red_diff - green_diff
                    blue_green_diff = blue_diff - green_diff

                    # Small difference
                    if (
                        (red_diff > -3 and red_diff < 2)
                        and (green_diff > -3 and green_diff < 2)
                        and (blue_diff > -3 and blue_diff < 2)
                    ):
                        red_diff_bit = (red_diff + 2) << 4
                        green_diff_bit = (green_diff + 2) << 2
                        blue_diff_bit = (blue_diff + 2) << 0
                        write_buffer += (
                            QOI_OP_DIFF | red_diff_bit | green_diff_bit | blue_diff_bit
                        ).to_bytes(1, "big")
                    # Medium difference
                    elif (
                        (green_diff > -33 and green_diff < 32)
                        and (red_green_diff > -9 and red_green_diff < 8)
                        and (blue_green_diff > -9 and blue_green_diff < 8)
                    ):
                        red_green_diff_bit = (red_green_diff + 8) << 4
                        blue_green_diff_bit = (blue_green_diff + 8) << 0

                        write_buffer += (QOI_OP_LUMA | (green_diff + 32)).to_bytes(
                            1, "big"
                        )
                        write_buffer += (
                            red_green_diff_bit | blue_green_diff_bit
                        ).to_bytes(1, "big")
                    # Large difference
                    else:
                        write_buffer += QOI_OP_RGB.to_bytes(1, "big")
                        write_buffer += red.to_bytes(1, "big")
                        write_buffer += green.to_bytes(1, "big")
                        write_buffer += blue.to_bytes(1, "big")
                # All values are different
                else:
                    write_buffer += QOI_OP_RGBA.to_bytes(1, "big")
                    write_buffer += red.to_bytes(1, "big")
                    write_buffer += green.to_bytes(1, "big")
                    write_buffer += blue.to_bytes(1, "big")
                    write_buffer += alpha.to_bytes(1, "big")
        previous_pixel = (red, green, blue, alpha)

    # End chunk signifier
    NULL_BYTE = 0
    LAST_BYTE = 1
    for _ in range(7):
        write_buffer += NULL_BYTE.to_bytes(1, "big")
    write_buffer += LAST_BYTE.to_bytes(1, "big")

    return bytes(write_buffer)


def qoi_decode(data: bytes) -> list[int]:
    # Check magic bytes
    if data[0:4] != QOIF:
        print("Invalid QOI file! Missing magic bytes.")
        return []

    # Get image size
    image_width = struct.unpack(">I", data[4:8])[0]
    image_height = struct.unpack(">I", data[8:12])[0]

    # Get number of channels
    num_channels = data[12]
    colorspace = data[13]

    total_pixel_count = image_width * image_height

    pixel_cache = [[0, 0, 0, 0] for _ in range(64)]
    pixel_data = [0 for _ in range(num_channels * total_pixel_count)]
    pixel_idx = 0

    byte_pointer = 14

    previous_pixel = [0, 0, 0, 255]

    run_length = 0

    padding_count = 0

    for pixel_idx in range(0, total_pixel_count):
        red = previous_pixel[0]
        green = previous_pixel[1]
        blue = previous_pixel[2]
        alpha = previous_pixel[3]
        if run_length > 0:
            run_length -= 1
        # Read bytes
        else:
            # Check 8 bit tags
            current_byte = data[byte_pointer]

            if current_byte == QOI_OP_RGB:
                # Read next three bytes
                red = data[byte_pointer + 1]
                green = data[byte_pointer + 2]
                blue = data[byte_pointer + 3]

                # Increment byte pointer
                byte_pointer += 3
            elif current_byte == QOI_OP_RGBA:
                # Read next four bytes
                red = data[byte_pointer + 1]
                green = data[byte_pointer + 2]
                blue = data[byte_pointer + 3]
                alpha = data[byte_pointer + 4]

                # Increment byte pointer
                byte_pointer += 4
            # 2 bit tags
            else:
                if (current_byte & QOI_MASK) == QOI_OP_INDEX:
                    hash_index = current_byte & 0x0FFF
                    red = pixel_cache[hash_index][0]
                    green = pixel_cache[hash_index][1]
                    blue = pixel_cache[hash_index][2]
                    alpha = pixel_cache[hash_index][3]
                elif (current_byte & QOI_MASK) == QOI_OP_DIFF:
                    red_diff = ((current_byte & 0x0F00) >> 4) + 2
                    green_diff = ((current_byte & 0x0F00) >> 2) + 2
                    blue_diff = ((current_byte & 0x0F00) >> 0) + 2

                    red = previous_pixel[0] + red_diff
                    green = previous_pixel[1] + green_diff
                    blue = previous_pixel[2] + blue_diff
                elif (current_byte & QOI_MASK) == QOI_OP_LUMA:
                    next_byte = data[byte_pointer + 1]
                    green_diff = (current_byte & 0x0FFF) + 32

                    red_green_diff = ((next_byte & 0xFF00) >> 4) + 8
                    blue_green_diff = ((next_byte & 0x00FF) >> 0) + 8

                    red = previous_pixel[0] + red_green_diff + green_diff
                    green = previous_pixel[1] + green_diff
                    blue = previous_pixel[2] + blue_green_diff + green_diff

                    # Increment byte pointer
                    byte_pointer += 1
                elif (current_byte & QOI_MASK) == QOI_OP_RUN:
                    run_length = (current_byte & 0x0FFF) + 1
                    byte_pointer -= 1
            # Increment byte pointer
            byte_pointer += 1

            # Store in cache
            hash_index = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64
            pixel_cache[hash_index][0] = red
            pixel_cache[hash_index][1] = green
            pixel_cache[hash_index][2] = blue
            pixel_cache[hash_index][3] = alpha

        # Set pixel value
        pixel_data[(num_channels * pixel_idx) + 0] = red
        pixel_data[(num_channels * pixel_idx) + 1] = green
        pixel_data[(num_channels * pixel_idx) + 2] = blue
        if num_channels == 4:
            pixel_data[(num_channels * pixel_idx) + 3] = alpha
        # Set previous pixel
        previous_pixel[0] = red
        previous_pixel[1] = green
        previous_pixel[2] = blue
        previous_pixel[3] = alpha

    # Check end bytes
    for _ in range(7):
        if data[byte_pointer] != 0x0000:
            print("SOMETHING WRONG")
        byte_pointer += 1
    if data[byte_pointer] != 0x0001:
        print("SOMETHING WRONG")

    return pixel_data


if __name__ == "__main__":
    encode_flag = False
    if encode_flag:
        print("ENCODING...")
        for cmap_name in cmaps:
            cmap_data = cmaps[cmap_name]
            data = [0 for _ in range(3 * len(cmap_data))]
            for idx, (red, green, blue) in enumerate(cmap_data):
                red = int(255 * red)
                green = int(255 * green)
                blue = int(255 * blue)
                data[(3 * idx) + 0] = red
                data[(3 * idx) + 1] = green
                data[(3 * idx) + 2] = blue
            data_stream = qoi_encode(data, len(cmap_data), 1, 3, False)

            file_name = f"{cmap_name}_colormap.qoi"
            with open(file_name, "wb") as save_file:
                save_file.write(data_stream)
    else:
        print("DECODING...")
        for cmap_name in cmaps:
            cmap_data = cmaps[cmap_name]
            data = [0 for _ in range(3 * len(cmap_data))]

            for idx, (red, green, blue) in enumerate(cmap_data):
                red = int(255 * red)
                green = int(255 * green)
                blue = int(255 * blue)
                data[(3 * idx) + 0] = red
                data[(3 * idx) + 1] = green
                data[(3 * idx) + 2] = blue
            data = qoi_encode(data, len(cmap_data), 1, 3, False)
            rgb_data = qoi_decode(data)

            rgb_data_array = np.zeros(shape=(1, len(cmap_data), 3))

            for value_idx, value in enumerate(rgb_data):
                pixel_idx = value_idx // 3
                channel_idx = value_idx % 3
                rgb_data_array[0, pixel_idx, channel_idx] = value / 255

            fig = plt.figure()
            ax = fig.add_subplot()
            ax.set_title(cmap_name)

            ax.imshow(rgb_data_array)

            plt.show()

            plt.close(fig)
