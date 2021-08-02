# STM32L4R9I Application Template

## Display

The Excel Sheet `GFXMMU_LUT_Configuration.xlsx` illustrates the start and end column for each row of the display.

|Row|Start Pixel|End Pixel|
|-|-|-|
|0|175|214|
|1|167|222|
|2|160|229|
|3|155|234|
|4|151|238|
|5|146|243|
|6|143|246|
|7|139|250|
|8|136|253|
|...|...|...|


## Known Issues

### STM32CubeMX 6.0.x

When opening an IOC file in STM32CubeMX 6.0.x, it is mandatory to open up the _LUT Configuration_ inside the Multimedia/GFXMMU settings first in order to generate a correct display mapping. 

Due to a bug in STM32CubeMX the _LUT Configuration_ tab does not show up immediately. 
As a workaround, open the IOC file in STM32CubeMX, go to _Multimedia_ in _Pinout & Configuration_, go to _GFXMMU_, go to _LTDC_, go back to _GFXMMU_. Now the _LUT Configuration_ tab will show up.

If the LUT table is not initialised before generating a project for STM32CubeIDE, Keil or IAR, all LUT settings will be lost and the display will only show noise.

To be sure valid LUT settings are used in your project, check the file `Core/Inc/gfxmmu_lut.h`.

```cpp
uint32_t gfxmmu_lut_config[2*GFXMMU_LUT_SIZE] = {
  0x001A1501, //GFXMMU_LUT0L
  0x003FFEB0, //GFXMMU_LUT0H
  0x001B1401, //GFXMMU_LUT1L
  0x003FFF20, //GFXMMU_LUT1H
  0x001C1401, //GFXMMU_LUT2L
  0x003FFFA0, //GFXMMU_LUT2H
  0x001D1301, //GFXMMU_LUT3L
  0x00000040, //GFXMMU_LUT3H
  ...
```


