#pragma once

class SgUvManager {
public:
   AtShaderGlobals* sg;
   float u, v;

   SgUvManager(AtShaderGlobals* _sg)
      :sg(_sg)
      {};

   void push(float _u, float _v){
      u = sg->u; v = sg->v;
      sg->u = _u; sg->v = _v;
   }

   void push(){
      u = sg->u; v = sg->v;
   }

   void set(float _u, float _v){
      /* only set u and v of sg, not push anything. */
      sg->u = _u; sg->v = _v;
   }

   void pop(){
      sg->u = u;
      sg->v = v;
   }

};

inline AtVector filterStep(const AtVector& edge, const AtVector& x){
   /* TODO: *Filtered* version if necessery. */
   return aivec((float)(x.x>edge.x),
                (float)(x.y>edge.y),
                (float)(x.z>edge.z));
}

template<class T>
inline T over(const T& bg, const T& fg){
   /* perform pre-multiplied merge operation. */
   return bg*(1-fg.a) + fg;
}