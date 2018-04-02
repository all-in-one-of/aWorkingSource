#ifndef TRANSFORM_MANIPULATORS_MESH_GENERATION_H
#define TRANSFORM_MANIPULATORS_MESH_GENERATION_H

#include <memory>
#include "FnDrawable.h"

namespace Foundry
{
namespace Katana
{
namespace ViewerUtils
{

void generateCylinder(
    const Vec3f& origin, float base, float top, float height, Drawable& mesh);

void generateSquare(const Vec3f& origin, float length, Drawable& mesh);

void generateCube(const Vec3f& origin, float length, Drawable& mesh);

void generateTorus(
    const Vec3f& origin, float centerRadius, float tubeRadius, Drawable& mesh);

void generateTriangle(
    const Vec3f& origin, float cathetusLength, float thickness, Drawable& mesh);

}  // namespace ViewerUtils
}  // namespace Katana
}  // namespace Foundry

#endif  // TRANSFORM_MANIPULATORS_MESH_GENERATION_H
