#include "common.h"
#if defined(__cplusplus)
extern "C" {
#endif

encoder_flags_and_data* blade_init(int samplerate, int bitrate);

int blade_get_chunk_size(encoder_flags_and_data* flags);

int blade_encode_chunk(encoder_flags_and_data* flags, float* left, float* right, char* output);

void blade_deinit(encoder_flags_and_data* flags);
#if defined(__cplusplus)
}
#endif

