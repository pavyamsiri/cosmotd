#!/usr/bin/env python3

from mpl_colormap_data import cmaps
from PIL import Image
import numpy as np
import struct

ctdd_flag = True

if __name__ == "__main__":
    for cmap_name in cmaps:
        cmap_data = cmaps[cmap_name]
        # Write as .ctdd
        if ctdd_flag:
            file_name = f"{cmap_name}_colormap.ctdd"
            with open(file_name, "wb") as save_file:
                # The number of fields in this file
                save_file.write(struct.pack("<I", 1))
                # Header of a single field
                M = 1
                N = len(cmap_data)
                save_file.write(struct.pack("<I", M))
                save_file.write(struct.pack("<I", N))

                # Field values
                for (idx, [red, green, blue]) in enumerate(cmap_data):
                    # Write data
                    save_file.write(struct.pack("<f", red))
                    save_file.write(struct.pack("<f", green))
                    save_file.write(struct.pack("<f", blue))
        else:
            red_array = np.zeros(len(cmap_data), dtype=np.uint8)
            green_array = np.zeros(len(cmap_data), dtype=np.uint8)
            blue_array = np.zeros(len(cmap_data), dtype=np.uint8)
            for (idx, [red, green, blue]) in enumerate(cmap_data):
                red_array[idx] = int(255 * red)
                green_array[idx] = int(255 * green)
                blue_array[idx] = int(255 * blue)
            rgb_array = np.dstack((red_array, green_array, blue_array))
            img = Image.fromarray(rgb_array, mode="RGB")
            img.save(f"{cmap_name}_colormap.png")
