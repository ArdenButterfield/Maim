#include "blade.h"
#include "codec.h"

#include <stdlib.h>

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

int blade_encode_chunk(encoder_flags_and_data* flags, float* left, float* right, char* output)
{
	short readBuffer[2304];
	for (int i = 0; i < 1152; ++i) {
		readBuffer[i * 2] = (short)(left[i] * (1 << 15));
		readBuffer[i * 2 + 1] = (short)(right[i] * (1 << 15));
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

}

void blade_set_mdct_band_reassignment_bends(encoder_flags_and_data* flags, int* band_reassignments)
{

}

void blade_set_mdct_feedback_bends(encoder_flags_and_data* flags, float feedback)
{

}

void blade_set_bitrate_squish_bends(encoder_flags_and_data* flags, float squish)
{

}

void blade_set_threshold_bias_bends(encoder_flags_and_data* flags, float bias)
{

}

float* blade_get_psychoanal_energy(encoder_flags_and_data* flags)
{
	return NULL;
}

float* blade_get_psychoanal_threshold(encoder_flags_and_data* flags)
{
	return NULL;
}

int blade_is_short_block(encoder_flags_and_data* flags)
{
	return 0;
}

void blade_clear_bends(encoder_flags_and_data* flags)
{
	flags->bends.mdct_post_v_shift = 0;
	flags->bends.mdct_post_h_shift = 0;
}




