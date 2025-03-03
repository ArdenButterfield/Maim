Where does the meat of the lame decoder lie?

in `mpglib_interface.c`, we call 
```c
decode1_headersB_clipchoice(PMPSTR pmp, unsigned char *buffer, int len,
                            char pcm_l_raw[], char pcm_r_raw[], mp3data_struct * mp3data,
                            int *enc_delay, int *enc_padding,
                            char *p, size_t psize, int decoded_sample_size,
                            int (*decodeMP3_ptr) (PMPSTR, unsigned char *, int, char *, int,
                            int *))
```

and the real parsing happens in the `decodeMP3_ptr` function. But what is that function really?

It gets passed here:

```c
int
hip_decode1_unclipped(hip_t hip, unsigned char *buffer, size_t len, sample_t pcm_l[], sample_t pcm_r[])
{
    char out[OUTSIZE_UNCLIPPED];
    mp3data_struct mp3data;
    int     enc_delay, enc_padding;

    if (hip) {
        return decode1_headersB_clipchoice(hip, buffer, len, (char *) pcm_l, (char *) pcm_r, &mp3data,
                                           &enc_delay, &enc_padding, out, OUTSIZE_UNCLIPPED,
                                           sizeof(FLOAT), decodeMP3_unclipped);
    }
    return 0;
}
```

In `interface.c`, we have:

```c
int
decodeMP3_unclipped(PMPSTR mp, unsigned char *in, int isize, char *out, int osize, int *done)
{
    /* we forbid input with more than 1152 samples per channel for output in unclipped mode */
    if (osize < (int) (1152 * 2 * sizeof(real))) {
        lame_report_fnc(mp->report_err, "hip: out space too small for unclipped mode\n");
        return MP3_ERR;
    }

    /* passing pointers to the functions which don't clip the samples */
    return decodeMP3_clipchoice(mp, in, isize, out, done, synth_1to1_mono_unclipped,
                                synth_1to1_unclipped);
}
```

And `decodeMP3_clipchoice` is defined in that same file. This seems to be where the magic actually happens?

confirmed that we are using layer 3.

so `decode_layer3_frame`, called on line 615, is the key function here.

It is defined, appropriately enough in `layer3.c`.

We can mess with the scalefactors -- sfbits -- to get some crunchiness. But ultimately I think we need to get into the 
`III_dequantize_sample` function to really introduce errors.

Ultimately, though, I think i'll pause here and 