//
// Created by arden on 8/29/24.
//

#ifndef MAIM_CODECCONTROLLER_H
#define MAIM_CODECCONTROLLER_H
#include <vector>

class CodecController
{
public:
    CodecController() = default;
    virtual ~CodecController() = default;

    virtual bool init(int sampleRate,
        int maxSamplesPerBlock,
        int bitrate
    ) = 0;
    virtual void deInit() = 0;
    virtual bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) = 0;

    virtual int getBitrate() = 0;
    virtual void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd) = 0;
    virtual void setMDCTbandstepBends(bool invert, int step) = 0;
    virtual void setMDCTpostshiftBends(int h_shift, float v_shift) = 0;
    virtual void setMDCTwindowincrBends(int window_incr) = 0;
    virtual void setMDCTBandReassignmentBends(int* order) = 0;
    virtual void setBitrateSquishBends(float squish) = 0;
    virtual void setThresholdBias(float bias) = 0;
    virtual void setMDCTfeedback(float feedback) = 0;

    virtual float* getPsychoanalThreshold() = 0;
    virtual float* getPsychoanalEnergy() = 0;
    virtual float* getMDCTpreBend() = 0;
    virtual float* getMDCTpostBend() = 0;
    virtual int getShortBlockStatus() = 0;
protected:
    virtual bool init_encoder() = 0;
    virtual void deinit_encoder() = 0;
    virtual int validate_bitrate(int bitrate) = 0;
    virtual int validate_samplerate(int samplerate) = 0;
    static int getClosest(int target, const std::vector<int>& options);

    int bitrate;
    int samplerate;

    static const int MP3FRAMESIZE = 1152;
};

#endif //MAIM_CODECCONTROLLER_H
