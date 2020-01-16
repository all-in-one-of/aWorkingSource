node = hou.pwd()
geo = node.geometry()

import math
# Add code to modify contents of geo.
# Use drop down menu to select examples.
list_x = []
list_z = []
first_point = geo.points()[0]
first_point.setAttribValue("N", hou.Vector3(0, 0, 1))
for point in geo.points():
    position = point.position()
    list_x.append(position[0])
    list_z.append(position[2])

x = z = 0
for temp_x in list_x:
    x = max(x, temp_x)
for temp_z in list_z:
    z = max(z, temp_z)

position = [x, 0, z]
pointgroup = geo.createPointGroup("FloorPoints")
pointgroup.add(first_point)
def createPoint(current_point, max_pos, size = 50):
    current_pos = current_point.position()
    if current_pos[0] >= max_pos[0] or current_pos[2] >=max_pos[2]:
        return 0
    normal = current_point.attribValue("N");

    vector_nextpoint = hou.Vector3(0, 0, 1)
    vector_nextline = hou.Vector3(1, 0, 0)
    pos = current_pos + vector_nextpoint*size

    if pos[2] < max_pos[2]:
        pt = geo.createPoint()
        pt.setPosition(pos)        
        pt.setAttribValue("N", hou.Vector3(0, 0, 1))
        createPoint(pt, max_pos)
        pointgroup.add(pt)
    else:
        pos = hou.Vector3(current_pos[0], 0, 0) + vector_nextline*size
        if pos[0] >= max_pos[0]:
            return
        pt = geo.createPoint()
        pt.setPosition(pos)        
        pt.setAttribValue("N", hou.Vector3(0, 0, 1))
        createPoint(pt, max_pos)
        pointgroup.add(pt)

createPoint(first_point, position)
