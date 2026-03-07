# Disney Sound Source Support

Some of the games supported by ReflectionHLE contained support for the Disney
Sound Source (DSS) as a playback device for digitized sounds aside from the
more standard Sound Blaster. Given that high level emulation of the Sound
Blaster already exists, the DSS code paths are entirely redundant. That is to
say that high level emulation of the DSS would sound identical to selecting to
emulate the original monaural Sound Blaster due to the games supported using
7kHz sampling rate for sounds anyway.

In the interest of doing *something* with those code paths, under some
operating systems ReflectionHLE retains support for talking to a real DSS
connected to a parallel port. There is no known practical reason why you would
want to do this, so this code path solely exists for educational and completion
purposes.

If adding a parallel port to a modern computer, do note that many USB to
parallel port adapters are soley for adapting printers. The parallel port must
have full support for SPP for the DSS to function.

To enable DSS support you must enable LPT passthrough and the low latency audio
mode, which must be done by editing your reflectionhle.cfg file. The low latency
audio mode requests that the audio service routine is called at a rate closer to
the 700Hz rate of original hardware. This is important since the DSS has a 16
sample buffer at 7000Hz +/- 5%, so timing is fairly precise.

Due to limitations on preemptive operating systems, combined with the
philosophical limitations on ReflectionHLE's design, audio playback may not be
able to properly keep pace and there may be stutters as a result. These
stutters would not necessarily be accurate to vanilla. Given this feature is of
limited practical utility, these limitations are just accepted.

## Linux

ReflectionHLE will use the ppdev parport driver. The DSS must be connected to
one of `/dev/parport0`, `/dev/parport1`, or `/dev/parport2` which are mapped
internally to LPT1 through LPT3. Ensure that your user is in the proper group to
have permission to access these devices.

## Windows

Working with the DSS requires low level access to the parallel port. To
accomplish this you must provide the DLL (inpout32.dll/inpoutx64.dll) for
[InpOut32/InpOutx64](https://www.highrez.co.uk/downloads/inpout32/) in your
ReflectionHLE install directory. The DSS must be installed on an LPT port using
the standard port addresses `378h`, `278h`, or `3BCh`. You will need to run
ReflectionHLE as an administrator for the driver to load.
