# siglent-bin2sr

Converts binary files exported from the *Siglent SDS-1000X-E* series oscilloscope into the Sigrok format.
This allows you to view the waveforms in [PulseView](https://sigrok.org/wiki/PulseView), which is free and open-source.

~~You can export the waveform in this format using the *Save/Recall* button to a thumbdrive if you are working at a remote location, or over LAN if you connect your scope to the network.
The binary format is the fastest and most convenient way to work wih the captured data because it is compact and fast to export, within 1-2 seconds and about 14 MB (1 byte per sample) in size.~~

~~If you _really_ want to, you can manually extract the data from offset `0x800` as unsigned bytes (128 as the middle zero point).
What this utility does is to also convert the values to the actual voltage values and time-scale from the metadata.
This saves you several steps from cutting the binary file to setting up the raw import in PulseView.~~

Siglent also provides a Windows tool from the scope's web UI to convert the binary file to a CSV file, but being a non-standard intermediate file is not helpful -- it is large (~300 MB!) and there are no tools to directly analyze, visualize or operate on it.

## Compile

TODO prerequisites (cmake, GCC, Windows support)

### Tests

TODO

Kaitai struct for debugging

## Installation

TODO cpack

~~Alternatively, pre-built binaries may be available under the [Releases](https://github.com/geekman/siglent-bin2sr/releases) section.~~

## Usage

    siglent-bin2sr [-10x | -v | -h] <input-file.bin> [output-folder]

where the options are:

- ~~`-raw` for a raw float32 file~~
- `-10x` to multiply values when used with a 10X probe. TODO make it per-channel
- `-v` to increase console logging verbosity

~~If the sample rate is too high (i.e. there are too many points), it can be reduced using `-decimate` option.
A decimation factor of 2 only counts 1 out of every 2 samples, effectively reducing the amount of samples by half.
Similarly a factor of 10 reduces the number of samples by 1/10th.~~

The output filename is fixed:

- ~~ `$FNAME-raw.bin` if a raw file is requested, or~~
- `input-file.srzip` otherwise


## Known Issues

- ~~There seems to be some slight voltage offset.
  I haven't yet determined whether this is due to my incorrect understanding of the binary file values, or the scope is exporting incorrect values.~~

- There's no indication when a 10X probe is used, so this needs to be applied manually using the `-10x` flag.

~~I did not want to invest too much time into reversing the format as it is not guaranteed to be "stable" by Siglent and may change in future.
The file format also does not contain any signatures (magic bytes) to uniquely identify it.

If you do know what's wrong or how to solve these problems, please let me know, or better yet, file a pull request on GitHub.
I wanted this tool to just be sufficiently usable.~~


## Specifications

* [Siglent binary file specs](https://siglentna.com/wp-content/uploads/dlm_uploads/2021/09/How-to-Extract-Data-from-the-Binary-File.pdf).

## License

**siglent-bin2sr is licensed under the 3-clause ("modified") BSD License.**

Copyright (C) 2022 giuliof
Copyright (C) 2020-2021 Darell Tan

TODO add license as file
