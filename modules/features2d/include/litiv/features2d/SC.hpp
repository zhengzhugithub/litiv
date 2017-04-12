
// This file is part of the LITIV framework; visit the original repository at
// https://github.com/plstcharles/litiv for more information.
//
// Copyright 2017 Pierre-Luc St-Charles; pierre-luc.st-charles<at>polymtl.ca
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "litiv/utils/opencv.hpp"
#include "litiv/utils/math.hpp"
#include <opencv2/features2d.hpp>

/**
    Shape Context (SC) feature extractor

    (inspired from OpenCV's implementation in the 'shape' module)

    For more details on the different parameters, see S. Belongie, J. Malik and J. Puzicha, "Shape
    Matching and Object Recognition Using Shape Contexts", in IEEE TPAMI2002.

*/
class ShapeContext : public cv::DescriptorExtractor {
public:
    /// default constructor
    ShapeContext(size_t nAngularBins=12, size_t nRadialBins=5, float fInnerRadius=0.1, float fOuterRadius=1.0, bool bRotationInvariant=false, bool bNormalizeBins=true);
    /// loads extractor params from the specified file node @@@@ not impl
    virtual void read(const cv::FileNode&) override;
    /// writes extractor params to the specified file storage @@@@ not impl
    virtual void write(cv::FileStorage&) const override;
    /// returns the window size that will be used around each keypoint (null here; defined at extraction time)
    virtual cv::Size windowSize() const;
    /// returns the border size required around each keypoint in x or y direction (null here; defined at extraction time)
    virtual int borderSize(int nDim=0) const; // typically equal to windowSize().width/2
    /// returns the expected dense descriptor matrix output info, for a given input matrix size/type
    virtual lv::MatInfo getOutputInfo(const lv::MatInfo& oInputInfo) const;
    /// returns the current descriptor size, in bytes (overrides cv::DescriptorExtractor's)
    virtual int descriptorSize() const override;
    /// returns the current descriptor data type (overrides cv::DescriptorExtractor's)
    virtual int descriptorType() const override;
    /// returns the default norm type to use with this descriptor (overrides cv::DescriptorExtractor's)
    virtual int defaultNorm() const override;
    /// return true if detector object is empty (overrides cv::DescriptorExtractor's)
    virtual bool empty() const override;

    /// returns whether descriptor bin arrays will be 0-1 normalized before returning or not
    bool isNormalizingBins() const;
    /// returns the cv::ContourApproximationModes detection strategy to use when finding contours in binary images
    int chainDetectMethod() const;

    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix, and all image points are described (note: descriptors close to borders will be invalid)
    void compute2(const cv::Mat& oImage, cv::Mat& oDescMap);
    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix, and all image points are described (note: descriptors close to borders will be invalid)
    void compute2(const cv::Mat& oImage, cv::Mat_<float>& oDescMap);
    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix
    void compute2(const cv::Mat& oImage, std::vector<cv::KeyPoint>& voKeypoints, cv::Mat_<float>& oDescMap);
    /// batch version of LBSP::compute2(const cv::Mat& image, ...)
    void compute2(const std::vector<cv::Mat>& voImageCollection, std::vector<cv::Mat_<float>>& voDescMapCollection);
    /// batch version of LBSP::compute2(const cv::Mat& image, ...)
    void compute2(const std::vector<cv::Mat>& voImageCollection, std::vector<std::vector<cv::KeyPoint> >& vvoPointCollection, std::vector<cv::Mat_<float>>& voDescMapCollection);

    /// utility function, used to reshape a descriptors matrix to its input image size (assumes fully-dense keypoints over input)
    void reshapeDesc(cv::Size oSize, cv::Mat& oDescriptors) const;
    /// utility function, used to filter out bad keypoints that would trigger out of bounds error because they're too close to the image border
    void validateKeyPoints(std::vector<cv::KeyPoint>& voKeypoints, cv::Size oImgSize) const;
    /// utility function, used to filter out bad pixels in a ROI that would trigger out of bounds error because they're too close to the image border
    void validateROI(cv::Mat& oROI) const;
    /// utility function, used to calculate the (C)EMD-L1 distance between two individual descriptors
    double calcDistance(const float* aDescriptor1, const float* aDescriptor2) const;
    /// utility function, used to calculate the (C)EMD-L1 distance between two individual descriptors
    double calcDistance(const cv::Mat_<float>& oDescriptor1, const cv::Mat_<float>& oDescriptor2) const;

protected:
    /// hides default keypoint detection impl (this class is a descriptor extractor only)
    using cv::DescriptorExtractor::detect;
    /// classic 'compute' implementation, based on DescriptorExtractor's arguments & expected output
    virtual void detectAndCompute(cv::InputArray oImage, cv::InputArray oMask, std::vector<cv::KeyPoint>& voKeypoints, cv::OutputArray oDescriptors, bool bUseProvidedKeypoints=false) override;
    /// number of angular bins to use
    const int m_nAngularBins; // default = 12
    /// number of radial bins to use
    const int m_nRadialBins; // default = 5
    /// inner radius of the descriptor (in 'mean-normalized' space)
    const float m_fInnerRadius; // default = 0.1
    /// outer radius of the descriptor (in 'mean-normalized' space)
    const float m_fOuterRadius; // default = 1.0
    /// defines whether descriptors will be made rotation-invariant or not
    const bool m_bRotationInvariant; // default = false
    /// defines whether descriptor bins will be 0-1 normalized or not
    const bool m_bNormalizeBins; // default = true

private:

    /// generates radius limits mask using internal parameters
    void scdesc_generate_radmask(std::vector<float>&) const;
    /// generates angle limits mask using internal parameters
    void scdesc_generate_angmask(std::vector<float>&) const;
    /// generates EMD distance cost map using internal parameters
    void scdesc_generate_emdmask(cv::Mat_<float>& oEMDCostMap) const;
    /// fills contour point map using provided binary image
    void scdesc_fill_contours(const cv::Mat& oImage);
    /// fills mean-normalized dist map using internal contour/key points
    void scdesc_fill_distmap(float fMeanDist=-1.0f);
    /// fills angle map using internal contour/key points
    void scdesc_fill_angmap();
    /// fills descriptor using internal maps
    void scdesc_fill_desc(cv::Mat_<float>& oDescriptors, bool bGenDescMap);

    // helper variables for internal impl (helps avoid continuous mem realloc)
    std::vector<float> m_vAngularLimits,m_vRadialLimits;
    cv::Mat_<float> m_oDistMap,m_oAngMap,m_oEMDCostMap;
    cv::Mat_<cv::Point2f> m_oKeyPts,m_oContourPts;
    std::vector<int> m_vKeyInliers,m_vContourInliers;
    cv::Mat_<uchar> m_oDistMask;

    cv::Size m_oCurrImageSize;
};