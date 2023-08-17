# Latency notes

We've come to that point in the project where everything's wrapping up, but we still need to figure out how much latency we're incurring, so we can tell the DAW, and also so we can implement a Dry/wet knob. I made a quick utility to measure latency using impulses; unfortunately, it showed an inconsistent latency based on encoder (makes sense), bitrate (not great), and block size (worrying).

So now we dive into the fun adventure of figuring out what the latency of MAIM, and making it consistent as much as possible.

Strap in!

## What do we expect to see?

Diving into `libmp3lame/encoder.h` gives us the following helpful information:

```c
/***********************************************************************
*
*  encoder and decoder delays
*
***********************************************************************/

/* 
 * layer III enc->dec delay:  1056 (1057?)   (observed)
 * layer  II enc->dec delay:   480  (481?)   (observed)
 *
 * polyphase 256-16             (dec or enc)        = 240
 * mdct      256+32  (9*32)     (dec or enc)        = 288
 * total:    512+16
 *
 * My guess is that delay of polyphase filterbank is actualy 240.5
 * (there are technical reasons for this, see postings in mp3encoder).
 * So total Encode+Decode delay = ENCDELAY + 528 + 1
 */

/* 
 * ENCDELAY  The encoder delay.  
 *
 * Minimum allowed is MDCTDELAY (see below)
 *  
 * The first 96 samples will be attenuated, so using a value less than 96
 * will result in corrupt data for the first 96-ENCDELAY samples.
 *
 * suggested: 576
 * set to 1160 to sync with FhG.
 */

#define ENCDELAY      576



/*
 * make sure there is at least one complete frame after the
 * last frame containing real data
 *
 * Using a value of 288 would be sufficient for a 
 * a very sophisticated decoder that can decode granule-by-granule instead
 * of frame by frame.  But lets not assume this, and assume the decoder  
 * will not decode frame N unless it also has data for frame N+1
 *
 */
/*#define POSTDELAY   288*/
#define POSTDELAY   1152



/* 
 * delay of the MDCT used in mdct.c
 * original ISO routines had a delay of 528!  
 * Takehiro's routines: 
 */

#define MDCTDELAY     48
#define FFTOFFSET     (224+MDCTDELAY)

/*
 * Most decoders, including the one we use, have a delay of 528 samples.  
 */

#define DECDELAY      528
```

Ok, so Encode + Decode delay = 576 + 528 = 1104 samples right? Not quite. The tests that I ran were giving delays in the 3000s and 4000s -- and sometimes higher with Blade. The encoder can't return anything until it has a full frame, And the same is true with the decoder. Additionally, our MP3 processor objects store buffers of input audio, compressed MP3 data, and output audio as well. How long are those buffers anyway? And do they need to be as long as they are? Let's find out.

### What about Blade?

The documentation for Blade is a bit more scarce than for Lame. We have these two definitions in `encode.c`:

```c
#define		FLUSH					1152
#define		DELAY					 768
```

These are used when rebuffering audio, and it is not entirely clear where the delay number comes from. The number 768 appears nowhere else in the code, and this macro is only used once.

## Our Buffers

Currently, this is what goes down when we start up the plugin:

```
lame0 init 96000 512 112
lame1 init 96000 512 8
lame0in: 512    enc: 336        dec: 0  outbuf: 0
lame1in: 512    enc: 0  dec: 0  outbuf: 0
can't copy out 512
can't copy out 512
lame0in: 512    enc: 0  dec: 1152       outbuf: 1152
lame1in: 512    enc: 0  dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1792
lame1in: 512    enc: 96 dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1280
lame1in: 512    enc: 0  dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1920
lame1in: 512    enc: 96 dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
JUCE v7.0.5
lame0in: 512    enc: 0  dec: 0  outbuf: 1408
lame1in: 512    enc: 0  dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 896
lame1in: 512    enc: 0  dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1536
lame1in: 512    enc: 96 dec: 0  outbuf: 0
can't copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1024
lame1in: 512    enc: 0  dec: 3456       outbuf: 3456
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1664
lame1in: 512    enc: 96 dec: 1152       outbuf: 4096
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1152
lame1in: 512    enc: 0  dec: 0  outbuf: 3584
lame1 copy out 512
lame0 copy out 512
JUCE v7.0.5
lame0in: 512    enc: 336        dec: 1152       outbuf: 1792
lame1in: 512    enc: 96 dec: 1152       outbuf: 4224
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1280
lame1in: 512    enc: 0  dec: 0  outbuf: 3712
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1920
lame1in: 512    enc: 96 dec: 1152       outbuf: 4352
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1408
lame1in: 512    enc: 0  dec: 0  outbuf: 3840
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 896
lame1in: 512    enc: 0  dec: 0  outbuf: 3328
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1536
lame1in: 512    enc: 96 dec: 1152       outbuf: 3968
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1024
lame1in: 512    enc: 0  dec: 0  outbuf: 3456
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 1664
lame1in: 512    enc: 96 dec: 1152       outbuf: 4096
lame1 copy out 512
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1152
lame1in: 512    enc: 0  dec: 0  outbuf: 3584
lame1 copy out 512
lame0 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 4224
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 3712
lame1 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 4352
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 3840
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 3328
lame1 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 3968
lame1 copy out 512
...
```

We initialize the current controller (lame0) with the default bitrate, and then start working on lame1 as the off controller with the new bitrate, to get ready to switch to it. Each frame, we pump the frame's samples into both controllers for the stated transition period, at which point we retire lame0 and switch over to lame1 for good.

Already, we can see a problem with this approach. lame1 needs more samples before it is ready for decoding. However, by the time it *can* copy out, it copies out a great number of samples (3456), at which point it sits in an equilibrium, never burning through that output buffer.

What do we need to do instead? We don't want to be in a situation where we don't have any samples to give back to the audioprocessor, which would result in an embarrassing gap of silence. The decoder is always going to give us a multiple of 1152 samples. 

If our DAW's block size was always 1152 samples then this would be perfect -- but of course it isn't. If the DAW block size is *less* than 1152 samples, then we need to make sure that we have at least `1152 - DAWBufferSize` samples in our output queue. If the DAW block size is more than 1152 samples, I *think* it's enough to have a reserve of `1152 - (DAWBufferSize % 1152)` samples. 

However, this might get tricky because we don't actually know the block size -- only the *maximum* block size (and even then it isn't guaranteed). To be safe, and keep things from getting too confusing in the code, let's just say that we need an output buffer with a minimum of 1152 samples in it. This means that we're going to have 1152 samples of latency, on top of encoding and decoding latency, so 576 + 528 + 1152 = 2256 samples total for Lame, and 786 + 528 + 1152 = 2466 samples for Blade. (Note that we can cut down on the Lame encoding delay, and potentially also the Blade encoding delays; I'm not sure if we can do anything about the decoding delay.)

## How can we turn this into code?

One easy fix would be to dequeue samples from the off controller's output queue down to 1152 right before swapping it in. The only issue with this is it still wouldn't give us guaranteed latency: the old encoder and the new encoder might have different numbers of samples "stored up" for the next encoding.

Another possible solution could be to match the number of samples in the old encoder's output buffer. I think this is correct, although I would need to sit down and do the math.

Would this work?

```
start: 
    put 1152 samples of silence in the output queue of the current buffer.

switch:
    start with 0 samples in the output queue of the off buffer
    while samples in off controller output queue < 1152:
        put frame into both controllers
        dequeue result samples from current controller
    let n be the number of samples left in the current controller output queue
    discard all but the last n samples in the off controller output queue
    fade from the current controller to the off controller
    make off controller the current controller
```

I think this would work, though I do want to draw up more of a proof. At the very least, this seems simpler than my current solution, which draws me to it.

Let's try it out.

In MP3Controller, we add the following method:

```c++
void MP3Controller::setOutputBufferToSilence (int numSamples)
{
    outputBufferL->clear();
    outputBufferR->clear();
    for (auto i = 0; i < numSamples; ++i) {
        outputBufferL->enqueue(0);
        outputBufferR->enqueue(0);
    }
}
```

To MP3ControllerManager's init method, we add

```c++
    currentController->setOutputBufferToSilence(MP3FRAMESIZE);
```

We re-write the process block method as well:

```c++
void MP3ControllerManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (parametersNeedUpdating) {
        updateParameters();
    }
    
    if (buffer.getNumChannels() != 2) {
        return;
    }
    auto samplesL = buffer.getWritePointer(0);
    auto samplesR = buffer.getWritePointer(1);

    for (int start = 0; start < buffer.getNumSamples(); start += samplesPerBlock) {
        int length = std::min(buffer.getNumSamples() - start, samplesPerBlock);
        currentController->addNextInput(samplesL, samplesR, length);
        if (wantingToSwitch)
        {
            offController->addNextInput (samplesL, samplesR, length);
            auto extraSamples = offController->samplesInOutputQueue() - currentController->samplesInOutputQueue();
            if (extraSamples >= 0)
            {
                offController->copyOutput (nullptr, nullptr, extraSamples);
                auto tempBuffer = juce::AudioBuffer<float> (buffer.getNumChannels(),
                    length);
                auto tempL = tempBuffer.getWritePointer (0);
                auto tempR = tempBuffer.getWritePointer (1);
                currentController->copyOutput (tempL, tempR, length);
                offController->copyOutput (samplesL, samplesR, length);
                buffer.applyGainRamp (start, length, 0, 1);
                buffer.addFromWithRamp (0, start, tempL, length, 1, 0);
                buffer.addFromWithRamp (1, start, tempR, length, 1, 0);
                currentControllerIndex = (currentControllerIndex + 1) % 2;
                currentController = offController;
                currentBitrate = desiredBitrate;
                currentEncoder = desiredEncoder;
                offController = nullptr;
                wantingToSwitch = false;
                continue;
            }
        }
        if (!currentController->copyOutput(samplesL, samplesR, length)) {
            memset(samplesL, 0, sizeof(float) * length);
            memset(samplesR, 0, sizeof(float) * length);
        }

        samplesL += samplesPerBlock;
        samplesR += samplesPerBlock;
    }
}
```

And remove some no-longer-relevant code here and there.

So what happens?

```
lame0 init 48000 512 112
lame1 init 48000 512 8
lame0in: 512    enc: 336        dec: 0  outbuf: 1152
lame1in: 512    enc: 0  dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 1152       outbuf: 1792
lame1in: 512    enc: 0  dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
lame1in: 512    enc: 96 dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1920
lame1in: 512    enc: 0  dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2560
lame1in: 512    enc: 96 dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 2048
lame1in: 512    enc: 0  dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1536
lame1in: 512    enc: 0  dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2176
lame1in: 512    enc: 96 dec: 0  outbuf: 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1664
lame1in: 512    enc: 0  dec: 3456       outbuf: 3456
lame1 copy out 1792
lame0 copy out 512
lame1 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 2304
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 1792
lame1 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 2432
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 1920
lame1 copy out 512
lame1in: 512    enc: 96 dec: 1152       outbuf: 2560
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 2048
lame1 copy out 512
lame1in: 512    enc: 0  dec: 0  outbuf: 1536
lame1 copy out 512
...
```

The output buffer's get pretty big. In fact, when I was messing around with the bitrate knob, I was able to get the output buffer to climb up into the 4000s (!). Is there some look-ahead going on here that we don't know about? We're still just using Lame, and haven't even gotten into Blade at all.

The beginning is a bit weird:

```
lame0in: 512    enc: 336        dec: 0  outbuf: 1152
lame0 copy out 512
lame0in: 512    enc: 0  dec: 1152       outbuf: 1792
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
```

How are we able to decode on both of these times? On the second input, we only have 1024 samples, yet we return 1152 samples in that decode. Then on the third line, we put in 512 more samples, bringing the total to 1536. Yet at this point, we've decoded 2304 samples!

Here's how it looks when we switch from lame to blade:

```
lame0 init 48000 512 112
blade1 init 48000 512 8
lame0in: 512    enc: 336        dec: 0  outbuf: 1152
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 1152       outbuf: 1792
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1920
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2560
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 2048
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1536
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2176
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1664
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2304
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1792
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1920
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2560
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 2048
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1536
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2176
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1664
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2304
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1792
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1920
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2560
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 2048
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1536
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2176
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1664
blade1in: 512   dec: 4608        outbuf 2815
blade1 copy out 1151
lame0 copy out 512
blade1 copy out 512
blade1in: 512    outbuf 1152
blade1 copy out 512
blade1in: 512    outbuf 640
blade1 copy out 512
blade1in: 512   dec: 0   outbuf 128
can't copy out 512, some space will be blank
blade1 copy out 512
blade1in: 512    outbuf 0
can't copy out 512, some space will be blank
blade1 copy out 512
blade1in: 512   dec: 2304        outbuf 2304
blade1 copy out 512
blade1in: 512    outbuf 1792
blade1 copy out 512
blade1in: 512   dec: 0   outbuf 1280
blade1 copy out 512
blade1in: 512    outbuf 768
blade1 copy out 512
blade1in: 512   dec: 2304        outbuf 2560
blade1 copy out 512
blade1in: 512    outbuf 2048
blade1 copy out 512
blade1in: 512    outbuf 1536
blade1 copy out 512
blade1in: 512   dec: 0   outbuf 1024
blade1 copy out 512
blade1in: 512    outbuf 512
blade1 copy out 512
```

It looks like blade might be using some sort of bit reservoir. Digging into the code confirms this. Is there a way to turn off this reservoir?

In reservoir.c we have the line:

```c
	resvLimit = 4088; /* main_data_begin has 9 bits in MPEG 1 */
```

which sets a limit on the max size of the reservoir here:

```c
	/*
		determine maximum size of reservoir:
		flags->ResvMax + frameLength <= 7680;

		limit max size to resvLimit bits because
		main_data_begin cannot indicate a
		larger value
	*/
	flags->ResvMax = MIN(MAX (0, 7680-frameLength), resvLimit);
```

What if we set this to 0?

```c
	resvLimit = 0; /* main_data_begin has 9 bits in MPEG 1 */ // BEND
```

```
lame0 init 48000 512 112
blade1 init 48000 512 8
lame0in: 512    enc: 336        dec: 0  outbuf: 1152
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 1152       outbuf: 1792
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2432
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1920
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2560
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 2048
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1536
blade1in: 512   dec: 0   outbuf 0
lame0 copy out 512
lame0in: 512    enc: 336        dec: 1152       outbuf: 2176
blade1in: 512    outbuf 0
lame0 copy out 512
lame0in: 512    enc: 0  dec: 0  outbuf: 1664
blade1in: 512   dec: 4608        outbuf 2815
blade1 copy out 1151
lame0 copy out 512
blade1 copy out 512
blade1in: 512    outbuf 1152
blade1 copy out 512
blade1in: 512    outbuf 640
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1280
blade1 copy out 512
blade1in: 512    outbuf 768
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1408
blade1 copy out 512
blade1in: 512    outbuf 896
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1536
blade1 copy out 512
blade1in: 512    outbuf 1024
blade1 copy out 512
...
```

Wait huh? `blade1in: 512   dec: 4608        outbuf 2815`. 
Why are there so fewer elements in the output buffer as was decoded?
This is because in BladeController init, we do this:

```c++
    outputBufferL = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);
```

This doesn't actually matter, though: when our queue is full, it discards the oldest samples, while keeping the most recent ones, which is what we were about to do anyway. There's no need to have the extra space to store all of those.

Besides that, though, it seems sensible, and we're returning more often.

We do, however, still run into some issues with not being able to copy out. 
```
blade1in: 512   dec: 1152        outbuf 1408
blade1 copy out 512
blade0 init 48000 512 16
blade1in: 512    outbuf 896
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1536
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 1024
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade0 init 48000 512 24
blade1in: 512   dec: 1152        outbuf 1664
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 1152
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 640
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1280
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 768
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade0 init 48000 512 32
blade1in: 512   dec: 1152        outbuf 1408
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 896
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1536
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 1024
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1664
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade0 init 48000 512 40
blade1in: 512    outbuf 1152
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 640
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1280
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 768
blade0in: 512    outbuf 0
blade1 copy out 512
blade0 init 48000 512 48
blade1in: 512   dec: 1152        outbuf 1408
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 896
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1536
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 1024
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512   dec: 1152        outbuf 1664
blade0in: 512   dec: 0   outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 1152
blade0in: 512    outbuf 0
blade1 copy out 512
blade1in: 512    outbuf 640
blade0in: 512   dec: 3456        outbuf 2815
blade0 copy out 2175
blade1 copy out 512
blade0 copy out 512
blade1 init 48000 512 56
blade0in: 512    outbuf 128
blade1in: 512    outbuf 0
can't copy out 512, some space will be blank
blade0 copy out 512
blade0in: 512   dec: 1152        outbuf 1152
blade1in: 512    outbuf 0
blade0 copy out 512
blade0in: 512    outbuf 640
blade1in: 512   dec: 0   outbuf 0
blade0 copy out 512
blade0in: 512    outbuf 128
blade1in: 512    outbuf 0
can't copy out 512, some space will be blank
blade0 copy out 512
blade0in: 512   dec: 1152        outbuf 1152
blade1in: 512   dec: 0   outbuf 0
blade0 copy out 512
blade0in: 512    outbuf 640
blade1in: 512    outbuf 0
blade0 copy out 512
blade0in: 512   dec: 1152        outbuf 1280
blade1in: 512   dec: 3456        outbuf 2815
blade1 copy out 1535
...
```

Why is the blade0 outbuf able to get so small?

I think the problem is because we're losing the samples in the in-buffer. We trim the new output buffer to be the same size as the old output buffer; however, our buffered samples are shared between the input and output buffers.

Lame is a bit more complicated: instead of managing our own input buffer, there is an input buffer inside of Lame that we can't access. But what if we changed our use of Lame to be more like how we're using Blade, and only called the encode function when we had a multiple of 1152 samples? 

Thus, our MP3 controller logic would be:

```
encodeSamples(data) {
    inbuffer.enqueue(data)
    while (size of inbuffer > 1152) {
        frame = inbuffer.dequeue(1152)
        encoded = decode(encode(frame))
        outbuffer.enqueue(encoded)
    }
    return outbuffer.dequeue(data.numSamples)
}
```

Hold on a sec. We can do this even simpler.

## An even simpler solution?

What if the mp3 controller logic is:

```
init() {
    normal init stuff...
    decode(encode(1152 * N samples of silence))
}

processFrame(frame) {
    assert frame length == 1152
    result = decode(encode(frame))
    assert result length == 1152
    return result
}
```

and then the mp3 controller manager is like:

```
processSamples(samples) {
    inBuffer.enqueue(samples)
    while (inBuffer.length >= 1152) {
        inFrame = inBuffer.dequeue(1152)
        if (wantingToSwitch) {
            frameNew = newController.processFrame(inFrame)
            frameOld = oldController.processFrame(inFrame)
            outBuffer.enqueue(fade between the two frames)
            oldController = newController
            etc
        } else {
            outBuffer.enqueue(oldController.processFrame(inFrame))
        }
    }
    samples = outbuffer.dequeue(numsamples)
}
```

The one issue with this is that I believe this solution will create a little gap when switching to the new encoder, because of the encoding delay, and because we are flushing with silence. I see two solutions to this: we could keep another queue of the recent 1152 * N samples, and flush with those instead. Alternatively, we could have a countdown before switch, as we did in an earlier version.

The first option is a bit simpler to write, and is also a bit more immediate: we can switch between the old controller and the new in just one frame, rather than needing N frames to flush into it. The one downside, of course, is that we have to copy data to this buffer on every single frame, and we are not using it most of the time. This is especially bad since out queue implementation leaves something to be desired in terms of efficiency.

That said, I'm getting ahead of myself. This whole method might not actually work, and I'm already worrying about optimizing stuff.

### Trying it out

