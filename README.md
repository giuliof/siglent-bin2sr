# siglent-bin2sr

This program converts binary files exported from the Siglent SDS-1000X-E series oscilloscope into the `.srzip` [Sigrok format](https://sigrok.org/wiki/File_format:Sigrok/v2).
`.srzip` files can be opened and analyzed with [PulseView](https://sigrok.org/wiki/PulseView).

This program is a C++ porting of [siglen-bin2sr](https://github.com/geekman/siglent-bin2sr), with the additional support logic probes decoding.

## Compile

`cmake`, `g++` and `libzip` must be installed to succesfully compile this program.

```
cmake -B build
cd build
make -j$(nproc)
```

## Usage

### Export data from the oscilloscope

Raw data can be exported by pressing the *Save/Recall* pushbutton, theb choosing *Binary* data format.

### Convert to .srzip

`./siglent-bin2sr [-o <folder>] <filename.bin>`

* `filename.bin` is the input file in Siglent binary format;
* `-o` is an optional argument, an output folder for the `.srzip` file may be provided.

## Known Issues

Siglent binary data does not provide any information regarding probe attenuation factor (x1, x10 and so on).
At the moment the data is not scaled, in the future may be added a way to correct the scaling factor.

## License

This project was initially a fork of [siglen-bin2sr](https://github.com/geekman/siglent-bin2sr), then moved to C++ due to my lack of experience in golang.
The license is unchanged (BSD-3-Clause).

Copyright (C) 2020-2021 Darell Tan

Copyright (C) 2022 giuliof