node = hou.pwd()
geo = node.geometry()

# Add code to modify contents of geo.
# Use drop down menu to select examples.

list_curve_points = []
list_mask_points = geo.findPointGroup("StairMask").points()

for point in geo.points():
    if not point in list_mask_points:
        list_curve_points.append(point)

min_x = max_x = None
min_y = max_y = None
min_z = max_z = None
for mask_point in list_mask_points:
    position = mask_point.position()
    if not min_x:
        min_x = position[0]
    else:
        min_x = min(min_x, position[0])
    if not max_x:
        max_x = position[0]
    else:
        max_x = max(max_x, position[0])

        
    if not min_y:
        min_y = position[1]
    else:
        min_y = min(min_y, position[1])
    if not max_y:
        max_y = position[1]
    else:
        max_y = max(max_y, position[1])

    if not min_z:
        min_z = position[2]
    else:
        min_z = min(min_z, position[2])
    if not max_z:
        max_z = position[2]
    else:
        max_z = max(max_z, position[2])
