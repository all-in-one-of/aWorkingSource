#include "memory.h"
#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(CacheMethods);

enum CacheParams
{
   p_input
};

node_parameters
{
   AiParameterRGB("input", 0.0f, 0.0f, 0.0f);
}


// the alignment is to make sure this struct takes up 64 bytes; this will
// ensure that when different threads access colors next to each other in
// memory, no false sharing will occur in the CPU cache; for multi-socket CPUs
// this is particularly important, otherwise they bus-lock and the render chugs
struct AS_ALIGN(CACHE_LINE_ALIGNMENT) CacheEntry
{
   AtColor  m_color;
   AtPoint  m_position;
   AtVector m_normal;
   AtPoint  m_ray_origin;
   float    m_u, m_v;
   bool     m_cached;
   
   CacheEntry() : m_u(0.0f), m_v(0.0f), m_cached(false) { }
};


// by using AlignBase<int> this will automatically align any allocations of this
// struct (class overloads of operator new/delete); very handy for the struct
// to say what its alignment requirements are, rather than client code have to
// remember to align it when allocating it
struct NodeResultCache : public AlignBase<CACHE_LINE_ALIGNMENT>
{
   CacheEntry m_cache[AI_MAX_THREADS];
};


node_initialize
{
}

node_update
{
   NodeResultCache *cache = reinterpret_cast<NodeResultCache*>(AiNodeGetLocalData(node));
   if (!cache)
   {
      cache = new NodeResultCache;
      AiNodeSetLocalData(node, cache);
   }
}

node_finish
{
   NodeResultCache *cache = reinterpret_cast<NodeResultCache*>(AiNodeGetLocalData(node));
   delete cache;
}


shader_evaluate
{
   NodeResultCache *cache = reinterpret_cast<NodeResultCache*>(AiNodeGetLocalData(node));
   if (!cache)
   {
      sg->out.RGB = AiShaderEvalParamRGB(p_input);
      return;
   }
   
   CacheEntry& entry = cache->m_cache[sg->tid];
   if (entry.m_cached &&
       entry.m_position == sg->P &&
       entry.m_normal == sg->N &&
       entry.m_ray_origin == sg->Ro &&
       entry.m_u == sg->u &&
       entry.m_v == sg->v)
   {
      sg->out.RGB = entry.m_color;
      return;
   }
   
   entry.m_cached = true;
   entry.m_position = sg->P;
   entry.m_normal = sg->N;
   entry.m_ray_origin = sg->Ro;
   entry.m_u = sg->u;
   entry.m_v = sg->v;
   // cache off the actual result (evaluate the connected network)
   entry.m_color = AiShaderEvalParamRGB(p_input);
   sg->out.RGB = entry.m_color;
}


void registerCache(AtNodeLib *node)
{
   node->methods     = CacheMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "cache";
   node->node_type   = AI_NODE_SHADER;
}

