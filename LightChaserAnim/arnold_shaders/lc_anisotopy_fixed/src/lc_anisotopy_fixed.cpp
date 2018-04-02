#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(AnisotopyFixedMethods);

enum AnisotopySpaceEnum
{
	AF_DPDX = 0,
	AF_DPDY,
	AF_PREF,
};

static const char* anisotopySpaceNames[] =
{
	"dPdx",
	"dPdy",
	"Pref",
	NULL
};

enum Params 
{
	p_anisotopy_vector = 0,
	p_standard_surface,
};

node_parameters
{
	AiParameterEnum("anisotopy_vector", 0, anisotopySpaceNames);
	AiParameterClosure("standard_surface");
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
	int anisotopy_vector = AiShaderEvalParamInt(p_anisotopy_vector);
	// get pref from the geometry
	AtVector Pref,dPdx,dPdy,result;
	if (AiUDataGetVec(AtString("Pref"), Pref))
	{	
		// just caculate Pref derivatives vector
		AiMsgWarning("[anisotopy_fix] Got Pref!");
		AiUDataGetDxyDerivativesVec(AtString("Pref"), dPdx, dPdy);
	}
	else 
	{
		// if we could find Pref from user data,we would use P instand of Pref.
		AiMsgWarning("[anisotopy_fix] Could not get Pref?");
		Pref = sg->P;
		dPdx = AiShaderGlobalsTransformVector(sg, sg->dPdx, AI_WORLD_TO_OBJECT);
		dPdy = AiShaderGlobalsTransformVector(sg, sg->dPdy, AI_WORLD_TO_OBJECT);	
	}

	switch(anisotopy_vector)
	{
		case AF_DPDX:
			result = dPdx;
			break;
		case AF_DPDY:
			result = dPdy;
			break;
		case AF_PREF:
			result =Pref;
			break;
		default:
			result = dPdx;
			break;
 	}

	sg->dPdu = result;

	sg->out.CLOSURE() = AiShaderEvalParamClosure(p_standard_surface);
}

node_loader
{
	if (i > 0)
	  return false;
	node->methods = AnisotopyFixedMethods;
	node->output_type = AI_TYPE_CLOSURE;
	node->name = "lc_anisotopy_fixed";
	node->node_type = AI_NODE_SHADER;
	strcpy(node->version, AI_VERSION);
	return true;
}