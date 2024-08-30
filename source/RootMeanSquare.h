/*
  ==============================================================================

    RootMeanSquare.h
    Created: 8 Apr 2023 1:18:52pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <cmath>

class RootMeanSquare
{
public:
    RootMeanSquare();

    int set_decay_time(float num_samples);
    float advance(float* samples, int numSamples);
    float tick(float sample);
    
private:
    float rmsVal;
    float rms_coeff;
    float decay_time;
};
