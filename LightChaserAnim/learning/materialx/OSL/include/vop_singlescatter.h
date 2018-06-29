//
// This public-domain source is included in this distribution with the kind
// permission of its creator, Mario Marengo of AXYZ FX. For more information
// regarding this shader, please visit:
//
//	http://odforce.net/forum/index.php?showtopic=2262
//	http://www.sidefx.com/index.php?option=com_wrapper&Itemid=146
//

/* Evaluate the Henyey-Greenstein phase function for two vectors with
   an asymmetry value g.  v1 and v2 should be normalized and g should 
   be in the range (-1, 1).  Negative values of g correspond to more
   back-scattering and positive values correspond to more forward scattering.
*/
float
phase(vector v1, vector v2, float g)
{
    float costheta = vop_dot(-v1, v2);
    return (1.0 - g*g) / pow(1.0 + g*g - 2.*g*costheta, 1.5);
}

/* Compute a the single-scattering approximation to scattering from
   a one-dimensional volumetric surface.  Given incident and outgoing
   directions wi and wo, surface normal n, asymmetry value g (see above),
   scattering albedo (between 0 and 1 for physically-valid volumes),
   and the depth of the volume, use the closed-form single-scattering
   equation to approximate overall scattering.
*/
float
singleScatter(vector in, vector out, normal nn, 
	float g, float albedo, float depth) 
{
    float win = abs(vop_dot(in, nn));
    float won = abs(vop_dot(out, nn));
    float offset = phase(out, in, g);

    float scatter = albedo * offset/(win + won);
    scatter *= 1.0 - exp(-(1.0/win + 1.0/won) * depth);

    return scatter;
}

vector
efresnel(vector ii, normal nn, float eta, output float Kr, output float Kt) 
{
    vector R, T;
    fresnel(ii, nn, eta, Kr, Kt, R, T);
    Kr = vop_smooth(0.0, 0.5, Kr, 1);
    Kt = 1.0 - Kr;
    return normalize(T);
}
