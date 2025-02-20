# Internal Junction Temperature read for Daisy Seed using ADC3 

This code allows to enable the ADC3 on daisySeed to read the internal channels of:
-  **V_refint**: the internal reference used by all ADCs channel to compare voltages
- **T_J**: the Junction Temperature (internal temperature of CPU)

For now measurement works only in polling mode (DMA seems not working in the right way)

## requirements

- DaisySeed board
- VSCode with [Daisy Toolchain](https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment) installed
- Serial Monitor