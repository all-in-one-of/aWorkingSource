# -*- coding: utf-8 -*-

def subtractImages(path1, path2, savepath):
    from PIL import Image,ImageChops
    import numpy as np
    # Open the images
    image1 = Image.open(path1);
    image2 = Image.open(path2);

    # Get the image buffer as ndarray
    buffer1 = np.asarray(image1);
    buffer2 = np.asarray(image2);
    # Subtract image2 from image1
    buffer3 = buffer1 - buffer2;
    # Construct a new Image from the resultant buffer
    result = Image.fromarray(buffer3);
    result.save(savepath)

# path = "D:/CodeGit/Falcon_Houdini/Houdini/Falcon_v2/wd/VSDemo/H2U"
# layer_mask_list = [
#     "earth",
#     "grass",
# ]
# swamp_mask = "swamp"
# import os
# swamp_mask_file_path = os.path.join(path, coord_folder, swamp_mask+".png")
# for layer_mask in layer_mask_list:
#     layer_mask_file_path = os.path.join(path, coord_folder, layer_mask+".png")
#     if os.path.isfile(layer_mask_file_path) and os.path.isfile(swamp_mask_file_path):
#         subtractImages(layer_mask_file_path, swamp_mask_file_path, layer_mask_file_path)

# import sys
# sys.path.append("D:/CodeGit/TencentWorks/QuickScripts")
# import ImageComposite