/*
    (C) Copyright 2013 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#ifndef N2D2_DECONVCELL_H
#define N2D2_DECONVCELL_H

#include <cassert>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Activation/Activation.hpp"
#include "Cell.hpp"
#include "Filler/NormalFiller.hpp"
#include "Solver/Solver.hpp"
#include "utils/Registrar.hpp"
#include "controler/Interface.hpp"

#ifdef WIN32
// For static library
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@DeconvCell_Frame@N2D2@@0U?$Registrar@VDeconvCell@N2D2@@@2@A")
#ifdef CUDA
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@DeconvCell_Frame_CUDA@N2D2@@0U?$Registrar@VDeconvCell@N2D2@@@2@A")
#endif
#endif

namespace N2D2 {
class DeconvCell : public virtual Cell {
public:
    typedef std::function<std::shared_ptr<DeconvCell>(
        Network&,
        const std::string&,
        unsigned int,
        unsigned int,
        unsigned int,
        unsigned int,
        unsigned int,
        int,
        int,
        const std::shared_ptr<Activation<Float_T> >&)> RegistryCreate_T;

    static RegistryMap_T& registry()
    {
        static RegistryMap_T rMap;
        return rMap;
    }
    static const char* Type;

    // N = output feature map
    // C = input feature map (channel)
    // H = kernel row
    // W = kernel col
    enum WeightsExportFormat {
        NCHW,
        HWNC
    };

    DeconvCell(const std::string& name,
               unsigned int kernelWidth,
               unsigned int kernelHeight,
               unsigned int nbOutputs,
               unsigned int strideX = 1,
               unsigned int strideY = 1,
               int paddingX = 0,
               int paddingY = 0);
    const char* getType() const
    {
        return Type;
    };
    void setWeightsFiller(const std::shared_ptr<Filler<Float_T> >& filler)
    {
        mWeightsFiller = filler;
    };
    void setBiasFiller(const std::shared_ptr<Filler<Float_T> >& filler)
    {
        mBiasFiller = filler;
    };
    void setWeightsSolver(const std::shared_ptr<Solver<Float_T> >& solver)
    {
        mWeightsSolver = solver;
    };
    void setBiasSolver(const std::shared_ptr<Solver<Float_T> >& solver)
    {
        mBiasSolver = solver;
    };
    virtual void logFreeParameters(const std::string& fileName,
                                   unsigned int output,
                                   unsigned int channel) const;
    virtual void logFreeParameters(const std::string& fileName,
                                   unsigned int output) const;
    virtual void logFreeParameters(const std::string& dirName) const;
    unsigned long long int getNbSharedSynapses() const;
    unsigned long long int getNbVirtualSynapses() const;
    unsigned int getKernelWidth() const
    {
        return mKernelWidth;
    };
    unsigned int getKernelHeight() const
    {
        return mKernelHeight;
    };
    unsigned int getStrideX() const
    {
        return mStrideX;
    };
    unsigned int getStrideY() const
    {
        return mStrideY;
    };
    unsigned int getPaddingX() const
    {
        return mPaddingX;
    };
    unsigned int getPaddingY() const
    {
        return mPaddingY;
    };
    std::shared_ptr<Solver<Float_T> > getWeightsSolver()
    {
        return mWeightsSolver;
    };
    std::shared_ptr<Solver<Float_T> > getBiasSolver()
    {
        return mBiasSolver;
    };
    virtual Float_T getWeight(unsigned int output,
                              unsigned int channel,
                              unsigned int sx,
                              unsigned int sy) const = 0;
    virtual Float_T getBias(unsigned int output) const = 0;
    void setKernel(unsigned int output,
                   unsigned int channel,
                   const Matrix<double>& value,
                   bool normalize);
    virtual Interface<Float_T>* getWeights() { return NULL; };
    virtual void setWeights(unsigned int /*k*/,
                    Interface<Float_T>* /*weights*/,
                    unsigned int /*offset*/) {};
    virtual void exportFreeParameters(const std::string& fileName) const;
    virtual void importFreeParameters(const std::string& fileName,
                                      bool ignoreNotExists = false);
    virtual void logFreeParametersDistrib(const std::string& fileName) const;
    void writeMap(const std::string& fileName) const;
    void discretizeFreeParameters(unsigned int nbLevels);
    void randomizeFreeParameters(double stdDev);
    void getStats(Stats& stats) const;
    virtual ~DeconvCell() {};

protected:
    virtual void setOutputsSize();
    virtual void setWeight(unsigned int output,
                           unsigned int channel,
                           unsigned int sx,
                           unsigned int sy,
                           Float_T value) = 0;
    virtual void setBias(unsigned int output, Float_T value) = 0;

    /// If true, the output neurons don't have bias
    Parameter<bool> mNoBias;
    /// If true, enable backpropogation
    Parameter<bool> mBackPropagate;
    Parameter<WeightsExportFormat> mWeightsExportFormat;

    // Kernel width
    const unsigned int mKernelWidth;
    // Kernel height
    const unsigned int mKernelHeight;
    // Horizontal stride for the convolution
    const unsigned int mStrideX;
    // Vertical stride for the convolution
    const unsigned int mStrideY;
    // Horizontal padding at left and at right
    const int mPaddingX;
    // Vertical padding at top and at bottom
    const int mPaddingY;

    std::shared_ptr<Filler<Float_T> > mWeightsFiller;
    std::shared_ptr<Filler<Float_T> > mBiasFiller;
    std::shared_ptr<Solver<Float_T> > mWeightsSolver;
    std::shared_ptr<Solver<Float_T> > mBiasSolver;
};
}

namespace {
template <>
const char* const EnumStrings<N2D2::DeconvCell::WeightsExportFormat>::data[]
    = {"NCHW", "HWNC"};
}

#endif // N2D2_DECONVCELL_H
