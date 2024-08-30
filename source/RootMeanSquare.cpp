/*
  ==============================================================================

    RootMeanSquare.cpp
    Created: 8 Apr 2023 1:18:52pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "RootMeanSquare.h"


RootMeanSquare::RootMeanSquare()
{
    rms_coeff = 1;
    decay_time = 0;
    rmsVal = 0;
}

int RootMeanSquare::set_decay_time(float num_samples)
{
    if (num_samples == decay_time) {
        return 0;
    }
    if (num_samples > 0) {
        decay_time = num_samples;
        
        rms_coeff = 1 - std::exp(-2.2 / num_samples);
        return 0;
    }
    if (num_samples == 0) {
        decay_time = num_samples;
        rms_coeff = 1;
        return 0;
    }
    return 1;
}

float RootMeanSquare::advance(float *samples, int numSamples)
{
    float energy;
    for (int i = 0; i < numSamples; ++i) {
        energy = samples[i] * samples[i];
        rmsVal += rms_coeff * (energy - rmsVal);
    }
    return std::sqrt(rmsVal);
}

float RootMeanSquare::tick(float sample)
{
    float energy = sample * sample;
    rmsVal += rms_coeff * (energy - rmsVal);
    return std::sqrt(rmsVal);
}
