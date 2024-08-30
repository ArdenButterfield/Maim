#include "common.h"
#if defined(__cplusplus)
extern "C" {
#endif

encoder_flags_and_data* blade_init(int samplerate, int bitrate);

int blade_get_chunk_size(encoder_flags_and_data* flags);

int blade_encode_chunk(encoder_flags_and_data* flags, float* left, float* right, char* output);

void blade_deinit(encoder_flags_and_data* flags);

void blade_set_butterfly_bends(encoder_flags_and_data* flags, float buinbu, float buinbd, float bdinbu, float bdinbd);
void blade_set_mdct_step_bends(encoder_flags_and_data* flags, int invert, int step);
void blade_set_mdct_post_shift_bends(encoder_flags_and_data* flags, int h_shift, float v_shift);
void blade_set_mdct_window_increment_bends(encoder_flags_and_data* flags, int window_inc);
void blade_set_mdct_band_reassignment_bends(encoder_flags_and_data* flags, int* band_reassignments);
void blade_set_mdct_feedback_bends(encoder_flags_and_data* flags, float feedback);
void blade_set_bitrate_squish_bends(encoder_flags_and_data* flags, float squish);
void blade_set_threshold_bias_bends(encoder_flags_and_data* flags, float bias);

float* blade_get_psychoanal_energy(encoder_flags_and_data* flags);
float* blade_get_psychoanal_threshold(encoder_flags_and_data* flags);
float* blade_get_mdct_pre_bend(encoder_flags_and_data* flags);
float* blade_get_mdct_post_bend(encoder_flags_and_data* flags);
int blade_is_short_block(encoder_flags_and_data* flags);

void blade_clear_bends(encoder_flags_and_data* flags);



#if defined(__cplusplus)
}
#endif

