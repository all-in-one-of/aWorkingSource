Batch Viewer Modifier Plug-in
=============================

This is a simple example plug-in that shows how to use a VMP's 'onFrameEnd()'
callback to implement draw-call batching. This example is not intended to be
complete, efficient or usable in production.

Requirements
------------

This example uses Glew to initialize OpenGL extensions.
It has been tested with Glew 1.13.0, and requires a version that supports
'glMultiDrawElementsIndirect()' method (OpenGL >= 4.3, Glew >= 1.9.0). This
version of Glew should be included in your library search path when launching
Katana.

Implementation Details
----------------------

The VMP renders all locations with type "customVMP" with a single draw
command ('glMultiDrawElementsIndirect()').

The VMP expects a list of vertices in the 'geometry.point.P' attribute, and
looks for (but does not require) a list of normals in the 'geometry.point.N'
attribute. If given, the number of normals must match the number of vertices.

The shader displays the normals as defined in object space.
If no normals are found, a placeholder normal is generated for each vertex
([0,1,0] -> green).

This VMP has a number of limitations:

* Only triangles and quads are supported.
* Animation is not supported.
* Resources are not deallocated.
* Once geometry data for a location has been read, it will not be updated.
* No more than MAX_OBJECTS locations, MAX_VERTEX_COUNT vertices and
  MAX_INDICES_COUNT indices are supported.

This VMP implements the following methods:

### setup()
  1. Used once for initializing resources ('InitResources()'), allocating
     a single VBO/IBO.
  2. It's used the first time a 'customVMP' location is expanded.

  We use this callback to read geometry data from the location itself, and
  fill the vertex and index buffers.

  A 'DrawElementsIndirectCommand' struct is added to 'commands' and used for
  controlling rendering.

### deepSetup()
  This function is used to update the transform matrix of a given 'customVMP'
  location when it changes.

### draw()
  As this function is called only if the location bounding box is inside the
  view frustum, it is used to establish visibility.

  We set the 'instanceCount' to 1 for the current location's drawing command
  so it will be rendered during the current frame.

### cleanup()
  This function is used to update the draw command to not draw the given
  location that has been collapsed.

### onFrameEnd()
  This function is used for drawing the visible objects.

  It binds the necessary buffers, and updates the per object transform matrix.

  After the draw-call we leave the OpenGL state clean.

### onFrameBegin()
  This function is not used in this example.

