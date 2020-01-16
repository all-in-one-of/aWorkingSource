# Move Points Up
node = hou.pwd()
geo = node.geometry()

def createPoint(current_point, last_point):
    current_pos = current_point.position()
    last_pos = last_point.position()
    dist = current_pos.distanceTo(last_pos)
    vector = (current_pos - last_pos).normalized()
    normal = current_point.attribValue("N");
    if dist >= 500:
        pt = geo.createPoint()
        pos = last_pos + vector*500
        pt.setPosition(pos)        
        pt.setAttribValue("N", normal)
        createPoint(current_point, pt)
    else:
        return 0

height = node.parm('height').eval()
# Add code to modify contents of geo.
last_point = None
# Add the first point to the end of list.
point_list = []
for point in geo.points():
    point_list.append(point)
point_list.append(point_list[0])
# Caculate and add new point.
for point in point_list:
    current_point = point
    normal = current_point.attribValue("N");
    if last_point:
        createPoint(current_point, last_point)
    last_point = point