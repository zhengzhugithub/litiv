
// This file is part of the LITIV framework; visit the original repository at
// https://github.com/plstcharles/litiv for more information.
//
// Copyright 2015 Pierre-Luc St-Charles; pierre-luc.st-charles<at>polymtl.ca
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

// note: we should already be in the litiv namespace
#ifndef __LITIV_DATASETS_IMPL_H
#error "This file should never be included directly; use litiv/datasets.hpp instead"
#endif //__LITIV_DATASETS_IMPL_H

template<ParallelUtils::eParallelAlgoType eEvalImpl>
struct Dataset_<eDatasetType_VideoSegm,eDataset_VideoSegm_CDnet,eEvalImpl> :
        public IDataset_<eDatasetType_VideoSegm,eDataset_VideoSegm_CDnet,eEvalImpl> {
protected: // should still be protected, as creation should always be done via datasets::create
    Dataset_(
            const std::string& sOutputDirName, // output directory (full) path for debug logs, evaluation reports and results archiving (will be created in CDnet dataset folder)
            bool bSaveOutput=false, // defines whether results should be archived or not
            bool bUseEvaluator=true, // defines whether results should be fully evaluated, or simply acknowledged
            bool bForce4ByteDataAlign=false, // defines whether data packets should be 4-byte aligned (useful for GPU upload)
            double dScaleFactor=1.0, // defines the scale factor to use to resize/rescale read packets
            bool b2014=true // defines whether to use the 2012 or 2014 version of the dataset (each should have its own folder in dataset root)
    ) :
            IDataset_<eDatasetType_VideoSegm,eDataset_VideoSegm_CDnet,eEvalImpl>(
                    b2014?"CDnet 2014":"CDnet 2012",
                    b2014?"CDNet2014/dataset":"CDNet/dataset",
                    std::string(DATASET_ROOT)+"/"+std::string(b2014?"CDNet2014/":"CDNet/")+sOutputDirName+"/",
                    "bin",
                    ".png",
                    std::vector<std::string>{"baseline_highway_cut2"},//b2014?std::vector<std::string>{"badWeather","baseline","cameraJitter","dynamicBackground","intermittentObjectMotion","lowFramerate","nightVideos","PTZ","shadow","thermal","turbulence"}:std::vector<std::string>{"baseline","cameraJitter","dynamicBackground","intermittentObjectMotion","shadow","thermal"},
                    std::vector<std::string>{},
                    b2014?std::vector<std::string>{"thermal","turbulence"}:std::vector<std::string>{"thermal"},
                    1,
                    bSaveOutput,
                    bUseEvaluator,
                    bForce4ByteDataAlign,
                    dScaleFactor
            ) {}
};

template<>
struct DataProducer_<eDatasetType_VideoSegm,eDataset_VideoSegm_CDnet,eNotGroup> :
        public IDataProducer_<eDatasetType_VideoSegm,eNotGroup> {
protected:
    virtual void parseData() override final {
        std::vector<std::string> vsSubDirs;
        PlatformUtils::GetSubDirsFromDir(getDataPath(),vsSubDirs);
        auto gtDir = std::find(vsSubDirs.begin(),vsSubDirs.end(),getDataPath()+"/groundtruth");
        auto inputDir = std::find(vsSubDirs.begin(),vsSubDirs.end(),getDataPath()+"/input");
        if(gtDir==vsSubDirs.end() || inputDir==vsSubDirs.end())
            lvErrorExt("CDnet sequence '%s' did not possess the required groundtruth and input directories",getName().c_str());
        PlatformUtils::GetFilesFromDir(*inputDir,m_vsInputFramePaths);
        PlatformUtils::GetFilesFromDir(*gtDir,m_vsGTFramePaths);
        if(m_vsGTFramePaths.size()!=m_vsInputFramePaths.size())
            lvErrorExt("CDnet sequence '%s' did not possess same amount of GT & input frames",getName().c_str());
        m_oROI = cv::imread(getDataPath()+"/ROI.bmp",cv::IMREAD_GRAYSCALE);
        if(m_oROI.empty())
            lvErrorExt("CDnet sequence '%s' did not possess a ROI.bmp file",getName().c_str());
        m_oROI = m_oROI>0; // @@@@@ check throw here???
        m_oSize = m_oROI.size();
        m_nFrameCount = m_vsInputFramePaths.size();
        CV_Assert(m_nFrameCount>0);
        // note: in this case, no need to use m_vnTestGTIndexes since all # of gt frames == # of test frames (but we assume the frames returned by 'GetFilesFromDir' are ordered correctly...)
    }
    virtual cv::Mat _getGTPacket_impl(size_t nIdx) override final {
        cv::Mat oFrame = cv::imread(m_vsGTFramePaths[nIdx],cv::IMREAD_GRAYSCALE);
        if(oFrame.empty())
            oFrame = cv::Mat(m_oSize,CV_8UC1,cv::Scalar_<uchar>(DATASETUTILS_VIDEOSEGM_OUTOFSCOPE_VAL));
        else if(oFrame.size()!=m_oSize)
            cv::resize(oFrame,oFrame,m_oSize,0,0,cv::INTER_NEAREST);
        return oFrame;
    }
};