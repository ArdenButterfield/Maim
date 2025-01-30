//
// Created by arden on 1/30/25.
//

#ifndef MAIM_PACKETLOSSMODEL_H
#define MAIM_PACKETLOSSMODEL_H

#include "juce_core/juce_core.h"

class PacketLossModel
{
public:
    PacketLossModel(int sampleRate) : fs(sampleRate), counter(0), nextOneMustBeOn(false) {}

    void setParameters(float length, float pulseWidth, float _jitter) {
        onLength = length * fs * pulseWidth;
        offLength = length * fs * (1 - pulseWidth);
        jitter = _jitter;
        setJitteredLengths();
    }

    bool processPacket(int lengthInSamples) {
        bool result = (counter < onLengthWithJitter) || nextOneMustBeOn;
        nextOneMustBeOn = false;
        auto totalLength = onLengthWithJitter + offLengthWithJitter;
        counter += lengthInSamples;
        if (counter > totalLength) {
            counter -= (totalLength);
            setJitteredLengths();
            nextOneMustBeOn = true;
        }
        return result;
    }
private:
    void setJitteredLengths() {
        if (jitter > 0) {
            onLengthWithJitter = random.nextInt({
                onLength - static_cast<int>(jitter * onLength),
                onLength + static_cast<int>(jitter * onLength) + 1});
            offLengthWithJitter = random.nextInt({
                offLength - static_cast<int>(jitter * offLength),
                offLength + static_cast<int>(jitter * offLength) + 1});
        } else {
            onLengthWithJitter = onLength;
            offLengthWithJitter = offLength;
        }
    }
    const int fs;
    int counter;
    int onLength;
    int offLength;
    int onLengthWithJitter;
    int offLengthWithJitter;
    float jitter;
    juce::Random random;
    bool nextOneMustBeOn;
};

#endif //MAIM_PACKETLOSSMODEL_H
