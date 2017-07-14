// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_XFORMMATRIX_H
#define FNRENDEROUTPUTUTILS_XFORMMATRIX_H

#include <vector>

#ifdef _WIN32
  #define RENDEROUTPUTUTILSAPI
#else
  #define RENDEROUTPUTUTILSAPI __attribute__ ((visibility("default")))
#endif

// TODO: Remove this whole file

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ...
     */
    class RENDEROUTPUTUTILSAPI XFormMatrix
    {
    public:
        /**
         * XFormMatrix
         */
        XFormMatrix(const double *values);

        /**
         * XFormMatrix
         */
        XFormMatrix(const XFormMatrix& matrix);

        /**
         * getValues
         */
        const double * getValues() const;

        /**
         * invert
         */
        bool invert();

    private:
        double m[16];
    };
    /**
     * @}
     */

    typedef std::vector<XFormMatrix> XFormMatrixVector;
    typedef std::vector<float> TimeSamplesVector;

}
}
}

namespace FnKat = Foundry::Katana;

#endif
