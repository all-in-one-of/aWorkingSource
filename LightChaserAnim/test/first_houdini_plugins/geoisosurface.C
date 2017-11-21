#include <GU/GU_Detail.h>

static float densityFunction(const UT_Vector3 &P)
{
	return 1 - P.length(); // Return signed distance to unit sphere
}

int main(int argc, char *argv[])
{
	GU_Detail            gdp;
	UT_BoundingBox       bounds;

	bounds.setBounds(-1, -1, -1, 1, 1, 1);
	gdp.polyIsoSurface(HDK_Sample::densityFunction, bounds, 20, 20, 20);
	gdp.save("sphere.bgeo", true, NULL);

	return 0;
}
