#include "blade.h"
#include "codec.h"

#include <stdlib.h>
#include <math.h>

encoder_flags_and_data* blade_init(int samplerate, int bitrate)
{
	encoder_flags_and_data* flags = calloc(1, sizeof(encoder_flags_and_data));
	if (flags == NULL) {
		return flags;
	}

	flags->codec_data.frequency = samplerate;
	flags->codec_data.mode = 0; // Stereo
	flags->codec_data.bitrate = bitrate;
	flags->codec_data.emphasis = 0;
	flags->codec_data.fPrivate = 0;
	flags->codec_data.fCRC = 0;
	flags->codec_data.fCopyright = 0;
	flags->codec_data.fOriginal = 1;

	codecInit(flags, &(flags->codec_data)); 
	// we don't need the return value, because its already stored in flags.

	return flags;
}

int blade_get_chunk_size(encoder_flags_and_data* flags)
{
	return 1152;
}

static short convert_sample(float s)
{
	// digital clip
	if (s <= -1) {
		return -32768;
	}
	if (s >= 1) {
		return 32767;
	}
	return (short)(s * (1 << 15));
#if 0

	// triangle rollover
	s = fabsf(fmodf(s - 1.f, 4.f)-2.f)-1;
	return (short)(s * (1 << 15));
#endif
}

int blade_encode_chunk(encoder_flags_and_data* flags, float* left, float* right, char* output)
{
	short readBuffer[2304];
	for (int i = 0; i < 1152; ++i) {
		readBuffer[i * 2] = convert_sample(left[i]);
		readBuffer[i * 2 + 1] = convert_sample(right[i]);
	}
	return codecEncodeChunk (flags, 2304, readBuffer, output);
}

void blade_deinit(encoder_flags_and_data* flags)
{
	free(flags);
}


// Bends

void blade_set_butterfly_bends(encoder_flags_and_data* flags, float buinbu, float buinbd, float bdinbu, float bdinbd)
{
	flags->bends.butterfly_bubu = buinbu;
	flags->bends.butterfly_bubd = buinbd;
	flags->bends.butterfly_bdbu = bdinbu;
	flags->bends.butterfly_bdbd = bdinbd;

}

void blade_set_mdct_step_bends(encoder_flags_and_data* flags, int invert, int step)
{

}

void blade_set_mdct_post_shift_bends(encoder_flags_and_data* flags, int h_shift, float v_shift)
{
	flags->bends.mdct_post_v_shift = v_shift;
	flags->bends.mdct_post_h_shift = h_shift;
}

void blade_set_mdct_window_increment_bends(encoder_flags_and_data* flags, int window_inc)
{
    flags->bends.mdct_window_increment = window_inc;
    flags->bends.mdct_samp_increment = 0;

}

void blade_set_mdct_band_reassignment_bends(encoder_flags_and_data* flags, int* band_reassignments)
{
    for (int i = 0; i < 32; ++i) {
        flags->bends.mdct_band_reassignments[i] = band_reassignments[i];
    }
}

void blade_set_mdct_feedback_bends(encoder_flags_and_data* flags, float feedback)
{
	flags->bends.mdct_feedback = feedback;
}

void blade_set_bitrate_squish_bends(encoder_flags_and_data* flags, float squish)
{
	flags->bends.bitrate_squish = squish;
}

void blade_set_threshold_bias_bends(encoder_flags_and_data* flags, float bias)
{
	float b;
    for (int i = 0; i < 22; ++i) {
        b = pow(10.f, (-bias) * (i - 11.f) / 1.f);

        if (bias < 0) {
            b *= pow(1000000.f, -bias);
        }
        flags->bends.threshold_bias[i] = b;
    }	
}

float* blade_get_psychoanal_energy(encoder_flags_and_data* flags)
{
	return flags->bends.psychoanal_energy;
}

float* blade_get_psychoanal_threshold(encoder_flags_and_data* flags)
{
	return flags->bends.psychoanal_threshold;
}

float* blade_get_mdct_pre_bend(encoder_flags_and_data* flags)
{
    return flags->bends.mdct_pre_bend;
}

float* blade_get_mdct_post_bend(encoder_flags_and_data* flags)
{
    return flags->bends.mdct_post_bend;
}


int blade_is_short_block(encoder_flags_and_data* flags)
{
	return flags->bends.in_short_block;
}

void blade_clear_bends(encoder_flags_and_data* flags)
{
	flags->bends.butterfly_bubu = 1;
    flags->bends.butterfly_bubd = 0;
    flags->bends.butterfly_bdbu = 0;
    flags->bends.butterfly_bdbd = 1;

    flags->bends.mdct_invert = 0;
    flags->bends.mdct_band_step = 18;

    flags->bends.mdct_post_h_shift = 0;
    flags->bends.mdct_post_v_shift = 0.f;

    flags->bends.mdct_window_increment = 64;
    flags->bends.mdct_samp_increment = 64;

    flags->bends.bitrate_squish = 1;

    flags->bends.mdct_feedback = 0;
    flags->bends.prev_block_long = 0;

    for (int i = 0; i < 32; ++i) {
        flags->bends.mdct_band_reassignments[i] = i;
    }

    for (int i = 0; i < 22; ++i) {
        flags->bends.psychoanal_energy[i] = 0;
        flags->bends.psychoanal_threshold[i] = 0;
    }
    memset(flags->bends.mdct_pre_bend, 0, 576 * sizeof(float));
    memset(flags->bends.mdct_post_bend, 0, 576 * sizeof(float));

    for (int i = 0; i < 22; ++i) {
        flags->bends.threshold_bias[i] = 1;
    }
    flags->bends.freq_gate = 0;

    memset(flags->bends.feedback_data, 0, 2 * 2 * 576 * sizeof(float));
}




