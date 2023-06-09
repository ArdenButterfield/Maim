# MAIM

*MAIM Ain't an Implementation of Mp3*

MAIM is an audio plugin that circuit bends an MP3 encoder in real time, disrupting the control flow sending data down unexpected paths to create digital distortions. 

This plugin is still very much a work in progress, and is changing rapidly, so there are no guarantees that it works at any point in time. That said, feel free to give the code a try, and raise an issue or make a pull request if you have any suggestions!

## Building and Installation

To build MAIM, first build the LAME library. This step is necessary even if you have LAME installed on your machine, as MAIM uses a modified version of LAME. To do this, run the following commands:

```sh
cd Maim/lib/lame
./configure
make
```

The rest of the build process is standard for a JUCE plugin: open Maim.jucer in Projucer, set the paths to JUCE if necessary, click the button to open Maim in your IDE, then build all.

## Contributing

### Adding more bends

To add more bends to the encoder requires updating the API between the C++ frontend code and the LAME backend. The steps are as follows:

- Update the `bend_flags_and_data` struct in  `Maim/lib/lame/libmp3lame/lame_global_flags.h` to include the parameter that you need for your bend. The lame global flags and lame internal flags share an instance of this struct throughout the encoder's operation.

- Add a setter function to `Maim/lib/lame/include/lame.h` and `Maim/lib/lame/libmp3lame/lame.c` for the parameter.

- Add a line in `lame_clear_bends` in `Maim/lib/lame/libmp3lame/lame.c` if you need to do anything to set default values.

- Implement your bend, using whatever data you need from `bend_flags_and_data`. The relevant encoder and decoder functions are mostly in `Maim/lib/lame/libmp3lame`.
