Katana Viewer Plug-ins
=======================

For build instructions, please refer to plugins/README.md.

- CameraLocationDelegate
- LightLocationDelegate
- TransformManipulators

Requirements
------------
- **TransformManipulators:** GLEW >= 1.9.0, OpenEXR >= 2.0

If these are are not found, the corresponding plug-ins will not be built. You
can specify the locations of these libraries using CMake's `CMAKE_PREFIX_PATH`
variable. On Linux, if you have these libraries installed in well-known
locations, specifying `CMAKE_PREFIX_PATH` may not be necessary.