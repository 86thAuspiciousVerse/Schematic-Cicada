# Medium-density performance line Arm<sup>®</sup>-based 32-bit MCU with 64 or 128 KB Flash, USB, CAN, 7 timers, 2 ADCs, 9 com. interfaces

## Datasheet - production data

## Features

## Includes ST state-of-the-art patented technology

• Arm<sup>®</sup> 32-bit Cortex<sup>®</sup>-M3 CPU core

72 MHz maximum frequency, 1.25 DMIPS/MHz (Dhrystone 2.1) performance at 0 wait state memory access

Single-cycle multiplication and hardware division

## • Memories

– 64 or 128 Kbytes of Flash memory

– 20 Kbytes of SRAM

• Clock, reset and supply management

– 2.0 to 3.6 V application supply and I/Os

– POR, PDR, and programmable voltage detector (PVD)

– 4 to 16 MHz crystal oscillator

– Internal 8 MHz factory-trimmed RC

– Internal 40 kHz RC

– PLL for CPU clock

– 32 kHz oscillator for RTC with calibration

## • Low-power

– Sleep, Stop and Standby modes

– V<sub>BAT</sub> supply for RTC and backup registers

• 2x 12-bit, 1 µs A/D converters (up to 16 channels)

– Conversion range: 0 to 3.6 V

– Dual-sample and hold capability

– Temperature sensor

## DMA

– 7-channel DMA controller

Peripherals supported: timers, ADC, SPIs, I<sup>2</sup>Cs and USARTs

## • Up to 80 fast I/O ports

26/37/51/80 I/Os, all mappable on 16 external interrupt vectors and almost all 5 V-tolerant

![](images/c8e80439ed3773623a046b56937f365b6226b033785889c70e3fef4eabf5a837.jpg)

## • Debug mode:

Serial wire debug (SWD) and JTAG interfaces

## • Seven timers

Three 16-bit timers, each with up to 4 IC/OC/PWM or pulse counter and quadrature (incremental) encoder input

– 16-bit, motor control PWM timer with dead-time generation and emergency stop

– Two watchdog timers (independent and window)

– SysTick timer 24-bit downcounter

• Up to nine communication interfaces

– Up to two I<sup>2</sup>C interfaces (SMBus/PMBus<sup>®</sup>)

– Up to three USARTs (ISO 7816 interface, LIN, IrDA capability, modem control)

– Up to two SPIs (18 Mbit/s)

– CAN interface (2.0B Active)

– USB 2.0 full-speed interface

• CRC calculation unit, 96-bit unique ID

• Packages are ECOPACK<sup>®</sup>

Table 1. Device summary

<table><tr><td>Reference</td><td>Part number</td></tr><tr><td>STM32F103x8</td><td>STM32F103C8, STM32F103R8STM32F103V8, STM32F103T8</td></tr><tr><td>STM32F103xB</td><td>STM32F103RB STM32F103VB,STM32F103CB, STM32F103TB</td></tr></table>

1 Introduction 9
2 Description 9
2.1 Device overview 10
2.2 Full compatibility throughout the family 13
2.3 Overview 14
2.3.1 Arm® Cortex®-M3 core with embedded flash and SRAM 14
2.3.2 Embedded flash memory 14
2.3.3 CRC (cyclic redundancy check) calculation unit 14
2.3.4 Embedded SRAM 14
2.3.5 Nested vectored interrupt controller (NVIC) 14
2.3.6 External interrupt/event controller (EXTI) 15
2.3.7 Clocks and startup 15
2.3.8 Boot modes 15
2.3.9 Power supply schemes 15
2.3.10 Power supply supervisor 15
2.3.11 Voltage regulator 16
2.3.12 Low-power modes 16
2.3.13 DMA 17
2.3.14 RTC (real-time clock) and backup registers 17
2.3.15 Timers and watchdogs 17
2.3.16 I²C bus 19
2.3.17 Universal synchronous/asynchronous receiver transmitter (USART) 19
2.3.18 Serial peripheral interface (SPI) 19
2.3.19 Controller area network (CAN) 19
2.3.20 Universal serial bus (USB) 19
2.3.21 GPIOs (general-purpose inputs/outputs) 20
2.3.22 ADC (analog-to-digital converter) 20
2.3.23 Temperature sensor 20
2.3.24 Serial wire JTAG debug port (SWJ-DP) 20
3 Pinouts and pin description 21
4 Memory mapping 34
5 Electrical characteristics 35
5.1 Parameter conditions 35
2/115 DS5319 Rev 20

5.1.1 Minimum and maximum values 35
5.1.2 Typical values 35
5.1.3 Typical curves 35
5.1.4 Loading capacitor 35
5.1.5 Pin input voltage 35
5.1.6 Power supply scheme 36
5.1.7 Current consumption measurement 36
5.2 Absolute maximum ratings 37
5.3 Operating conditions 38
5.3.1 General operating conditions 38
5.3.2 Operating conditions at power-up / power-down 39
5.3.3 Embedded reset and power control block characteristics 39
5.3.4 Embedded reference voltage 40
5.3.5 Supply current characteristics 40
5.3.6 External clock source characteristics 50
5.3.7 Internal clock source characteristics 54
5.3.8 PLL characteristics 56
5.3.9 Memory characteristics 56
5.3.10 EMC characteristics 57
5.3.11 Absolute maximum ratings (electrical sensitivity) 59
5.3.12 I/O current injection characteristics 60
5.3.13 I/O port characteristics 61
5.3.14 NRST pin characteristics 66
5.3.15 TIM timer characteristics 67
5.3.16 Communications interfaces 68
5.3.17 CAN (controller area network) interface 73
5.3.18 12-bit ADC characteristics 74
5.3.19 Temperature sensor characteristics 78

Package information 79
6.1 Device marking 79
6.2 VFQFPN36 package information (ZR) 80
6.3 UFQFPN48 package information (A0B9) 83
6.4 LFBGA100 package information (H0) 85
6.5 LQFP100 package information (1L) 88
6.6 UFBGA100 package information (A0C2) 91

DS5319 Rev 20 3/115

6.7 LQFP64 package information (5W) 94  
6.8 TFBGA64 package information (R8) 97  
6.9 LQFP48 package information (5B) 99  
6.10 Thermal characteristics 102  
6.10.1 Reference document 102  
6.10.2 Selecting the product temperature range 103  
7 Ordering information scheme 105  
8 Important security notice 106  
9 Revision history 107

## List of tables

Table 1. Device summary ..... 1
Table 2. STM32F103xx medium-density device features and peripheral counts ..... 10
Table 3. STM32F103xx family ..... 13
Table 4. Timer feature comparison ..... 17
Table 5. Medium-density STM32F103xx pin definitions ..... 28
Table 6. Voltage characteristics ..... 37
Table 7. Current characteristics ..... 37
Table 8. Thermal characteristics ..... 37
Table 9. General operating conditions ..... 38
Table 10. Operating conditions at power-up / power-down ..... 39
Table 11. Embedded reset and power control block characteristics ..... 39
Table 12. Embedded internal reference voltage ..... 40
Table 13. Maximum current consumption in Run mode, code with data processing running from Flash ..... 41
Table 14. Maximum current consumption in Run mode, code with data processing running from RAM ..... 41
Table 15. Maximum current consumption in Sleep mode, code running from Flash or RAM ..... 43
Table 16. Typical and maximum current consumptions in Stop and Standby modes ..... 44
Table 17. Typical current consumption in Run mode, code with data processing running from Flash ..... 47
Table 18. Typical current consumption in Sleep mode, code running from Flash or RAM ..... 48
Table 19. Peripheral current consumption ..... 49
Table 20. High-speed external user clock characteristics ..... 50
Table 21. Low-speed external user clock characteristics ..... 51
Table 22. HSE 4-16 MHz oscillator characteristics ..... 52
Table 23. LSE oscillator characteristics (fLSE = 32.768 kHz) ..... 53
Table 24. HSI oscillator characteristics ..... 54
Table 25. LSI oscillator characteristics ..... 55
Table 26. Low-power mode wakeup timings ..... 56
Table 27. PLL characteristics ..... 56
Table 28. Flash memory characteristics ..... 56
Table 29. Flash memory endurance and data retention ..... 57
Table 30. EMS characteristics ..... 58
Table 31. EMI characteristics for fHSE = 8 MHz and fHCLK = 48 MHz ..... 58
Table 32. EMI characteristics for fHSE = 8 MHz and fHCLK = 72 MHz ..... 59
Table 33. ESD absolute maximum ratings ..... 59
Table 34. Electrical sensitivities ..... 59
Table 35. I/O current injection susceptibility ..... 60
Table 36. I/O static characteristics ..... 61
Table 37. Output voltage characteristics ..... 64
Table 38. I/O AC characteristics ..... 65
Table 39. NRST pin characteristics ..... 66
Table 40. TIMx characteristics ..... 67
Table 41. I²C characteristics ..... 68
Table 42. SCL frequency (fPCLK1 = 36 MHz, VDD\_I2C = 3.3 V) ..... 69
Table 43. SPI characteristics ..... 70

Table 44. USB startup time. 72
Table 45. USB DC electrical characteristics. 73
Table 46. USB: Full-speed electrical characteristics. 73
Table 47. ADC characteristics. 74
Table 48. R $_{AIN}$ max for f $_{ADC}$ = 14 MHz. 75
Table 49. ADC accuracy - Limited test conditions. 75
Table 50. ADC accuracy. 76
Table 51. TS characteristics. 78
Table 52. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package mechanical data. 81
Table 53. UFQFPN48 – Mechanical data. 84
Table 54. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package mechanical data. 86
Table 55. LFBGA100 recommended PCB design rules (0.8 mm pitch BGA). 87
Table 56. LQFP100 - Mechanical data. 89
Table 57. UFBGA100 - Mechanical data. 92
Table 58. UFBGA100 - Example of PCB design rules (0.5 mm pitch BGA). 93
Table 59. LQFP64 - Mechanical data. 95
Table 60. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch, thin profile fine pitch ball grid array package mechanical data. 97
Table 61. TFBGA64 recommended PCB design rules (0.5 mm pitch BGA). 98
Table 62. LQFP48 – Mechanical data. 100
Table 63. Package thermal characteristics. 102
Table 64. Document revision history. 107

## List of figures

Figure 1. STM32F103xx performance line block diagram ..... 11
Figure 2. Clock tree ..... 12
Figure 3. STM32F103xx performance line LFBGA100 ballout ..... 21
Figure 4. STM32F103xx performance line LQFP100 pinout ..... 22
Figure 5. STM32F103xx performance line UFBGA100 pinout ..... 23
Figure 6. STM32F103xx performance line LQFP64 pinout ..... 24
Figure 7. STM32F103xx performance line TFBGA64 ballout ..... 25
Figure 8. STM32F103xx performance line LQFP48 pinout ..... 26
Figure 9. STM32F103xx performance line UFQFPN48 pinout ..... 26
Figure 10. STM32F103xx performance line VFQFPN36 pinout ..... 27
Figure 11. Memory map ..... 34
Figure 12. Pin loading conditions ..... 35
Figure 13. Pin input voltage ..... 35
Figure 14. Power supply scheme ..... 36
Figure 15. Current consumption measurement scheme ..... 36
Figure 16. Typical current consumption in Run mode versus frequency (at 3.6 V), code with data processing running from RAM, peripherals enabled ..... 42
Figure 17. Typical current consumption in Run mode versus frequency (at 3.6 V), code with data processing running from RAM, peripherals disabled ..... 42
Figure 18. Typical current consumption on $V_{BAT}$ (RTC on) ..... 44
Figure 19. Typical current consumption in Stop mode, with regulator in Run mode ..... 45
Figure 20. Typical current consumption in Stop mode, with regulator in Low-power mode ..... 45
Figure 21. Typical current consumption in Standby mode ..... 46
Figure 22. High-speed external clock source AC timing diagram ..... 51
Figure 23. Low-speed external clock source AC timing diagram ..... 52
Figure 24. Typical application with an 8 MHz crystal ..... 53
Figure 25. Typical application with a 32.768 kHz crystal ..... 54
Figure 26. Standard I/O input characteristics - CMOS port ..... 62
Figure 27. Standard I/O input characteristics - TTL port ..... 62
Figure 28. 5 V tolerant I/O input characteristics - CMOS port ..... 63
Figure 29. 5 V tolerant I/O input characteristics - TTL port ..... 63
Figure 30. I/O AC characteristics definition ..... 66
Figure 31. Recommended NRST pin protection ..... 67
Figure 32. I²C bus AC waveforms and measurement circuit ..... 69
Figure 33. SPI timing diagram - slave mode and CPHA = 0 ..... 71
Figure 34. SPI timing diagram - slave mode and CPHA = 1 ..... 71
Figure 35. SPI timing diagram - master mode ..... 72
Figure 36. USB timings: definition of data signal rise and fall time ..... 73
Figure 37. ADC accuracy characteristics ..... 76
Figure 38. Typical connection diagram using the ADC ..... 77
Figure 39. Power supply and reference decoupling ( $V_{REF+}$ not connected to $V_{DDA}$ ). ..... 77
Figure 40. Power supply and reference decoupling ( $V_{REF+}$ connected to $V_{DDA}$ ). ..... 78
Figure 41. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package outline ..... 80
Figure 42. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package recommended footprint ..... 82
Figure 43. UFQFPN48 – Outline ..... 83
Figure 44. UFQFPN48 – Footprint example ..... 85

Figure 45. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package outline ..... 85
Figure 46. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package recommended footprint ..... 86
Figure 47. LQFP100 - Outline $^{(15)}$ ..... 88
Figure 48. LQFP100 - Footprint example ..... 90
Figure 49. UFBGA100 - Outline $^{(13)}$ ..... 91
Figure 50. UFBGA100 - Footprint example ..... 93
Figure 51. LQFP64 - Outline $^{(15)}$ ..... 94
Figure 52. LQFP64 - Footprint example ..... 96
Figure 53. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch thin profile fine pitch ball grid array package outline ..... 97
Figure 54. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch, thin profile fine pitch ball grid array , recommended footprint ..... 98
Figure 55. LQFP48 – Outline $^{(15)}$ ..... 99
Figure 56. LQFP48 – Footprint example ..... 101
Figure 57. LQFP100 P $_{D}$ max vs. T $_{A}$ ..... 104

## Introduction

This document provides the ordering information and mechanical device characteristics of the STM32F103x8 and STM32F103xB medium-density performance line microcontrollers. For more details on the whole STMicroelectronics STM32F103xx family, refer to Section 2.2: Full compatibility throughout the family.

The medium-density STM32F103xx datasheet must be read in conjunction with the low-, medium-, and high-density STM32F10xxx reference manual. For information on the device errata with respect to the datasheet and reference manual, refer to the STM32F103x8/B errata sheet (ES096). The errata sheet, reference manual, and flash programming manual are all available on the STMicroelectronics website www.st.com.

For information on the Arm<sup>®(a)</sup> Cortex<sup>®</sup>-M3 core refer to the Cortex<sup>®</sup>-M3 Technical Reference Manual, available from the www.arm.com website.

## 2 Description

The STM32F103xx medium-density performance line family incorporates the high-performance Arm<sup>®</sup> Cortex<sup>®</sup>-M3 32-bit RISC core operating at a 72 MHz frequency, high-speed embedded memories (Flash memory up to 128 Kbytes and SRAM up to 20 Kbytes), and an extensive range of enhanced I/Os and peripherals connected to two APB buses. All devices offer two 12-bit ADCs, three general purpose 16-bit timers plus one PWM timer, as well as standard and advanced communication interfaces: up to two I<sup>2</sup>Cs and SPIs, three USARTs, an USB and a CAN.

The devices operate from a 2.0 to 3.6 V power supply. They are available in both the –40 to +85°C temperature range and the –40 to +105 °C extended temperature range. A comprehensive set of power-saving mode allows the design of low-power applications.

The STM32F103xx medium-density performance line family includes devices in six different package types: from 36 pins to 100 pins. Depending on the device chosen, different sets of peripherals are included, the description below gives an overview of the complete range of peripherals proposed in this family.

These features make the STM32F103xx medium-density performance line microcontroller family suitable for a wide range of applications such as motor drives, application control, medical and handheld equipment, PC and gaming peripherals, GPS platforms, industrial applications, PLCs, inverters, printers, scanners, alarm systems, video intercoms, and HVACs.

![](images/c818847eb835d4cd8213815425db78a7a05ac6f84a0c2f707e80b853a75d3462.jpg)

## 2.1 Device overview

Table 2. STM32F103xx medium-density device features and peripheral counts

<table><tr><td colspan="2">Peripheral</td><td colspan="2">STM32F103Tx</td><td colspan="2">STM32F103Cx</td><td colspan="2">STM32F103Rx</td><td colspan="2">STM32F103Vx</td></tr><tr><td colspan="2">Flash - Kbytes</td><td>64</td><td>128</td><td>64</td><td>128</td><td>64</td><td>128</td><td>64</td><td>128</td></tr><tr><td colspan="2">SRAM - Kbytes</td><td colspan="2">20</td><td colspan="2">20</td><td colspan="2">20</td><td colspan="2">20</td></tr><tr><td rowspan="2">Timers</td><td>General-purpose</td><td colspan="2">3</td><td colspan="2">3</td><td colspan="2">3</td><td colspan="2">3</td></tr><tr><td>Advanced-control</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td></tr><tr><td rowspan="5">Communication</td><td>SPI</td><td colspan="2">1</td><td colspan="2">2</td><td colspan="2">2</td><td colspan="2">2</td></tr><tr><td> $I^{2}C$ </td><td colspan="2">1</td><td colspan="2">2</td><td colspan="2">2</td><td colspan="2">2</td></tr><tr><td>USART</td><td colspan="2">2</td><td colspan="2">3</td><td colspan="2">3</td><td colspan="2">3</td></tr><tr><td>USB</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td></tr><tr><td>CAN</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td><td colspan="2">1</td></tr><tr><td colspan="2">GPIOs</td><td colspan="2">26</td><td colspan="2">37</td><td colspan="2">51</td><td colspan="2">80</td></tr><tr><td colspan="2">12-bit synchronized ADC Number of channels</td><td colspan="2">2 10 channels</td><td colspan="2">2 10 channels</td><td colspan="2">2 16  $channels^{(1)}$ </td><td colspan="2">2 16 channels</td></tr><tr><td colspan="2">CPU frequency</td><td colspan="8">72 MHz</td></tr><tr><td colspan="2">Operating voltage</td><td colspan="8">2.0 to 3.6 V</td></tr><tr><td colspan="2">Operating temperatures</td><td colspan="8">Ambient temperatures: -40 to +85 °C / -40 to +105 °C (see Table 9)Junction temperature: -40 to + 125 °C (see Table 9)</td></tr><tr><td colspan="2">Packages</td><td colspan="2">VFQFPN36</td><td colspan="2">LQFP48, UFQFPN48</td><td colspan="2">LQFP64, TFBGA64</td><td colspan="2">LQFP100, LFBGA100, UFBGA100</td></tr></table>

1. On the TFBGA64 package only 15 channels are available (one analog input pin has been replaced by V<sub>REF+</sub>).

Figure 1. STM32F103xx performance line block diagram  
![](images/b18f86d05b2038c797933d546be3a7d351bb89adf4d2c5f1de6dda34a192af8e.jpg)  
1. $\mathsf { T } _ { \mathsf { A } } = - 4 0 ^ { \circ } \mathsf { C }$ to $+ 1 0 5 ^ { \circ } \mathrm { C }$ (junction temperature up to $1 2 5 ^ { \circ } \mathrm { C } )$ .  
2. AF = alternate function on I/O port pin.

Figure 2. Clock tree  
![](images/3ada8f8bfb8770209885f6994a55a3aa4d1800de75e9a4de36299d184b541afe.jpg)  
1. When the HSI is used as a PLL clock input, the maximum system clock frequency that can be achieved is 64 MHz.

2. For the availability of the USB function both HSE and PLL must be enabled, with USBCLK running at 48 MHz.

3. To have an ADC conversion time of 1 µs, APB2 must be at 14 MHz, 28 MHz, or 56 MHz.

## 2.2 Full compatibility throughout the family

STM32F103xx is a complete family whose members are fully pin-to-pin, software, and feature compatible. In the reference manual, STM32F103x4 and STM32F103x6 are identified as low-density devices, STM32F103x8 and STM32F103xB are referred to as medium-density devices, and STM32F103xC, STM32F103xD, and STM32F103xE are referred to as high-density devices.

Low- and high-density devices are an extension of the STM32F103x8/B devices; they are specified in the STM32F103x4/6 and STM32F103xC/D/E datasheets, respectively. Low-density devices feature lower flash memory and RAM capacities, and fewer timers and peripherals. High-density devices have higher flash memory and RAM capacities, and additional peripherals like SDIO, FSMC, I<sup>2</sup>S, and DAC, while remaining fully compatible with the other members of the STM32F103xx family.

The STM32F103x4, STM32F103x6, STM32F103xC, STM32F103xD and STM32F103xE are a drop-in replacement for STM32F103x8/B medium-density devices, allowing the user to try different memory densities and providing a greater degree of freedom during the development cycle.

Moreover, the STM32F103xx performance line family is fully compatible with all existing STM32F101xx access line and STM32F102xx USB access line devices.

Table 3. STM32F103xx family

<table><tr><td rowspan="3">Pinout</td><td colspan="2">Low-density devices</td><td colspan="2">Medium-density devices</td><td colspan="3">High-density devices</td></tr><tr><td>16 KB Flash</td><td>32 KB Flash</td><td>64 KB Flash</td><td>128 KB Flash</td><td>256 KB Flash</td><td>384 KB Flash</td><td>512 KB Flash</td></tr><tr><td>6 KB RAM</td><td>10 KB RAM</td><td>20 KB RAM</td><td>20 KB RAM</td><td>48 KB RAM</td><td>64 KB RAM</td><td>64 KB RAM</td></tr><tr><td>144</td><td>-</td><td>-</td><td>-</td><td>-</td><td rowspan="3" colspan="3">5× USARTs4× 16-bit timers, 2× basic timers3× SPIs, 2×  $I^{2}Ss$ , 2× I2CsUSB, CAN, 2× PWM timers3× ADCs, 2× DACs, 1× SDIO FSMC (100 and 144 pins)</td></tr><tr><td>100</td><td>-</td><td>-</td><td rowspan="4" colspan="2">3× USARTs3× 16-bit timers2× SPIs, 2×  $I^{2}Cs$ , USB, CAN, 1× PWM timer2× ADCs</td></tr><tr><td>64</td><td rowspan="3" colspan="2">2× USARTs2× 16-bit timers1× SPI, 1×  $I^{2}C$ , USB, CAN, 1× PWM timer2× ADCs</td></tr><tr><td>48</td><td>-</td><td>-</td><td>-</td></tr><tr><td>36</td><td>-</td><td>-</td><td>-</td></tr></table>

## 2.3 Overview

## 2.3.1 $\mathsf { \pmb { A r m } } ^ { \odot }$ Cortex<sup>®</sup>-M3 core with embedded flash and SRAM

The $\mathsf { A r m } ^ { \circledast }$ Cortex<sup>®</sup>-M3 processor is the latest generation of $\mathsf { A r m } ^ { \circledast }$ processors for embedded systems. It has been developed to provide a low-cost platform that meets the needs of MCU implementation, with a reduced pin count and low-power consumption, while delivering outstanding computational performance and an advanced system response to interrupts.

The Arm<sup>®</sup> Cortex<sup>®</sup>-M3 32-bit RISC processor features exceptional code-efficiency, delivering the high-performance expected from an Arm<sup>®</sup> core in the memory size usually associated with 8- and 16-bit devices.

The STM32F103xx performance line family having an embedded $\mathsf { A r m } ^ { \circledast }$ core, it is compatible with all $\doteq \mathrm { \Delta } \dot { \mathsf { m } } ^ { \circledast }$ tools and software.

Figure 1 shows the general block diagram of the device family.

## 2.3.2 Embedded flash memory

64 or 128 Kbytes of embedded flash memory is available for storing programs and data.

## 2.3.3 CRC (cyclic redundancy check) calculation unit

The CRC (cyclic redundancy check) calculation unit is used to get a CRC code from a 32-bit data word and a fixed generator polynomial.

Among other applications, CRC-based techniques are used to verify data transmission or storage integrity. In the scope of the EN/IEC 60335-1 standard, they offer a means of verifying the flash memory integrity. The CRC calculation unit helps compute a signature of the software during runtime, to be compared with a reference signature generated at linktime and stored at a given memory location.

## 2.3.4 Embedded SRAM

Twenty Kbytes of embedded SRAM accessed (read/write) at CPU clock speed with 0 wait states.

## 2.3.5 Nested vectored interrupt controller (NVIC)

The STM32F103xx performance line embeds a nested vectored interrupt controller able to handle up to 43 maskable interrupt channels (not including the 16 interrupt lines of Cortex<sup>®</sup>- M3) and 16 priority levels.

• Closely coupled NVIC gives low-latency interrupt processing

• Interrupt entry vector table address passed directly to the core

• Closely coupled NVIC core interface

• Allows early processing of interrupts

• Processing of late arriving higher priority interrupts

• Support for tail-chaining

• Processor state automatically saved

• Interrupt entry restored on interrupt exit with no instruction overhead

This hardware block provides flexible interrupt management features with minimal interrupt latency.

## 2.3.6 External interrupt/event controller (EXTI)

The external interrupt/event controller consists of 19 edge detector lines used to generate interrupt/event requests. Each line can be independently configured to select the trigger event (rising edge, falling edge, both) and can be masked independently. A pending register maintains the status of the interrupt requests. The EXTI can detect an external line with a pulse width shorter than the internal APB2 clock period. Up to 80 GPIOs can be connected to the 16 external interrupt lines.

## 2.3.7 Clocks and startup

System clock selection is performed on startup, but the internal RC 8 MHz oscillator is selected as the default CPU clock on reset. An external 4-16 MHz clock can be selected, in which case it is monitored for failure. If failure is detected, the system automatically switches back to the internal RC oscillator. A software interrupt is generated if enabled. Similarly, full interrupt management of the PLL clock entry is available when necessary (for example, on failure of an indirectly used external crystal, resonator, or oscillator).

Several prescalers allow the configuration of the AHB frequency, the high-speed APB (APB2) and the low-speed APB (APB1) domains. The maximum frequency of the AHB and the high-speed APB domains is 72 MHz. The maximum allowed frequency of the low-speed APB domain is 36 MHz. See Figure 2 for details on the clock tree.

## 2.3.8 Boot modes

At startup, boot pins are used to select one of three boot options:

• Boot from user Flash

• Boot from system memory

• Boot from embedded SRAM

The bootloader is located in the system memory. It is used to reprogram the flash memory by using USART1. For further details, refer to AN2606, available on www.st.com.

## 2.3.9 Power supply schemes

$\mathsf { V } _ { \mathsf { D D } } = 2 . 0$ to 3.6 V: external power supply for I/Os and the internal regulator.

Provided externally through $\mathsf { V } _ { \mathsf { D D } }$ pins.

$\mathsf { V } _ { \mathsf { S S A } } , \mathsf { V } _ { \mathsf { D D A } } = 2 . 0$ to 3.6 V: external analog power supplies for ADC, reset blocks, RCs, and PLL (minimum voltage to be applied to $\mathsf { V } _ { \mathsf { D D A } }$ is 2.4 V when the ADC is used). $\mathsf { V } _ { \mathsf { D D A } }$ and $\mathsf { V } _ { \mathsf { S S A } }$ must be connected to $\mathsf { V } _ { \mathsf { D D } }$ and $\mathsf { V } _ { \mathsf { S S } }$ , respectively.

$\mathsf { V } _ { \mathsf { B A T } } = 1 . 8$ to 3.6 V: power supply for RTC, external clock 32 kHz oscillator and backup registers (through power switch) when $\mathsf { V } _ { \mathsf { D D } }$ is not present.

For more details on how to connect power pins, refer to Figure 14: Power supply scheme.

## 2.3.10 Power supply supervisor

The device has an integrated power-on reset (POR)/power-down reset (PDR) circuitry. It is always active, and ensures proper operation starting from/down to $_ { 2 \lor . }$ The device remains in reset mode when $\mathsf { V } _ { \mathsf { D D } }$ is below a specified threshold, V<sub>POR/PDR</sub>, without the need for an external reset circuit.

The device features an embedded programmable voltage detector (PVD) that monitors the $\mathsf { V } _ { \mathsf { D D } } / \mathsf { V } _ { \mathsf { D D A } }$ power supply and compares it to the $\mathsf { V } _ { \mathsf { P V D } }$ threshold. An interrupt can be generated when $\mathsf { V } _ { \mathsf { D D } } / \mathsf { V } _ { \mathsf { D D A } }$ drops below the $\mathsf { V } _ { \mathsf { P V D } }$ threshold and/or when $\mathsf { V } _ { \mathsf { D D } } / \mathsf { V } _ { \mathsf { D D A } }$ is higher than the $\mathsf { V } _ { \mathsf { P V D } }$ threshold. The interrupt service routine can then generate a warning message and/or put the MCU into a safe state. The PVD is enabled by software.

Refer to Table 11 for the values of V<sub>POR/PDR</sub> and $\mathsf { V } _ { \mathsf { P V D } }$ .

## 2.3.11 Voltage regulator

The regulator has three operation modes: main (MR), low-power (LPR) and power down.

• MR is used in the nominal regulation mode (Run)

• LPR is used in the Stop mode

Power down is used in Standby mode: the regulator output is in high impedance: the kernel circuitry is powered down, inducing zero consumption (but the contents of the registers and SRAM are lost)

This regulator is always enabled after reset. It is disabled in Standby mode, providing high impedance output.

## 2.3.12 Low-power modes

The STM32F103xx performance line supports three low-power modes to achieve the best compromise between low-power consumption, short startup time and available wakeup sources:

• Sleep mode

In Sleep mode, only the CPU is stopped. All peripherals continue to operate and can wake up the CPU when an interrupt/event occurs.

• Stop mode

The Stop mode achieves the lowest power consumption while retaining the content of SRAM and registers. All clocks in the 1.8 V domain are stopped, the PLL, the HSI RC and the HSE crystal oscillators are disabled. The voltage regulator can also be put either in normal or in low-power mode.

The device can be woken up from Stop mode by any of the EXTI lines. The EXTI line source can be one of the 16 external lines, the PVD output, the RTC alarm or the USB wakeup.

## • Standby mode

The Standby mode is used to achieve the lowest power consumption. The internal voltage regulator is switched off so that the entire 1.8 V domain is powered off. The PLL, the HSI RC and the HSE crystal oscillators are also switched off. After entering Standby mode, SRAM and register contents are lost except for registers in the Backup domain and Standby circuitry.

The device exits Standby mode when an external reset (NRST pin), an IWDG reset, a rising edge on the WKUP pin, or an RTC alarm occurs.

The RTC, the IWDG, and the corresponding clock sources are not stopped by entering Stop or Standby mode.

## 2.3.13 DMA

The flexible 7-channel general-purpose DMA is able to manage memory-to-memory, peripheral-to-memory and memory-to-peripheral transfers. The DMA controller supports circular buffer management avoiding the generation of interrupts when the controller reaches the end of the buffer.

Each channel is connected to dedicated hardware DMA requests, with support for software trigger on each channel. Configuration is made by software and transfer sizes between source and destination are independent.

The DMA can be used with the main peripherals: SPI, ${ \mathsf { I } } ^ { 2 } { \mathsf { C } } ,$ USART, general-purpose and advanced-control timers TIMx and ADC.

## 2.3.14 RTC (real-time clock) and backup registers

The RTC and the backup registers are supplied through a switch that takes power either on $\mathsf { V } _ { \mathsf { D D } }$ supply when present or through the $\mathsf { V } _ { \mathsf { B A T } }$ pin. The backup registers are ten 16-bit registers used to store 20 bytes of user application data when $\mathsf { V } _ { \mathsf { D D } }$ power is not present.

The real-time clock provides a set of continuously running counters which can be used with suitable software to provide a clock calendar function, and provides an alarm interrupt and a periodic interrupt. It is clocked by a 32.768 kHz external crystal, resonator or oscillator, the internal low-power RC oscillator or the high-speed external clock divided by 128. The internal low-power RC has a typical frequency of 40 kHz. The RTC can be calibrated using an external 512 Hz output to compensate for any natural crystal deviation. The RTC features a 32-bit programmable counter for long-term measurement using the Compare register to generate an alarm. A 20-bit prescaler is used for the time base clock and is by default configured to generate a time base of 1 second from a clock at 32.768 kHz.

## 2.3.15 Timers and watchdogs

The medium-density STM32F103xx performance line devices include an advanced-control timer, three general-purpose timers, two watchdog timers and a SysTick timer.

Table 4 compares the features of the advanced-control and general-purpose timers.

Table 4. Timer feature comparison

<table><tr><td>Timer</td><td>Counter resolution</td><td>Counter type</td><td>Prescaler factor</td><td>DMA request generation</td><td>Capture/compare channels</td><td>Complementary outputs</td></tr><tr><td>TIM1</td><td>16-bit</td><td>Up, down, up/down</td><td>Any integer between 1 and 65536</td><td>Yes</td><td>4</td><td>Yes</td></tr><tr><td>TIM2, TIM3, TIM4</td><td>16-bit</td><td>Up, down, up/down</td><td>Any integer between 1 and 65536</td><td>Yes</td><td>4</td><td>No</td></tr></table>

## Advanced-control timer (TIM1)

The advanced-control timer (TIM1) can be seen as a three-phase PWM multiplexed on 6 channels. It has complementary PWM outputs with programmable inserted dead-times. It can also be seen as a complete general-purpose timer. The 4 independent channels can be used for

• Input capture

• Output compare

• PWM generation (edge- or center-aligned modes)

• One-pulse mode output

If configured as a general-purpose 16-bit timer, it has the same features as the TIMx timer. If configured as the 16-bit PWM generator, it has full modulation capability (0-100%).

In debug mode, the advanced-control timer counter can be frozen and the PWM outputs disabled to turn off any power switch driven by these outputs.

Many features are shared with those of the general-purpose TIM timers which have the same architecture. The advanced-control timer can therefore work together with the TIM timers via the Timer Link feature for synchronization or event chaining.

## General-purpose timers (TIMx)

There are up to three synchronizable general-purpose timers embedded in the STM32F103xx performance line devices. These timers are based on a 16-bit auto-reload up/down counter, a 16-bit prescaler and feature four independent channels each for input capture/output compare, PWM or one-pulse mode output. This gives up to 12 input captures/output compares/PWMs on the largest packages.

The general-purpose timers can work together with the advanced-control timer via the Timer Link feature for synchronization or event chaining. Their counter can be frozen in debug mode. Any of the general-purpose timers can be used to generate PWM outputs. They all have independent DMA request generation.

These timers are capable of handling quadrature (incremental) encoder signals and the digital outputs from one to three Hall-effect sensors.

## Independent watchdog

The independent watchdog is based on a 12-bit downcounter and 8-bit prescaler. It is clocked from an independent 40 kHz internal RC and as it operates independently of the main clock, it can operate in Stop and Standby modes. It can be used either as a watchdog to reset the device when a problem occurs, or as a free-running timer for application timeout management. It is hardware- or software-configurable through the option bytes. The counter can be frozen in debug mode.

## Window watchdog

The window watchdog is based on a 7-bit downcounter that can be set as free-running. It can be used as a watchdog to reset the device when a problem occurs. It is clocked from the main clock. It has an early warning interrupt capability and the counter can be frozen in debug mode.

## SysTick timer

This timer is dedicated for OS, but can be used also as a standard downcounter. It features:

• A 24-bit downcounter

• Autoreload capability

• Maskable system interrupt generation when the counter reaches 0

• Programmable clock source

## 2.3.16 I²C bus

Up to two I²C bus interfaces can operate in multimaster and slave modes. They can support standard and fast modes.

They support dual slave addressing (7-bit only) and both 7/10-bit addressing in master mode. A hardware CRC generation/verification is embedded.

They can be served by DMA and they support SM Bus 2.0/PM Bus.

## 2.3.17 Universal synchronous/asynchronous receiver transmitter (USART)

One of the USART interfaces is able to communicate at speeds of up to 4.5 Mbit/s. The other available interfaces communicate at up to 2.25 Mbit/s. They provide hardware management of the CTS and RTS signals, IrDA SIR ENDEC support, are ISO 7816 compliant and have LIN Master/Slave capability.

All USART interfaces can be served by the DMA controller.

## 2.3.18 Serial peripheral interface (SPI)

Up to two SPIs are able to communicate up to 18 Mbits/s in slave and master modes in full-duplex and simplex communication modes. The 3-bit prescaler gives 8 master mode frequencies and the frame is configurable to 8 bits or 16 bits. The hardware CRC generation/verification supports basic SD Card/MMC modes.

Both SPIs can be served by the DMA controller.

## 2.3.19 Controller area network (CAN)

The CAN is compliant with specifications 2.0A and B (active) with a bit rate up to 1 Mbit/s. It can receive and transmit standard frames with 11-bit identifiers as well as extended frames with 29-bit identifiers. It has three transmit mailboxes, two receive FIFOs with three stages and 14 scalable filter banks.

## 2.3.20 Universal serial bus (USB)

The STM32F103xx performance line embeds a USB device peripheral compatible with the USB full-speed 12 Mbs. The USB interface implements a full-speed (12 Mbit/s) function interface. It has software-configurable endpoint setting and suspend/resume support. The dedicated 48 MHz clock is generated from the internal main PLL (the clock source must use a HSE crystal oscillator).

## 2.3.21 GPIOs (general-purpose inputs/outputs)

Each of the GPIO pins can be configured by software as output (push-pull or open-drain), as input (with or without pull-up or pull-down) or as peripheral alternate function. Most of the GPIO pins are shared with digital or analog alternate functions. All GPIOs are high currentcapable.

The I/Os alternate function configuration can be locked if needed following a specific sequence in order to avoid spurious writing to the I/Os registers.

I/Os on APB2 with up to 18 MHz toggling speed.

## 2.3.22 ADC (analog-to-digital converter)

Two 12-bit analog-to-digital converters are embedded into STM32F103xx performance line devices and each ADC shares up to 16 external channels, performing conversions in singleshot or scan modes. In scan mode, automatic conversion is performed on a selected group of analog inputs.

Additional logic functions embedded in the ADC interface allow:

• Simultaneous sample and hold

• Interleaved sample and hold

• Single shunt

The ADC can be served by the DMA controller.

An analog watchdog feature allows very precise monitoring of the converted voltage of one, some or all selected channels. An interrupt is generated when the converted voltage is outside the programmed thresholds.

The events generated by the general-purpose timers (TIMx) and the advanced-control timer (TIM1) can be internally connected to the ADC start trigger, injection trigger, and DMA trigger respectively, to allow the application to synchronize A/D conversion and timers.

## 2.3.23 Temperature sensor

The temperature sensor has to generate a voltage that varies linearly with temperature. The conversion range is between $2 \vee < \vee _ { \tt D D A } < 3 . 6 \vee$ . The temperature sensor is internally connected to the ADC12\_IN16 input channel which is used to convert the sensor output voltage into a digital value.

## 2.3.24 Serial wire JTAG debug port (SWJ-DP)

The Arm SWJ-DP Interface is embedded. and is a combined JTAG and serial wire debug port that enables either a serial wire debug or a JTAG probe to be connected to the target. The JTAG TMS and TCK pins are shared with SWDIO and SWCLK, respectively, and a specific sequence on the TMS pin is used to switch between JTAG-DP and SW-DP.

# 3 Pinouts and pin description

Figure 3. STM32F103xx performance line LFBGA100 ballout  
![](images/1191351c1d75919df0815409a2b59e984221391e83e390d627b14f8cbaba7140.jpg)

Figure 4. STM32F103xx performance line LQFP100 pinout

<table><tr><td></td><td>100</td><td>VDD_3</td><td>VSS_3</td><td>PE1</td><td>PE0</td><td>PB9</td><td>PB8</td><td>BOOT0</td><td>PB7</td><td>PB6</td><td>PB5</td><td>PB4</td><td>PB3</td><td>PD7</td><td>PD6</td><td>PD5</td><td>PD4</td><td>PD3</td><td>PD2</td><td>PD1</td><td>PD0</td><td>PC12</td><td>PC11</td><td>PC10</td><td>PA15</td><td>PA14</td></tr><tr><td>PE2</td><td>1</td><td>99</td><td>98</td><td>97</td><td>96</td><td>95</td><td>94</td><td>93</td><td>92</td><td>91</td><td>90</td><td>89</td><td>88</td><td>87</td><td>86</td><td>85</td><td>84</td><td>83</td><td>82</td><td>81</td><td>80</td><td>79</td><td>78</td><td>77</td><td>76</td><td></td></tr><tr><td>PE3</td><td>2</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>75</td><td>VDD_2</td></tr><tr><td>PE4</td><td>3</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>74</td><td>VSS_2</td></tr><tr><td>PE5</td><td>4</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>73</td><td>NC</td></tr><tr><td>PE6</td><td>5</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>72</td><td>PA 13</td></tr><tr><td>VBAT</td><td>6</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>71</td><td>PA 12</td></tr><tr><td>PC13-TAMPER-RTC</td><td>7</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>70</td><td>PA 11</td></tr><tr><td>PC14-OSC32_IN</td><td>8</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>69</td><td>PA 10</td></tr><tr><td>PC15-OSC32_OUT</td><td>9</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>68</td><td>PA 9</td></tr><tr><td>VSS_5</td><td>10</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>67</td><td>PA 8</td></tr><tr><td>VDD_5</td><td>11</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>66</td><td>PC9</td></tr><tr><td>OSC_IN</td><td>12</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>65</td><td>PC8</td></tr><tr><td>OSC_OUT</td><td>13</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>64</td><td>PC7</td></tr><tr><td>NRST</td><td>14</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>63</td><td>PC6</td></tr><tr><td>PC0</td><td>15</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>62</td><td>PD15</td></tr><tr><td>PC1</td><td>16</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>61</td><td>PD14</td></tr><tr><td>PC2</td><td>17</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>60</td><td>PD13</td></tr><tr><td>PC3</td><td>18</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>59</td><td>PD12</td></tr><tr><td>VSSA</td><td>19</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>58</td><td>PD11</td></tr><tr><td>VREF-</td><td>20</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>57</td><td>PD10</td></tr><tr><td>VREF+</td><td>21</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>56</td><td>PD9</td></tr><tr><td>VDDA</td><td>22</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>55</td><td>PD8</td></tr><tr><td>PA0-WKUP</td><td>23</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>54</td><td>PB15</td></tr><tr><td>PA1</td><td>24</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>53</td><td>PB14</td></tr><tr><td>PA2</td><td>25</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>52</td><td>PB13</td></tr><tr><td>PA3</td><td>26</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>51</td><td>PB12</td></tr><tr><td>VSS_4</td><td>27</td><td>28</td><td>29</td><td>30</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>PB11</td></tr><tr><td>VDD_4</td><td>28</td><td>29</td><td>30</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB10</td></tr><tr><td>PA4</td><td>29</td><td>30</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB11</td><td>PB10</td></tr><tr><td>PA5</td><td>30</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PA6</td><td>31</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PA7</td><td>32</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PC4</td><td>33</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PC5</td><td>34</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PB0</td><td>35</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PB1</td><td>36</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PB2</td><td>37</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB19</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PE7</td><td>38</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB20</td><td>PB19</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td></tr><tr><td>PE8</td><td>39</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB21</td><td>PB20</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td></tr><tr><td>PE9</td><td>40</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB22</td><td>PB21</td><td>PB19</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td></tr><tr><td>PE10</td><td>41</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB23</td><td>PB22</td><td>PB21</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td><td>PB10</td></tr><tr><td>PE11</td><td>42</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB24</td><td>PB23</td><td>PB22</td><td>PB21</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td><td>PB10</td></tr><tr><td>PE12</td><td>43</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB25</td><td>PB24</td><td>PB23</td><td>PB22</td><td>PB21</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td><td>PB10</td></tr><tr><td>PE13</td><td>44</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB26</td><td>PB25</td><td>PB24</td><td>PB23</td><td>PB22</td><td>PB21</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td><td>PB10</td></tr><tr><td>PE14</td><td>45</td><td>46</td><td>47</td><td>48</td><td>49</td><td>50</td><td>51</td><td>52</td><td>PB27</td><td>PB26</td><td>PB25</td><td>PB24</td><td>PB23</td><td>PB22</td><td>PB21</td><td>PB18</td><td>PB17</td><td>PB16</td><td>PB15</td><td>PB14</td><td>PB13</td><td>PB12</td><td>PB11</td><td>PB10</td><td>PB10</td><td>PB10</td></tr></table>

Figure 5. STM32F103xx performance line UFBGA100 pinout  
![](images/300b5ad2124c88920fc9552bd752db93c343eddaacfdeb9b2100a454cb9a6345.jpg)

Figure 6. STM32F103xx performance line LQFP64 pinout  
![](images/ea3acde9d23f29d9e50263eb20adffd91150cd6de2a1c6c67696635683052c2e.jpg)

Figure 7. STM32F103xx performance line TFBGA64 ballout  
![](images/2fd606bcb12021cc0ca94570ba6b1994fbcc37e95657ef57248e1bb822885f49.jpg)

Figure 8. STM32F103xx performance line LQFP48 pinout  
![](images/422d69138f7dc7630c19350e2e5eaebeca56c9979b7504a6dce7c4f13ea657e4.jpg)

Figure 9. STM32F103xx performance line UFQFPN48 pinout  
![](images/2d95343f3e8018e61e6256b95466a2b031ae396a5c6e6c9002e2724f520f7ae4.jpg)

Figure 10. STM32F103xx performance line VFQFPN36 pinout  
![](images/74c4652198e514d43ac9491f1cccc4230952773d8a6e3d34c704626316a86444.jpg)

Table 5. Medium-density STM32F103xx pin definitions

<table><tr><td colspan="7">Pins</td><td rowspan="2">Pin name</td><td rowspan="2">Type(1)</td><td rowspan="2">I/O Level(2)</td><td rowspan="2">Main function(3)(after reset)</td><td colspan="2">Alternate functions(4)</td></tr><tr><td>LFBGA100</td><td>UFBG100</td><td>LQFP48/UFQFPN48</td><td>TFBGA64</td><td>LQFP64</td><td>LQFP100</td><td>VFQFPN36</td><td>Default</td><td>Remap</td></tr><tr><td>A3</td><td>B2</td><td>-</td><td>-</td><td>-</td><td>1</td><td>-</td><td>PE2</td><td>I/O</td><td>FT</td><td>PE2</td><td>TRACECK</td><td>-</td></tr><tr><td>B3</td><td>A1</td><td>-</td><td>-</td><td>-</td><td>2</td><td>-</td><td>PE3</td><td>I/O</td><td>FT</td><td>PE3</td><td>TRACED0</td><td>-</td></tr><tr><td>C3</td><td>B1</td><td>-</td><td>-</td><td>-</td><td>3</td><td>-</td><td>PE4</td><td>I/O</td><td>FT</td><td>PE4</td><td>TRACED1</td><td>-</td></tr><tr><td>D3</td><td>C2</td><td>-</td><td>-</td><td>-</td><td>4</td><td>-</td><td>PE5</td><td>I/O</td><td>FT</td><td>PE5</td><td>TRACED2</td><td>-</td></tr><tr><td>E3</td><td>D2</td><td>-</td><td>-</td><td>-</td><td>5</td><td>-</td><td>PE6</td><td>I/O</td><td>FT</td><td>PE6</td><td>TRACED3</td><td>-</td></tr><tr><td>B2</td><td>E2</td><td>1</td><td>B2</td><td>1</td><td>6</td><td>-</td><td> $V_{BAT}$ </td><td>S</td><td>-</td><td> $V_{BAT}$ </td><td>-</td><td>-</td></tr><tr><td>A2</td><td>C1</td><td>2</td><td>A2</td><td>2</td><td>7</td><td>-</td><td>PC13-TAMPER- $RTC^{(5)}$ </td><td>I/O</td><td>-</td><td>PC13(6)</td><td>TAMPER-RTC</td><td>-</td></tr><tr><td>A1</td><td>D1</td><td>3</td><td>A1</td><td>3</td><td>8</td><td>-</td><td>PC14-OSC32_IN(5)</td><td>I/O</td><td>-</td><td>PC14(6)</td><td>OSC32_IN</td><td>-</td></tr><tr><td>B1</td><td>E1</td><td>4</td><td>B1</td><td>4</td><td>9</td><td>-</td><td>PC15- $OSC32\_OUT^{(5)}$ </td><td>I/O</td><td>-</td><td>PC15(6)</td><td>OSC32_OUT</td><td>-</td></tr><tr><td>C2</td><td>F2</td><td>-</td><td>-</td><td>-</td><td>10</td><td>-</td><td> $V_{SS\_5}$ </td><td>S</td><td>-</td><td> $V_{SS\_5}$ </td><td>-</td><td>-</td></tr><tr><td>D2</td><td>G2</td><td>-</td><td>-</td><td>-</td><td>11</td><td>-</td><td> $V_{DD\_5}$ </td><td>S</td><td>-</td><td> $V_{DD\_5}$ </td><td>-</td><td></td></tr><tr><td>C1</td><td>F1</td><td>5</td><td>C1</td><td>5</td><td>12</td><td>2</td><td>OSC_IN</td><td>I</td><td>-</td><td>OSC_IN</td><td>-</td><td> $PD0^{(7)}$ </td></tr><tr><td>D1</td><td>G1</td><td>6</td><td>D1</td><td>6</td><td>13</td><td>3</td><td>OSC_OUT</td><td>O</td><td>-</td><td>OSC_OUT</td><td></td><td> $PD1^{(7)}$ </td></tr><tr><td>E1</td><td>H2</td><td>7</td><td>E1</td><td>7</td><td>14</td><td>4</td><td>NRST</td><td>I/O</td><td>-</td><td>NRST</td><td>-</td><td>-</td></tr><tr><td>F1</td><td>H1</td><td>-</td><td>E3</td><td>8</td><td>15</td><td>-</td><td>PC0</td><td>I/O</td><td>-</td><td>PC0</td><td>ADC12_IN10</td><td>-</td></tr><tr><td>F2</td><td>J2</td><td>-</td><td>E2</td><td>9</td><td>16</td><td>-</td><td>PC1</td><td>I/O</td><td>-</td><td>PC1</td><td>ADC12_IN11</td><td>-</td></tr><tr><td>E2</td><td>J3</td><td>-</td><td>F2</td><td>10</td><td>17</td><td>-</td><td>PC2</td><td>I/O</td><td>-</td><td>PC2</td><td>ADC12_IN12</td><td>-</td></tr><tr><td>F3</td><td>K2</td><td>-</td><td>-(8)</td><td>11</td><td>18</td><td>-</td><td>PC3</td><td>I/O</td><td>-</td><td>PC3</td><td>ADC12_IN13</td><td>-</td></tr><tr><td>G1</td><td>J1</td><td>8</td><td>F1</td><td>12</td><td>19</td><td>5</td><td> $V_{SSA}$ </td><td>S</td><td>-</td><td> $V_{SSA}$ </td><td>-</td><td>-</td></tr><tr><td>H1</td><td>K1</td><td>-</td><td>-</td><td>-</td><td>20</td><td>-</td><td> $V_{REF-}$ </td><td>S</td><td>-</td><td> $V_{REF-}$ </td><td>-</td><td>-</td></tr><tr><td>J1</td><td>L1</td><td>-</td><td> $G1^{(8)}$ </td><td>-</td><td>21</td><td>-</td><td> $V_{REF+}$ </td><td>S</td><td>-</td><td> $V_{REF+}$ </td><td>-</td><td>-</td></tr><tr><td>K1</td><td>M1</td><td>9</td><td>H1</td><td>13</td><td>22</td><td>6</td><td> $V_{DDA}$ </td><td>S</td><td>-</td><td> $V_{DDA}$ </td><td>-</td><td>-</td></tr><tr><td>G2</td><td>L2</td><td>10</td><td>G2</td><td>14</td><td>23</td><td>7</td><td>PA0-WKUP</td><td>I/O</td><td>-</td><td>PA0</td><td>WKUP/USART2_CTS(9)/ADC12_IN0/TIM2_CH1_ETR(9)</td><td>-</td></tr><tr><td>H2</td><td>M2</td><td>11</td><td>H2</td><td>15</td><td>24</td><td>8</td><td>PA1</td><td>I/O</td><td>-</td><td>PA1</td><td>USART2_RTS(9)/ADC12_IN1/TIM2_CH2(9)</td><td>-</td></tr><tr><td>J2</td><td>K3</td><td>12</td><td>F3</td><td>16</td><td>25</td><td>9</td><td>PA2</td><td>I/O</td><td>-</td><td>PA2</td><td>USART2_TX(9)/ADC12_IN2/TIM2_CH3(9)</td><td>-</td></tr><tr><td>K2</td><td>L3</td><td>13</td><td>G3</td><td>17</td><td>26</td><td>10</td><td>PA3</td><td>I/O</td><td>-</td><td>PA3</td><td>USART2_RX(9)/ADC12_IN3/TIM2_CH4(9)</td><td>-</td></tr><tr><td>E4</td><td>E3</td><td>-</td><td>C2</td><td>18</td><td>27</td><td>-</td><td> $V_{SS\_4}$ </td><td>S</td><td>-</td><td> $V_{SS\_4}$ </td><td>-</td><td>-</td></tr><tr><td>F4</td><td>H3</td><td>-</td><td>D2</td><td>19</td><td>28</td><td>-</td><td> $V_{DD\_4}$ </td><td>S</td><td>-</td><td> $V_{DD\_4}$ </td><td>-</td><td>-</td></tr><tr><td>G3</td><td>M3</td><td>14</td><td>H3</td><td>20</td><td>29</td><td>11</td><td>PA4</td><td>I/O</td><td>-</td><td>PA4</td><td>SPI1_NSS(9)/USART2_CK(9)/ADC12_IN4</td><td>-</td></tr><tr><td>H3</td><td>K4</td><td>15</td><td>F4</td><td>21</td><td>30</td><td>12</td><td>PA5</td><td>I/O</td><td>-</td><td>PA5</td><td>SPI1_SCK(9)/ADC12_IN5</td><td>-</td></tr><tr><td>J3</td><td>L4</td><td>16</td><td>G4</td><td>22</td><td>31</td><td>13</td><td>PA6</td><td>I/O</td><td>-</td><td>PA6</td><td>SPI1_MISO(9)/ADC12_IN6/TIM3_CH1(9)</td><td>TIM1_BKIN</td></tr><tr><td>K3</td><td>M4</td><td>17</td><td>H4</td><td>23</td><td>32</td><td>14</td><td>PA7</td><td>I/O</td><td>-</td><td>PA7</td><td>SPI1_MOSI(9)/ADC12_IN7/TIM3_CH2(9)</td><td>TIM1_CH1N</td></tr><tr><td>G4</td><td>K5</td><td>-</td><td>H5</td><td>24</td><td>33</td><td></td><td>PC4</td><td>I/O</td><td>-</td><td>PC4</td><td>ADC12_IN14</td><td>-</td></tr><tr><td>H4</td><td>L5</td><td>-</td><td>H6</td><td>25</td><td>34</td><td></td><td>PC5</td><td>I/O</td><td>-</td><td>PC5</td><td>ADC12_IN15</td><td>-</td></tr><tr><td>J4</td><td>M5</td><td>18</td><td>F5</td><td>26</td><td>35</td><td>15</td><td>PB0</td><td>I/O</td><td>-</td><td>PB0</td><td>ADC12_IN8/TIM3_CH3(9)</td><td>TIM1_CH2N</td></tr><tr><td>K4</td><td>M6</td><td>19</td><td>G5</td><td>27</td><td>36</td><td>16</td><td>PB1</td><td>I/O</td><td>-</td><td>PB1</td><td>ADC12_IN9/TIM3_CH4(9)</td><td>TIM1_CH3N</td></tr><tr><td>G5</td><td>L6</td><td>20</td><td>G6</td><td>28</td><td>37</td><td>17</td><td>PB2</td><td>I/O</td><td>FT</td><td>PB2/BOOT1</td><td>-</td><td>-</td></tr><tr><td>H5</td><td>M7</td><td>-</td><td>-</td><td>-</td><td>38</td><td>-</td><td>PE7</td><td>I/O</td><td>FT</td><td>PE7</td><td>-</td><td>TIM1_ETR</td></tr><tr><td>J5</td><td>L7</td><td>-</td><td>-</td><td>-</td><td>39</td><td>-</td><td>PE8</td><td>I/O</td><td>FT</td><td>PE8</td><td>-</td><td>TIM1_CH1N</td></tr><tr><td>K5</td><td>M8</td><td>-</td><td>-</td><td>-</td><td>40</td><td>-</td><td>PE9</td><td>I/O</td><td>FT</td><td>PE9</td><td>-</td><td>TIM1_CH1</td></tr><tr><td>G6</td><td>L8</td><td>-</td><td>-</td><td>-</td><td>41</td><td>-</td><td>PE10</td><td>I/O</td><td>FT</td><td>PE10</td><td>-</td><td>TIM1_CH2N</td></tr><tr><td>H6</td><td>M9</td><td>-</td><td>-</td><td>-</td><td>42</td><td>-</td><td>PE11</td><td>I/O</td><td>FT</td><td>PE11</td><td>-</td><td>TIM1_CH2</td></tr><tr><td>J6</td><td>L9</td><td>-</td><td>-</td><td>-</td><td>43</td><td>-</td><td>PE12</td><td>I/O</td><td>FT</td><td>PE12</td><td>-</td><td>TIM1_CH3N</td></tr><tr><td>K6</td><td>M10</td><td>-</td><td>-</td><td>-</td><td>44</td><td>-</td><td>PE13</td><td>I/O</td><td>FT</td><td>PE13</td><td>-</td><td>TIM1_CH3</td></tr><tr><td>G7</td><td>M11</td><td>-</td><td>-</td><td>-</td><td>45</td><td>-</td><td>PE14</td><td>I/O</td><td>FT</td><td>PE14</td><td>-</td><td>TIM1_CH4</td></tr><tr><td>H7</td><td>M12</td><td>-</td><td>-</td><td>-</td><td>46</td><td>-</td><td>PE15</td><td>I/O</td><td>FT</td><td>PE15</td><td>-</td><td>TIM1_BKIN</td></tr><tr><td>J7</td><td>L10</td><td>21</td><td>G7</td><td>29</td><td>47</td><td>-</td><td>PB10</td><td>I/O</td><td>FT</td><td>PB10</td><td>I2C2_SCL/USART3_TX(9)</td><td>TIM2_CH3</td></tr><tr><td>K7</td><td>L11</td><td>22</td><td>H7</td><td>30</td><td>48</td><td>-</td><td>PB11</td><td>I/O</td><td>FT</td><td>PB11</td><td>I2C2_SDA/USART3_RX(9)</td><td>TIM2_CH4</td></tr><tr><td>E7</td><td>F12</td><td>23</td><td>D6</td><td>31</td><td>49</td><td>18</td><td> $V_{SS\_1}$ </td><td>S</td><td>-</td><td> $V_{SS\_1}$ </td><td>-</td><td>-</td></tr><tr><td>F7</td><td>G12</td><td>24</td><td>E6</td><td>32</td><td>50</td><td>19</td><td> $V_{DD\_1}$ </td><td>S</td><td>-</td><td> $V_{DD\_1}$ </td><td>-</td><td>-</td></tr><tr><td>K8</td><td>L12</td><td>25</td><td>H8</td><td>33</td><td>51</td><td>-</td><td>PB12</td><td>I/O</td><td>FT</td><td>PB12</td><td>SPI2_NSS/I2C2_SMBAI/USART3_CK(9)/TIM1_BKIN(9)</td><td>-</td></tr><tr><td>J8</td><td>K12</td><td>26</td><td>G8</td><td>34</td><td>52</td><td>-</td><td>PB13</td><td>I/O</td><td>FT</td><td>PB13</td><td>SPI2_SCK/USART3_CTS(9)/TIM1_CH1N(9)</td><td>-</td></tr><tr><td>H8</td><td>K11</td><td>27</td><td>F8</td><td>35</td><td>53</td><td>-</td><td>PB14</td><td>I/O</td><td>FT</td><td>PB14</td><td>SPI2_MISO/USART3_RTS(9)TIM1_CH2N(9)</td><td>-</td></tr><tr><td>G8</td><td>K10</td><td>28</td><td>F7</td><td>36</td><td>54</td><td>-</td><td>PB15</td><td>I/O</td><td>FT</td><td>PB15</td><td>SPI2_MOSI/TIM1_CH3N(9)</td><td>-</td></tr><tr><td>K9</td><td>K9</td><td>-</td><td>-</td><td>-</td><td>55</td><td>-</td><td>PD8</td><td>I/O</td><td>FT</td><td>PD8</td><td>-</td><td>USART3_TX</td></tr><tr><td>J9</td><td>K8</td><td>-</td><td>-</td><td>-</td><td>56</td><td>-</td><td>PD9</td><td>I/O</td><td>FT</td><td>PD9</td><td>-</td><td>USART3_RX</td></tr><tr><td>H9</td><td>J12</td><td>-</td><td>-</td><td>-</td><td>57</td><td>-</td><td>PD10</td><td>I/O</td><td>FT</td><td>PD10</td><td>-</td><td>USART3_CK</td></tr><tr><td>G9</td><td>J11</td><td>-</td><td>-</td><td>-</td><td>58</td><td>-</td><td>PD11</td><td>I/O</td><td>FT</td><td>PD11</td><td>-</td><td>USART3_CTS</td></tr><tr><td>K10</td><td>J10</td><td>-</td><td>-</td><td>-</td><td>59</td><td>-</td><td>PD12</td><td>I/O</td><td>FT</td><td>PD12</td><td>-</td><td>TIM4_CH1 / USART3_RTS</td></tr><tr><td>J10</td><td>H12</td><td>-</td><td>-</td><td>-</td><td>60</td><td>-</td><td>PD13</td><td>I/O</td><td>FT</td><td>PD13</td><td>-</td><td>TIM4_CH2</td></tr><tr><td>H10</td><td>H11</td><td>-</td><td>-</td><td>-</td><td>61</td><td>-</td><td>PD14</td><td>I/O</td><td>FT</td><td>PD14</td><td>-</td><td>TIM4_CH3</td></tr><tr><td>G10</td><td>H10</td><td>-</td><td>-</td><td>-</td><td>62</td><td>-</td><td>PD15</td><td>I/O</td><td>FT</td><td>PD15</td><td>-</td><td>TIM4_CH4</td></tr><tr><td>F10</td><td>E12</td><td>-</td><td>F6</td><td>37</td><td>63</td><td>-</td><td>PC6</td><td>I/O</td><td>FT</td><td>PC6</td><td>-</td><td>TIM3_CH1</td></tr><tr><td>E10</td><td>E11</td><td></td><td>E7</td><td>38</td><td>64</td><td>-</td><td>PC7</td><td>I/O</td><td>FT</td><td>PC7</td><td>-</td><td>TIM3_CH2</td></tr><tr><td>F9</td><td>E10</td><td></td><td>E8</td><td>39</td><td>65</td><td>-</td><td>PC8</td><td>I/O</td><td>FT</td><td>PC8</td><td>-</td><td>TIM3_CH3</td></tr><tr><td>E9</td><td>D12</td><td>-</td><td>D8</td><td>40</td><td>66</td><td>-</td><td>PC9</td><td>I/O</td><td>FT</td><td>PC9</td><td>-</td><td>TIM3_CH4</td></tr><tr><td>D9</td><td>D11</td><td>29</td><td>D7</td><td>41</td><td>67</td><td>20</td><td>PA8</td><td>I/O</td><td>FT</td><td>PA8</td><td>USART1_CK/TIM1_CH1(9)/MCO</td><td>-</td></tr><tr><td>C9</td><td>D10</td><td>30</td><td>C7</td><td>42</td><td>68</td><td>21</td><td>PA9</td><td>I/O</td><td>FT</td><td>PA9</td><td>USART1_TX(9)/TIM1_CH2(9)</td><td>-</td></tr><tr><td>D10</td><td>C12</td><td>31</td><td>C6</td><td>43</td><td>69</td><td>22</td><td>PA10</td><td>I/O</td><td>FT</td><td>PA10</td><td>USART1_RX(9)/TIM1_CH3(9)</td><td>-</td></tr><tr><td>C10</td><td>B12</td><td>32</td><td>C8</td><td>44</td><td>70</td><td>23</td><td>PA11</td><td>I/O</td><td>FT</td><td>PA11</td><td>USART1_CTS/CANRX(9)/USBDM/TIM1_CH4(9)</td><td>-</td></tr><tr><td>B10</td><td>A12</td><td>33</td><td>B8</td><td>45</td><td>71</td><td>24</td><td>PA12</td><td>I/O</td><td>FT</td><td>PA12</td><td>USART1_RTS/CANTX(9)/USBDPTIM1_ETR(9)</td><td>-</td></tr><tr><td>A10</td><td>A11</td><td>34</td><td>A8</td><td>46</td><td>72</td><td>25</td><td>PA13</td><td>I/O</td><td>FT</td><td>JTMS/SWDIO</td><td>-</td><td>PA13</td></tr><tr><td>F8</td><td>C11</td><td>-</td><td>-</td><td>-</td><td>73</td><td>-</td><td colspan="5">Not connected</td><td>-</td></tr><tr><td>E6</td><td>F11</td><td>35</td><td>D5</td><td>47</td><td>74</td><td>26</td><td>VSS_2</td><td>S</td><td>-</td><td>VSS_2</td><td>-</td><td>-</td></tr><tr><td>F6</td><td>G11</td><td>36</td><td>E5</td><td>48</td><td>75</td><td>27</td><td>VDD_2</td><td>S</td><td>-</td><td>VDD_2</td><td>-</td><td>-</td></tr><tr><td>A9</td><td>A10</td><td>37</td><td>A7</td><td>49</td><td>76</td><td>28</td><td>PA14</td><td>I/O</td><td>FT</td><td>JTCK/SWCLK</td><td>-</td><td>PA14</td></tr><tr><td>A8</td><td>A9</td><td>38</td><td>A6</td><td>50</td><td>77</td><td>29</td><td>PA15</td><td>I/O</td><td>FT</td><td>JTDI</td><td>-</td><td>TIM2_CH1_ETR/ PA15/SPI1_NSS</td></tr><tr><td>B9</td><td>B11</td><td>-</td><td>B7</td><td>51</td><td>78</td><td></td><td>PC10</td><td>I/O</td><td>FT</td><td>PC10</td><td>-</td><td>USART3_TX</td></tr><tr><td>B8</td><td>C10</td><td>-</td><td>B6</td><td>52</td><td>79</td><td></td><td>PC11</td><td>I/O</td><td>FT</td><td>PC11</td><td>-</td><td>USART3_RX</td></tr><tr><td>C8</td><td>B10</td><td>-</td><td>C5</td><td>53</td><td>80</td><td></td><td>PC12</td><td>I/O</td><td>FT</td><td>PC12</td><td>-</td><td>USART3_CK</td></tr><tr><td>D8</td><td>C9</td><td>-</td><td>C1</td><td>-</td><td>81</td><td>2</td><td>PD0</td><td>I/O</td><td>FT</td><td>PD0</td><td>-</td><td>CANRX</td></tr><tr><td>E8</td><td>B9</td><td>-</td><td>D1</td><td>-</td><td>82</td><td>3</td><td>PD1</td><td>I/O</td><td>FT</td><td>PD1</td><td>-</td><td>CANTX</td></tr><tr><td>B7</td><td>C8</td><td></td><td>B5</td><td>54</td><td>83</td><td>-</td><td>PD2</td><td>I/O</td><td>FT</td><td>PD2</td><td>TIM3_ETR</td><td>-</td></tr><tr><td>C7</td><td>B8</td><td>-</td><td>-</td><td>-</td><td>84</td><td>-</td><td>PD3</td><td>I/O</td><td>FT</td><td>PD3</td><td>-</td><td>USART2_CTS</td></tr><tr><td>D7</td><td>B7</td><td>-</td><td>-</td><td>-</td><td>85</td><td>-</td><td>PD4</td><td>I/O</td><td>FT</td><td>PD4</td><td>-</td><td>USART2_RTS</td></tr><tr><td>B6</td><td>A6</td><td>-</td><td>-</td><td>-</td><td>86</td><td>-</td><td>PD5</td><td>I/O</td><td>FT</td><td>PD5</td><td>-</td><td>USART2_TX</td></tr><tr><td>C6</td><td>B6</td><td>-</td><td>-</td><td>-</td><td>87</td><td>-</td><td>PD6</td><td>I/O</td><td>FT</td><td>PD6</td><td>-</td><td>USART2_RX</td></tr><tr><td>D6</td><td>A5</td><td>-</td><td>-</td><td>-</td><td>88</td><td>-</td><td>PD7</td><td>I/O</td><td>FT</td><td>PD7</td><td>-</td><td>USART2_CK</td></tr><tr><td>A7</td><td>A8</td><td>39</td><td>A5</td><td>55</td><td>89</td><td>30</td><td>PB3</td><td>I/O</td><td>FT</td><td>JTDO</td><td>-</td><td>TIM2_CH2 / PB3 TRACESWO SPI1_SCK</td></tr><tr><td>A6</td><td>A7</td><td>40</td><td>A4</td><td>56</td><td>90</td><td>31</td><td>PB4</td><td>I/O</td><td>FT</td><td>JNTRST</td><td>-</td><td>TIM3_CH1/PB4/SPI1_MISO</td></tr><tr><td>C5</td><td>C5</td><td>41</td><td>C4</td><td>57</td><td>91</td><td>32</td><td>PB5</td><td>I/O</td><td></td><td>PB5</td><td>I2C1_SMBAI</td><td>TIM3_CH2 / SPI1_MOSI</td></tr><tr><td>B5</td><td>B5</td><td>42</td><td>D3</td><td>58</td><td>92</td><td>33</td><td>PB6</td><td>I/O</td><td>FT</td><td>PB6</td><td>I2C1_SCL(9)/TIM4_CH1(9)</td><td>USART1_TX</td></tr><tr><td>A5</td><td>B4</td><td>43</td><td>C3</td><td>59</td><td>93</td><td>34</td><td>PB7</td><td>I/O</td><td>FT</td><td>PB7</td><td>I2C1_SDA(9)/TIM4_CH2(9)</td><td>USART1_RX</td></tr><tr><td>D5</td><td>A4</td><td>44</td><td>B4</td><td>60</td><td>94</td><td>35</td><td>BOOT0</td><td>I</td><td></td><td>BOOT0</td><td>-</td><td>-</td></tr><tr><td colspan="7">Pins</td><td rowspan="2">Pin name</td><td rowspan="2"> $Type^{(1)}$ </td><td rowspan="2">I/O Level(2)</td><td rowspan="2">Main function(3)(after reset)</td><td colspan="2">Alternate  $functions^{(4)}$ </td></tr><tr><td>LFBGA100</td><td>UFBG100</td><td>LQFP48/UFQFPN48</td><td>TFBGA64</td><td>LQFP64</td><td>LQFP100</td><td>VFQFPN36</td><td>Default</td><td>Remap</td></tr><tr><td>B4</td><td>A3</td><td>45</td><td>B3</td><td>61</td><td>95</td><td>-</td><td>PB8</td><td>I/O</td><td>FT</td><td>PB8</td><td>TIM4_CH3(9)</td><td>I2C1_SCL / CANRX</td></tr><tr><td>A4</td><td>B3</td><td>46</td><td>A3</td><td>62</td><td>96</td><td>-</td><td>PB9</td><td>I/O</td><td>FT</td><td>PB9</td><td>TIM4_CH4(9)</td><td>I2C1_SDA/CANTX</td></tr><tr><td>D4</td><td>C3</td><td>-</td><td>-</td><td>-</td><td>97</td><td>-</td><td>PE0</td><td>I/O</td><td>FT</td><td>PE0</td><td>TIM4_ETR</td><td>-</td></tr><tr><td>C4</td><td>A2</td><td>-</td><td>-</td><td>-</td><td>98</td><td>-</td><td>PE1</td><td>I/O</td><td>FT</td><td>PE1</td><td>-</td><td>-</td></tr><tr><td>E5</td><td>D3</td><td>47</td><td>D4</td><td>63</td><td>99</td><td>36</td><td> $V_{SS\_3}$ </td><td>S</td><td>-</td><td> $V_{SS\_3}$ </td><td>-</td><td>-</td></tr><tr><td>F5</td><td>C4</td><td>48</td><td>E4</td><td>64</td><td>100</td><td>1</td><td> $V_{DD\_3}$ </td><td>S</td><td>-</td><td> $V_{DD\_3}$ </td><td>-</td><td>-</td></tr></table>

1. I = input, O = output, S = supply.

2. FT = 5 V tolerant.

3. Function availability depends upon the chosen device. For devices having reduced peripheral counts, it is always the lowe number of peripheral that is included. For example, if a device has only one SPI and two USARTs, they are called SPI1 and USART1 and USART2, respectively. Refer to Table 2.

4. If several peripherals share the same I/O pin, to avoid conflict between these alternate functions only one peripheral should be enabled at a time through the peripheral clock enable bit (in the corresponding RCC peripheral clock enable register).

5. PC13, PC14 and PC15 are supplied through the power switch. Since the switch only sinks a limited amount of current (3 mA), the use of GPIOs PC13 to PC15 in output mode is limited: the speed should not exceed 2 MHz with a maximum load of 30 pF and these IOs must not be used as a current source (e.g. to drive a LED).

6. Main function after the first backup domain power-up. Later on, it depends on the contents of the Backup registers even after reset (because these registers are not reset by the main reset). For details on how to manage these IOs, refer to the Battery backup domain and BKP register description sections in the STM32F10xxx reference manual, available from the STMicroelectronics website: www.st.com.

7. The pins number 2 and 3 in the VFQFPN36 package, 5 and 6 in the LQFP48, UFQFP48 and LQFP64 packages, and C1 and C2 in the TFBGA64 package are configured as OSC\_IN/OSC\_OUT after reset, however the functionality of PD0 and PD1 can be remapped by software on these pins. For the LQFP100 package, PD0 and PD1 are available by default, so there is no need for remapping. For more details, refer to the Alternate function I/O and debug configuration section in the STM32F10xxx reference manual.

The use of PD0 and PD1 in output mode is limited as they can only be used at 50 MHz in output mode.

8. Unlike in the LQFP64 package, there is no PC3 in the TFBGA64 package. The V<sub>REF+</sub> functionality is provided instead.

9. This alternate function can be remapped by software to some other port pins (if available on the used package). For more details, refer to the Alternate function I/O and debug configuration section in the STM32F10xxx reference manual, available from the STMicroelectronics website: www.st.com.

## Memory mapping

The memory map is shown in Figure 11.

Figure 11. Memory map  
![](images/5d5b67d4d87dd76b190c29e47ecff7e96da75df7a01645033619691d3b004f48.jpg)

# Electrical characteristics

## 5.1 Parameter conditions

Unless otherwise specified, all voltages are referenced to $\mathsf { V } _ { \mathsf { S S } }$

## 5.1.1 Minimum and maximum values

Unless otherwise specified the minimum and maximum values are guaranteed in the worst conditions of ambient temperature, supply voltage and frequencies by tests in production on 100% of the devices with an ambient temperature at $\bar { \mathsf { T } } _ { \mathsf { A } } = 2 5 ^ { \circ } \mathsf { C }$ and $\mathsf T _ { \mathsf A } = \mathsf T _ { \mathsf A }$ max (given by the selected temperature range).

Data based on characterization results, design simulation and/or technology characteristics are indicated in the table footnotes and are not tested in production. Based on characterization, the minimum and maximum values refer to sample tests and represent the mean value plus or minus three times the standard deviation (mean ± 3σ).

## 5.1.2 Typical values

Unless otherwise specified, typical data are based on $\mathsf { T } _ { \mathsf { A } } = 2 5 ^ { \circ } \mathsf { C } , \mathsf { V } _ { \mathsf { D D } } = 3 . 3 \mathsf { V }$ (for the $2 \vee \le \vee _ { \mathrm { D D } } \le 3 . 6 \vee$ voltage range). They are given only as design guidelines and are not tested.

Typical ADC accuracy values are determined by characterization of a batch of samples from a standard diffusion lot over the full temperature range, where 95% of the devices have an error less than or equal to the value indicated (mean ± 2σ).

## 5.1.3 Typical curves

Unless otherwise specified, all typical curves are given only as design guidelines and are not tested.

## 5.1.4 Loading capacitor

The loading conditions used for pin parameter measurement are shown in Figure 12.

## 5.1.5 Pin input voltage

The input voltage measurement on a pin of the device is described in Figure 13.

<table><tr><td>Figure 12. Pin loading conditions<img src="images/a6bac392635c35716e4130fece645261c416606c93fc4f269d1c50afdf82f9ac.jpg"/></td><td>Figure 13. Pin input voltage<img src="images/47d52dcabc055f9a379cf60595bc73393cb15b7b43d5330bbaa52d28afd49baa.jpg"/></td></tr></table>

## 5.1.6 Power supply scheme

Figure 14. Power supply scheme  
![](images/a59d9bd3f2f4e4a36c1114562faba16859e996356f7b722fa88a8fc69046c1ed.jpg)  
Caution: In Figure 14, the 4.7 µF capacitor must be connected to V<sub>DD3</sub>.

## 5.1.7 Current consumption measurement

Figure 15. Current consumption measurement scheme  
![](images/40de5e9c9022046bea39eb770e9765a5fdf1e7c6b255a09c5581e79cb508e853.jpg)

## 5.2 Absolute maximum ratings

Stresses above the absolute maximum ratings listed in Table 6, Table 7, and Table 8 may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these conditions is not implied. Exposure to maximum rating conditions for extended periods may affect device reliability.

Table 6. Voltage characteristics

<table><tr><td>Symbol</td><td>Ratings</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td> $V_{DD} - V_{SS}$ </td><td>External main supply voltage (including  $V_{DDA}$  and  $V_{DD}$ ) $^{(1)}$ </td><td>-0.3</td><td>4.0</td><td rowspan="3">V</td></tr><tr><td rowspan="2"> $V_{IN}^{(2)}$ </td><td>Input voltage on 5 V tolerant pin</td><td> $V_{SS} - 0.3$ </td><td> $V_{DD} + 4.0$ </td></tr><tr><td>Input voltage on any other pin</td><td> $V_{SS} - 0.3$ </td><td>4.0</td></tr><tr><td> $|\Delta V_{DDx}|$ </td><td>Variations between different  $V_{DD}$  power pins</td><td>-</td><td>50</td><td rowspan="2">mV</td></tr><tr><td> $|V_{SSX} - V_{SS}|$ </td><td>Variations between all the different ground pins</td><td>-</td><td>50</td></tr><tr><td> $V_{ESD(HBM)}$ </td><td>Electrostatic discharge voltage (human body model)</td><td colspan="2">See Section 5.3.11</td><td></td></tr></table>

1. All main power $( \mathsf { V } _ { \mathsf { D } \mathsf { P } } , \mathsf { V } _ { \mathsf { D } \mathsf { D } \mathsf { A } } )$ and ground $( \mathsf { V } _ { \mathsf { S S } } , \mathsf { V } _ { \mathsf { S S A } } )$ pins must always be connected to the external power supply, in the permitted range.  
2. $\mathsf { V } _ { \mathsf { I N } }$ maximum must always be respected. Refer to Table 7 for the maximum allowed injected current values.

Table 7. Current characteristics

<table><tr><td>Symbol</td><td>Ratings</td><td>Max.</td><td>Unit</td></tr><tr><td> $I_{VDD}$ </td><td>Total current into  $V_{DD}/V_{DDA}$  power lines (source) $^{(1)}$ </td><td>150</td><td rowspan="7">mA</td></tr><tr><td> $I_{VSS}$ </td><td>Total current out of  $V_{SS}$  ground lines (sink) $^{(1)}$ </td><td>150</td></tr><tr><td rowspan="2"> $I_{IO}$ </td><td>Output current sunk by any I/O and control pin</td><td>25</td></tr><tr><td>Output current source by any I/Os and control pin</td><td>-25</td></tr><tr><td rowspan="2"> $I_{INJ(PIN)}^{(2)}$ </td><td>Injected current on five volt tolerant pins $^{(3)}$ </td><td>-5/+0</td></tr><tr><td>Injected current on any other pin $^{(4)}$ </td><td>± 5</td></tr><tr><td> $\Sigma I_{INJ(PIN)}$ </td><td>Total injected current (sum of all I/O and control pins) $^{(5)}$ </td><td>± 25</td></tr></table>

1. All main power $( \mathsf { V } _ { \mathsf { D } \mathsf { D } } , \mathsf { V } _ { \mathsf { D } \mathsf { D } \mathsf { A } } )$ and ground $( \mathsf { V } _ { \mathsf { S S } } , \mathsf { V } _ { \mathsf { S S A } } )$ pins must always be connected to the external power supply, in the permitted range.  
2. Negative injection disturbs the analog performance of the device. See footnote 2 of Table 49.  
3. Positive injection is not possible on these I/Os. A negative injection is induced by $\mathsf { V } _ { \mathsf { I N } } { \mathsf { < } } \mathsf { V } _ { \mathsf { S } \mathsf { S } } .$ . I<sub>INJ(PIN)</sub> must never be exceeded. Refer to Table 6 for the maximum allowed input voltage values.  
4. A positive injection is induced by $\mathsf { V } _ { \perp \mathsf { N } } > \mathsf { V } _ { \mathsf { D } \mathsf { D } }$ , while a negative injection is induced $\sf { b } \sf { y } \vee \ubscript { | N } { < } \sf { V } _ { S S }$ I<sub>INJ(PIN)</sub> must never be exceeded. Refer to Table 6 for the maximum allowed input voltage values.  
5. When several inputs are submitted to a current injection, the maximum $\Sigma \mathsf { I } _ { \mathsf { I N J ( P | N ) } }$ is the absolute sum of the positive and negative injected currents (instantaneous values).

Table 8. Thermal characteristics

<table><tr><td>Symbol</td><td>Ratings</td><td>Value</td><td>Unit</td></tr><tr><td> $T_{STG}$ </td><td>Storage temperature range</td><td>-65 to +150</td><td rowspan="2">°C</td></tr><tr><td> $T_J$ </td><td>Maximum junction temperature</td><td>150</td></tr></table>

## 5.3 Operating conditions

## 5.3.1 General operating conditions

Table 9. General operating conditions

<table><tr><td>Symbol</td><td>Parameter</td><td colspan="2">Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{HCLK}$ </td><td>Internal AHB clock frequency</td><td colspan="2">-</td><td>0</td><td>72</td><td rowspan="3">MHz</td></tr><tr><td> $f_{PCLK1}$ </td><td>Internal APB1 clock frequency</td><td colspan="2">-</td><td>0</td><td>36</td></tr><tr><td> $f_{PCLK2}$ </td><td>Internal APB2 clock frequency</td><td colspan="2">-</td><td>0</td><td>72</td></tr><tr><td> $V_{DD}$ </td><td>Standard operating voltage</td><td colspan="2">-</td><td>2</td><td>3.6</td><td rowspan="4">V</td></tr><tr><td rowspan="2"> $\mathsf{V}_{\mathsf{DDA}}^{(1)}$ </td><td>Analog operating voltage (ADC not used)</td><td rowspan="2" colspan="2">Must be the same potential as  $V_{DD}^{(2)}$ </td><td>2</td><td>3.6</td></tr><tr><td>Analog operating voltage (ADC used)</td><td>2.4</td><td>3.6</td></tr><tr><td> $V_{BAT}$ </td><td>Backup operating voltage</td><td colspan="2">-</td><td>1.8</td><td>3.6</td></tr><tr><td rowspan="4"> $V_{IN}$ </td><td rowspan="4">I/O input voltage</td><td colspan="2">Standard IO</td><td>-0.3</td><td> $V_{DD}^{+}$ 0.3</td><td rowspan="4">V</td></tr><tr><td rowspan="2">FT IO(3)</td><td>2 V &lt;  $V_{DD}$  ≤ 3.6 V</td><td>-0.3</td><td>5.5</td></tr><tr><td> $V_{DD}$ =2 V</td><td>-0.3</td><td>5.2</td></tr><tr><td colspan="2">BOOT0</td><td>0</td><td>5.5</td></tr><tr><td rowspan="8"> $P_D$ </td><td rowspan="8">Power dissipation at  $T_A$ =85 °C for suffix 6 or  $T_A$ =105 °C for suffix 7(4)</td><td colspan="2">LFBGA100</td><td>-</td><td>454</td><td rowspan="8">mW</td></tr><tr><td colspan="2">LQFP100</td><td>-</td><td>434</td></tr><tr><td colspan="2">UFBGA100</td><td>-</td><td>339</td></tr><tr><td colspan="2">TFBGA64</td><td>-</td><td>308</td></tr><tr><td colspan="2">LQFP64</td><td>-</td><td>444</td></tr><tr><td colspan="2">LQFP48</td><td>-</td><td>363</td></tr><tr><td colspan="2">UFQFPN48</td><td>-</td><td>624</td></tr><tr><td colspan="2">VFQFPN36</td><td>-</td><td>1000</td></tr><tr><td rowspan="4"> $T_A$ </td><td rowspan="2">Ambient temperature for 6 suffix version</td><td colspan="2">Maximum power dissipation</td><td>-40</td><td>85</td><td rowspan="6">°C</td></tr><tr><td colspan="2">Low-power dissipation(5)</td><td>-40</td><td>105</td></tr><tr><td rowspan="2">Ambient temperature for 7 suffix version</td><td colspan="2">Maximum power dissipation</td><td>-40</td><td>105</td></tr><tr><td colspan="2">Low-power dissipation(5)</td><td>-40</td><td>125</td></tr><tr><td rowspan="2"> $T_J$ </td><td rowspan="2">Junction temperature range</td><td colspan="2">6 suffix version</td><td>-40</td><td>105</td></tr><tr><td colspan="2">7 suffix version</td><td>-40</td><td>125</td></tr></table>

1. When the ADC is used, refer to Table 47.  
2. It is recommended to power $\mathsf { V } _ { \mathsf { D D } }$ and $\mathsf { V } _ { \mathsf { D D A } }$ from the same source. A maximum difference of 300 mV between $\mathsf { V } _ { \mathsf { D D } }$ and $\mathsf { V } _ { \mathsf { D D A } }$ can be tolerated during power-up and operation.  
3. To sustain a voltage higher than $\mathsf { V } _ { \mathsf { D D } } + 0 . 3 \mathsf { V } ,$ the internal pull-up/pull-down resistors must be disabled.  
4. $\mathsf { I f } \mathsf { T } _ { \mathsf { A } }$ is lower, higher $\mathsf { P } _ { \mathsf { D } }$ values are allowed as long as $\mathsf { T } _ { \mathsf { J } }$ does not exceed $\tau _ { \mathrm { { J } } } \mathsf { m a x }$ (see Section 6.10).  
5. In low-power dissipation state, $\mathsf { T } _ { \mathsf { A } }$ can be extended to this range as long as ${ \sf T } _ { \sf J }$ does not exceed T<sub>J</sub>max (see Section 6.10).

## 5.3.2 Operating conditions at power-up / power-down

Subject to general operating conditions for ${ \mathsf { T } } _ { \mathsf { A } } .$

Table 10. Operating conditions at power-up / power-down

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="2"> $t_{VDD}$ </td><td> $V_{DD}$  rise time rate</td><td rowspan="2">-</td><td>0</td><td>∞</td><td rowspan="2">μs/V</td></tr><tr><td> $V_{DD}$  fall time rate</td><td>20</td><td>∞</td></tr></table>

## 5.3.3 Embedded reset and power control block characteristics

The parameters given in Table 11 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 11. Embedded reset and power control block characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="16"> $V_{PVD}$ </td><td rowspan="16">Programmable voltage detector level selection</td><td>PLS[2:0] = 000 (rising edge)</td><td>2.10</td><td>2.18</td><td>2.26</td><td rowspan="16">V</td></tr><tr><td>PLS[2:0] = 000 (falling edge)</td><td>2,00</td><td>2.08</td><td>2.16</td></tr><tr><td>PLS[2:0] = 001 (rising edge)</td><td>2.19</td><td>2.28</td><td>2.37</td></tr><tr><td>PLS[2:0] = 001 (falling edge)</td><td>2.09</td><td>2.18</td><td>2.27</td></tr><tr><td>PLS[2:0] = 010 (rising edge)</td><td>2.28</td><td>2.38</td><td>2.48</td></tr><tr><td>PLS[2:0] = 010 (falling edge)</td><td>2.18</td><td>2.28</td><td>2.38</td></tr><tr><td>PLS[2:0] = 011 (rising edge)</td><td>2.38</td><td>2.48</td><td>2.58</td></tr><tr><td>PLS[2:0] = 011 (falling edge)</td><td>2.28</td><td>2.38</td><td>2.48</td></tr><tr><td>PLS[2:0] = 100 (rising edge)</td><td>2.47</td><td>2.58</td><td>2.69</td></tr><tr><td>PLS[2:0] = 100 (falling edge)</td><td>2.37</td><td>2.48</td><td>2.59</td></tr><tr><td>PLS[2:0] = 101 (rising edge)</td><td>2.57</td><td>2.68</td><td>2.79</td></tr><tr><td>PLS[2:0] = 101 (falling edge)</td><td>2.47</td><td>2.58</td><td>2.69</td></tr><tr><td>PLS[2:0] = 110 (rising edge)</td><td>2.66</td><td>2.78</td><td>2.90</td></tr><tr><td>PLS[2:0] = 110 (falling edge)</td><td>2.56</td><td>2.68</td><td>2.80</td></tr><tr><td>PLS[2:0] = 111 (rising edge)</td><td>2.76</td><td>2.88</td><td>3.00</td></tr><tr><td>PLS[2:0] = 111 (falling edge)</td><td>2.66</td><td>2.78</td><td>2.90</td></tr><tr><td> $V_{PVDhyst}^{(2)}$ </td><td>PVD hysteresis</td><td>-</td><td>-</td><td>100</td><td>-</td><td>mV</td></tr><tr><td rowspan="2"> $V_{POR/PDR}$ </td><td rowspan="2">Power on/power down reset threshold</td><td>Falling edge</td><td> $1.8^{(1)}$ </td><td>1.88</td><td>1.96</td><td rowspan="2">V</td></tr><tr><td>Rising edge</td><td>1.84</td><td>1.92</td><td>2.0</td></tr><tr><td> $V_{PDRhyst}^{(2)}$ </td><td>PDR hysteresis</td><td>-</td><td>-</td><td>40</td><td>-</td><td>mV</td></tr><tr><td> $T_{RSTTEMPO}^{(2)}$ </td><td>Reset temporization</td><td>-</td><td>1.0</td><td>2.5</td><td>4.5</td><td>ms</td></tr></table>

1. The product behavior is specified by design down to the minimum V<sub>POR/PDR</sub> value.  
2. Specified by design, not tested in production.

## 5.3.4 Embedded reference voltage

The parameters given in Table 12 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 12. Embedded internal reference voltage

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="2"> $V_{REFINT}$ </td><td rowspan="2">Internal reference voltage</td><td>-40 °C &lt;  $T_A$ &lt; +105 °C</td><td>1.16</td><td>1.20</td><td>1.26</td><td rowspan="2">V</td></tr><tr><td>-40 °C &lt;  $T_A$ &lt; +85 °C</td><td>1.16</td><td>1.20</td><td>1.24</td></tr><tr><td> $T_{S\_vrefint}^{(1)}$ </td><td>ADC sampling time when reading the internal reference voltage</td><td>-</td><td>-</td><td>5.1</td><td>17.1(2)</td><td>μs</td></tr><tr><td> $V_{RERINT}^{(2)}$ </td><td>Internal reference voltage spread over the temperature range</td><td> $V_{DD}$ = 3 V ±10 mV</td><td>-</td><td>-</td><td>10</td><td>mV</td></tr><tr><td> $T_{Coeff}^{(2)}$ </td><td>Temperature coefficient</td><td>-</td><td>-</td><td>-</td><td>100</td><td>ppm/°C</td></tr></table>

1. Shortest sampling time can be determined in the application by multiple iterations.  
2. Specified by design, not tested in production.

## 5.3.5 Supply current characteristics

The current consumption is a function of several parameters and factors such as operating voltage, ambient temperature, I/O pin loading, device software configuration, operating frequencies, I/O pin switching rate, program location in memory, and executed binary code.

The current consumption is measured as described in Figure 15.

All Run-mode current consumption measurements given in this section are performed with a reduced code that gives a consumption equivalent to Dhrystone 2.1 code.

## Maximum current consumption

The MCU is placed under the following conditions:

• All I/O pins are in input mode with a static value at $\mathsf { V } _ { \mathsf { D D } }$ or $\mathsf { V } _ { \mathsf { S S } }$ (no load)

• All peripherals are disabled except when explicitly mentioned

The flash memory access time is adjusted to the $\mathsf { f } _ { \mathsf { H C L K } }$ frequency (0 wait state from 0 to 24 MHz, one wait state from 24 to 48 MHz and two wait states above)

Prefetch in ON (reminder: this bit must be set before clock setting and bus prescaling)

• When the peripherals are enabled $\mathsf { f } _ { \mathsf { P C L K } 1 } = \mathsf { f } _ { \mathsf { H C L K } } / 2 , \mathsf { f } _ { \mathsf { P C L K } 2 } = \mathsf { f } _ { \mathsf { H C L K } }$

The parameters given in Table 13, Table 14 and Table 15 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 13. Maximum current consumption in Run mode, code with data processing running from Flash

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td rowspan="2"> $f_{HCLK}$ </td><td colspan="2">Max(1)</td><td rowspan="2">Unit</td></tr><tr><td> $T_A=85°C$ </td><td> $T_A=105°C$ </td></tr><tr><td rowspan="12"> $I_{DD}$ </td><td rowspan="12">Supply current in Run mode</td><td rowspan="6">External clock(2), all peripherals enabled</td><td>72 MHz</td><td>50.0</td><td>50.3</td><td rowspan="12">mA</td></tr><tr><td>48 MHz</td><td>36.1</td><td>36.2</td></tr><tr><td>36 MHz</td><td>28.6</td><td>28.7</td></tr><tr><td>24 MHz</td><td>19.9</td><td>20.1</td></tr><tr><td>16 MHz</td><td>14.7</td><td>14.9</td></tr><tr><td>8 MHz</td><td>8.6</td><td>8.9</td></tr><tr><td rowspan="6">External clock(2), all peripherals disabled</td><td>72 MHz</td><td>32.8</td><td>32.9</td></tr><tr><td>48 MHz</td><td>24.4</td><td>24.5</td></tr><tr><td>36 MHz</td><td>19.8</td><td>19.9</td></tr><tr><td>24 MHz</td><td>13.9</td><td>14.2</td></tr><tr><td>16 MHz</td><td>10.7</td><td>11.0</td></tr><tr><td>8 MHz</td><td>6.8</td><td>7.1</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. External clock is 8 MHz and PLL is on when $\mathsf { f } _ { \mathsf { H C L K } } > 8$ MHz.

Table 14. Maximum current consumption in Run mode, code with data processing running from RAM

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td rowspan="2"> $f_{HCLK}$ </td><td colspan="2">Max(1)</td><td rowspan="2">Unit</td></tr><tr><td> $T_A=85°C$ </td><td> $T_A=105°C$ </td></tr><tr><td rowspan="12"> $I_{DD}$ </td><td rowspan="12">Supply current in Run mode</td><td rowspan="6">External clock(2), all peripherals enabled</td><td>72 MHz</td><td>48</td><td>50</td><td rowspan="12">mA</td></tr><tr><td>48 MHz</td><td>31.5</td><td>32</td></tr><tr><td>36 MHz</td><td>24</td><td>25.5</td></tr><tr><td>24 MHz</td><td>17.5</td><td>18</td></tr><tr><td>16 MHz</td><td>12.5</td><td>13</td></tr><tr><td>8 MHz</td><td>7.5</td><td>8</td></tr><tr><td rowspan="6">External clock(2), all peripherals disabled</td><td>72 MHz</td><td>29</td><td>29.5</td></tr><tr><td>48 MHz</td><td>20.5</td><td>21</td></tr><tr><td>36 MHz</td><td>16</td><td>16.5</td></tr><tr><td>24 MHz</td><td>11.5</td><td>12</td></tr><tr><td>16 MHz</td><td>8.5</td><td>9</td></tr><tr><td>8 MHz</td><td>5.5</td><td>6</td></tr></table>

1. Based on characterization, tested in production at $\mathsf { V } _ { \mathsf { D D } }$ max, f<sub>HCLK</sub> max.  
2. External clock is 8 MHz and PLL is on when $\mathsf { f } _ { \mathsf { H C L K } } > 8 \mathsf { M H z } .$

Figure 16. Typical current consumption in Run mode versus frequency (at 3.6 V), code with data processing running from RAM, peripherals enabled  
![](images/d399f0618f6cb80ec8d22565aac4fe061a63645fe340d351110171d3e2821ccd.jpg)

Figure 17. Typical current consumption in Run mode versus frequency (at 3.6 V), code with data processing running from RAM, peripherals disabled  
![](images/3c85b395435010647deeb3d46fe4b97db9f47633b6ee3cab397babc6cf097709.jpg)

Table 15. Maximum current consumption in Sleep mode, code running from Flash or RAM

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td rowspan="2"> $f_{HCLK}$ </td><td colspan="2">Max(1)</td><td rowspan="2">Unit</td></tr><tr><td> $T_A=85°C$ </td><td> $T_A=105°C$ </td></tr><tr><td rowspan="12"> $I_{DD}$ </td><td rowspan="12">Supply current in Sleep mode</td><td rowspan="6">External clock(2), all peripherals enabled</td><td>72 MHz</td><td>30</td><td>32</td><td rowspan="12">mA</td></tr><tr><td>48 MHz</td><td>20</td><td>20.5</td></tr><tr><td>36 MHz</td><td>15.5</td><td>16</td></tr><tr><td>24 MHz</td><td>11.5</td><td>12</td></tr><tr><td>16 MHz</td><td>8.5</td><td>9</td></tr><tr><td>8 MHz</td><td>5.5</td><td>6</td></tr><tr><td rowspan="6">External clock(2), all peripherals disabled</td><td>72 MHz</td><td>7.5</td><td>8</td></tr><tr><td>48 MHz</td><td>6</td><td>6.5</td></tr><tr><td>36 MHz</td><td>5</td><td>5.5</td></tr><tr><td>24 MHz</td><td>4.5</td><td>5</td></tr><tr><td>16 MHz</td><td>4</td><td>4.5</td></tr><tr><td>8 MHz</td><td>3</td><td>4</td></tr></table>

1. Based on characterization, tested in production at V , f max with peripherals enabled.  
2. External clock is 8 MHz and PLL is on when $f _ { { \sf H C L K } } > 8 ~ { \sf M H z } .$

Table 16. Typical and maximum current consumptions in Stop and Standby modes

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td colspan="3"> $Typ^{(1)}$ </td><td colspan="2">Max</td><td rowspan="2">Unit</td></tr><tr><td> $V_{DD}/V_{BAT} = 2.0 V$ </td><td> $V_{DD}/V_{BAT} = 2.4 V$ </td><td> $V_{DD}/V_{BAT} = 3.3 V$ </td><td> $T_A = 85 °C$ </td><td> $T_A = 105 °C$ </td></tr><tr><td rowspan="5"> $I_{DD}$ </td><td rowspan="2">Supply current in Stop mode</td><td>Regulator in Run mode, low-speed and high-speed internal RC oscillators and high-speed oscillator OFF (no independent watchdog)</td><td>-</td><td>23.5</td><td>24</td><td>200</td><td>370</td><td rowspan="6">μA</td></tr><tr><td>Regulator in Low-power mode, low-speed and high-speed internal RC oscillators and high-speed oscillator OFF (no independent watchdog)</td><td>-</td><td>13.5</td><td>14</td><td>180</td><td>340</td></tr><tr><td rowspan="3">Supply current in Standby mode</td><td>Low-speed internal RC oscillator and independent watchdog ON</td><td>-</td><td>2.6</td><td>3.4</td><td>-</td><td>-</td></tr><tr><td>Low-speed internal RC oscillator ON, independent watchdog OFF</td><td>-</td><td>2.4</td><td>3.2</td><td>-</td><td>-</td></tr><tr><td>Low-speed internal RC oscillator and independent watchdog OFF, low-speed oscillator and RTC OFF</td><td>-</td><td>1.7</td><td>2</td><td>4</td><td>5</td></tr><tr><td> $I_{DD\_VBAT}$ </td><td>Backup domain supply current</td><td>Low-speed oscillator and RTC ON</td><td>0.9</td><td>1.1</td><td>1.4</td><td> $1.9^{(2)}$ </td><td>2.2</td></tr></table>

1. Typical values are measured at ${ \sf T } _ { \sf A } = 2 5 ^ { \circ } { \sf C } .$  
2. Evaluated by characterization, not tested in production, unless otherwise specified.

Figure 18. Typical current consumption on $v _ { B A T }$ (RTC on)  
![](images/41ddd92612435ac32e715fd775c52c97db7fd3bab13a44dfcb86079659fd0316.jpg)

Figure 19. Typical current consumption in Stop mode, with regulator in Run mode  
![](images/551f7a5084d2be326971fbce562b4de1c1e0941234a9ccb51005cc926b14045f.jpg)  
Figure 20. Typical current consumption in Stop mode, with regulator in Low-power mode

![](images/e3d39356bfb64bd3b20bed22e3998ce822765518ac06f8af99dff961c4fcc4cd.jpg)

Figure 21. Typical current consumption in Standby mode  
![](images/c271dfae5904fcd2751efb243a1acd24201305d30b30c54b5b049d88b19b6bce.jpg)

## Typical current consumption

The MCU is placed under the following conditions:

• All I/O pins are in input mode with a static value at $\mathsf { V } _ { \mathsf { D D } }$ or $\mathsf { V } _ { \mathsf { S S } }$ (no load)

• All peripherals are disabled except if explicitly mentioned

The flash access time is adjusted to $\mathsf { f } _ { \mathsf { H C L K } }$ frequency (0 wait state from 0 to 24 MHz, one wait state from 24 to 48 MHz and two wait states above)

• Ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9

• Prefetch is ON (this bit must be set before clock setting and bus prescaling)

• When the peripherals are enabled $\mathsf { f } _ { \mathsf { P C L K } 1 } = \mathsf { f } _ { \mathsf { H C L K } } / 4 , \mathsf { f } _ { \mathsf { P C L K } 2 } = \mathsf { f } _ { \mathsf { H C L K } } / 2 ,$ $\mathsf { f } _ { \mathsf { A D C C L K } } = \mathsf { f } _ { \mathsf { P C L K } 2 } / 4$

Table 17. Typical current consumption in Run mode, code with data processing running from Flash

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td rowspan="2"> $f_{HCLK}$ </td><td colspan="2"> $\text{Typ}^{(1)}$ </td><td rowspan="2">Unit</td></tr><tr><td>All peripherals enabled $^{(2)}$ </td><td>All peripherals disabled</td></tr><tr><td rowspan="22"> $I_{DD}$ </td><td rowspan="22">Supply current in Run mode</td><td rowspan="11">External clock $^{(3)}$ </td><td>72 MHz</td><td>36</td><td>27</td><td rowspan="11">mA</td></tr><tr><td>48 MHz</td><td>24.2</td><td>18.6</td></tr><tr><td>36 MHz</td><td>19.0</td><td>14.8</td></tr><tr><td>24 MHz</td><td>12.9</td><td>10.1</td></tr><tr><td>16 MHz</td><td>9.3</td><td>7.4</td></tr><tr><td>8 MHz</td><td>5.5</td><td>4.6</td></tr><tr><td>4 MHz</td><td>3.3</td><td>2.8</td></tr><tr><td>2 MHz</td><td>2.2</td><td>1.9</td></tr><tr><td>1 MHz</td><td>1.6</td><td>1.45</td></tr><tr><td>500 kHz</td><td>1.3</td><td>1.25</td></tr><tr><td>125 kHz</td><td>1.08</td><td>1.06</td></tr><tr><td rowspan="11">Running on high speed internal RC (HSI), AHB prescaler used to reduce the frequency</td><td>64 MHz</td><td>31.4</td><td>23.9</td><td rowspan="11">mA</td></tr><tr><td>48 MHz</td><td>23.5</td><td>17.9</td></tr><tr><td>36 MHz</td><td>18.3</td><td>14.1</td></tr><tr><td>24 MHz</td><td>12.2</td><td>9.5</td></tr><tr><td>16 MHz</td><td>8.5</td><td>6.8</td></tr><tr><td>8 MHz</td><td>4.9</td><td>4.0</td></tr><tr><td>4 MHz</td><td>2.7</td><td>2.2</td></tr><tr><td>2 MHz</td><td>1.6</td><td>1.4</td></tr><tr><td>1 MHz</td><td>1.02</td><td>0.9</td></tr><tr><td>500 kHz</td><td>0.73</td><td>0.67</td></tr><tr><td>125 kHz</td><td>0.5</td><td>0.48</td></tr></table>

1. Typical values are measures at $\mathsf { T } _ { \mathsf { A } } = 2 5 ^ { \circ } \mathsf { C } , \mathsf { V } _ { \mathsf { D D } } = 3 . 3 \mathsf { V } .$  
2. Add an additional power consumption of 0.8 mA per ADC for the analog part. In applications, this consumption occurs only while the ADC is on (ADON bit is set in the ADC\_CR2 register).  
3. External clock is 8 MHz and PLL is on when $f _ { { \sf H C L K } } > 8 ~ { \sf M H z }$

Table 18. Typical current consumption in Sleep mode, code running from Flash or RAM

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td rowspan="2"> $f_{HCLK}$ </td><td colspan="2"> $\text{Typ}^{(1)}$ </td><td rowspan="2">Unit</td></tr><tr><td>All peripherals enabled $^{(2)}$ </td><td>All peripherals disabled</td></tr><tr><td rowspan="22"> $I_{DD}$ </td><td rowspan="22">Supply current in Sleep mode</td><td rowspan="11">External clock $^{(3)}$ </td><td>72 MHz</td><td>14.4</td><td>5.5</td><td rowspan="22">mA</td></tr><tr><td>48 MHz</td><td>9.9</td><td>3.9</td></tr><tr><td>36 MHz</td><td>7.6</td><td>3.1</td></tr><tr><td>24 MHz</td><td>5.3</td><td>2.3</td></tr><tr><td>16 MHz</td><td>3.8</td><td>1.8</td></tr><tr><td>8 MHz</td><td>2.1</td><td>1.2</td></tr><tr><td>4 MHz</td><td>1.6</td><td>1.1</td></tr><tr><td>2 MHz</td><td>1.3</td><td>1.0</td></tr><tr><td>1 MHz</td><td>1.11</td><td>0.98</td></tr><tr><td>500 kHz</td><td>1.04</td><td>0.96</td></tr><tr><td>125 kHz</td><td>0.98</td><td>0.95</td></tr><tr><td rowspan="11">Running on high speed internal RC (HSI), AHB prescaler used to reduce the frequency</td><td>64 MHz</td><td>12.3</td><td>4.4</td></tr><tr><td>48 MHz</td><td>9.3</td><td>3.3</td></tr><tr><td>36 MHz</td><td>7</td><td>2.5</td></tr><tr><td>24 MHz</td><td>4.8</td><td>1.8</td></tr><tr><td>16 MHz</td><td>3.2</td><td>1.2</td></tr><tr><td>8 MHz</td><td>1.6</td><td>0.6</td></tr><tr><td>4 MHz</td><td>1.0</td><td>0.5</td></tr><tr><td>2 MHz</td><td>0.72</td><td>0.47</td></tr><tr><td>1 MHz</td><td>0.56</td><td>0.44</td></tr><tr><td>500 kHz</td><td>0.49</td><td>0.42</td></tr><tr><td>125 kHz</td><td>0.43</td><td>0.41</td></tr></table>

1. Typical values are measures at $\mathsf { T } _ { \mathsf { A } } = 2 5 ^ { \circ } \mathsf { C } , \mathsf { V } _ { \mathsf { D D } } = 3 . 3 \mathsf { V } .$  
2. Add an additional power consumption of 0.8 mA per ADC for the analog part. In applications, this consumption occurs only while the ADC is on (ADON bit is set in the ADC\_CR2 register).  
3. External clock is 8 MHz and PLL is on when f<sub>HCLK</sub> > 8 MHz.

## On-chip peripheral current consumption

The current consumption of the on-chip peripherals is given in Table 19. The MCU is put under the following conditions:

• all I/O pins are in input mode with a static value at $\mathsf { V } _ { \mathsf { D D } }$ or $\mathsf { V } _ { \mathsf { S S } }$ (no load)

• all peripherals are disabled unless otherwise mentioned

• the given value is calculated by measuring the current consumption

– with all peripherals clocked off

– with only one peripheral clocked on

ambient operating temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 6

Table 19. Peripheral current consumption

<table><tr><td colspan="2">Peripherals</td><td>μA/MHz</td></tr><tr><td rowspan="2">AHB (up to 72 MHz)</td><td>DMA1</td><td>16.53</td></tr><tr><td>BusMatrix(1)</td><td>8.33</td></tr><tr><td rowspan="15">APB1 (up to 36 MHz)</td><td>APB1-Bridge</td><td>10.28</td></tr><tr><td>TIM2</td><td>32.50</td></tr><tr><td>TIM3</td><td>31.39</td></tr><tr><td>TIM4</td><td>31.94</td></tr><tr><td>SPI2</td><td>4.17</td></tr><tr><td>USART2</td><td>12.22</td></tr><tr><td>USART3</td><td>12.22</td></tr><tr><td>I2C1</td><td>10.00</td></tr><tr><td>I2C2</td><td>10.00</td></tr><tr><td>USB</td><td>17.78</td></tr><tr><td>CAN1</td><td>18.06</td></tr><tr><td>WWDG</td><td>2.50</td></tr><tr><td>PWR</td><td>1.67</td></tr><tr><td>BKP</td><td>2.50</td></tr><tr><td>IWDG</td><td>11.67</td></tr><tr><td rowspan="11">APB2 (up to 72 MHz)</td><td>APB2-Bridge</td><td>3.75</td></tr><tr><td>GPIOA</td><td>6.67</td></tr><tr><td>GPIOB</td><td>6.53</td></tr><tr><td>GPIOC</td><td>6.53</td></tr><tr><td>GPIOD</td><td>6.53</td></tr><tr><td>GPIOE</td><td>6.39</td></tr><tr><td>SPI1</td><td>4.72</td></tr><tr><td>USART1</td><td>11.94</td></tr><tr><td>TIM1</td><td>23.33</td></tr><tr><td>ADC1(2)</td><td>17.50</td></tr><tr><td>ADC2(2)</td><td>16.07</td></tr></table>

1. The BusMatrix is automatically active when at least one master peripheral is ON (CPU or DMA).  
2. Specific conditions for measuring ADC current consumption: $f _ { \mathrm { H C L K } } = 5 6 ~ \mathsf { M H } z ,$ $\mathsf { f } _ { \mathsf { A P B 1 } } = \mathsf { f } _ { \mathsf { H C L K } } / 2 ,$ f<sub>APB2</sub> = f<sub>HCLK</sub>, f<sub>ADCCLK</sub> = f<sub>APB2</sub> / 4, When ADON bit in the ADCx\_CR2 register is set to 1, a current consumption of analog part equal to 0.65 mA must be added for each ADC.

## 5.3.6 External clock source characteristics

## High-speed external user clock generated from an external source

The characteristics given in Table 20 result from tests performed using a high-speed external clock source, and under ambient temperature and supply voltage conditions summarized in Table 9.

Table 20. High-speed external user clock characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{HSE\_ext}$ </td><td>User external clock source frequency(1)</td><td rowspan="5">-</td><td>1</td><td>8</td><td>25</td><td>MHz</td></tr><tr><td> $V_{HSEH}$ </td><td>OSC_IN input pin high level voltage</td><td>0.7 $V_{DD}$ </td><td>-</td><td> $V_{DD}$ </td><td rowspan="2">V</td></tr><tr><td> $V_{HSEL}$ </td><td>OSC_IN input pin low level voltage</td><td> $V_{SS}$ </td><td>-</td><td>0.3 $V_{DD}$ </td></tr><tr><td> $t_{w(HSE)}$  $t_{w(HSE)}$ </td><td>OSC_IN high or low time(1)</td><td>5</td><td>-</td><td>-</td><td rowspan="2">ns</td></tr><tr><td> $t_{r(HSE)}$  $t_{f(HSE)}$ </td><td>OSC_IN rise or fall time(1)</td><td>-</td><td>-</td><td>20</td></tr><tr><td> $C_{in(HSE)}$ </td><td>OSC_IN input capacitance(1)</td><td>-</td><td>-</td><td>5</td><td>-</td><td>pF</td></tr><tr><td>DuCy(HSE)</td><td>Duty cycle</td><td>-</td><td>45</td><td>-</td><td>55</td><td>%</td></tr><tr><td> $I_L$ </td><td>OSC_IN Input leakage current</td><td> $V_{SS} \leq V_{IN} \leq V_{DD}$ </td><td>-</td><td>-</td><td>±1</td><td>μA</td></tr></table>

1. Specified by design, not tested in production.

## Low-speed external user clock generated from an external source

The characteristics given in Table 21 result from tests performed using a low-speed external clock source, and under ambient temperature and supply voltage conditions summarized in Table 9.

Table 21. Low-speed external user clock characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{LSE\_ext}$ </td><td>User external clock source frequency(1)</td><td rowspan="5">-</td><td></td><td>32.768</td><td>1000</td><td>kHz</td></tr><tr><td> $V_{LSEH}$ </td><td>OSC32_IN input pin high level voltage</td><td> $0.7V_{DD}$ </td><td>-</td><td> $V_{DD}$ </td><td rowspan="2">V</td></tr><tr><td> $V_{LSEL}$ </td><td>OSC32_IN input pin low level voltage</td><td> $V_{SS}$ </td><td>-</td><td> $0.3V_{DD}$ </td></tr><tr><td> $t_{w(LSE)}$  $t_{w(LSE)}$ </td><td>OSC32_IN high or low time(1)</td><td>450</td><td>-</td><td>-</td><td rowspan="2">ns</td></tr><tr><td> $t_{r(LSE)}$  $t_{f(LSE)}$ </td><td>OSC32_IN rise or fall time(1)</td><td>-</td><td>-</td><td>50</td></tr><tr><td> $C_{in(LSE)}$ </td><td>OSC32_IN input capacitance(1)</td><td>-</td><td>-</td><td>5</td><td>-</td><td>pF</td></tr><tr><td>DuCy(LSE)</td><td>Duty cycle</td><td>-</td><td>30</td><td>-</td><td>70</td><td>%</td></tr><tr><td> $I_L$ </td><td>OSC32_IN Input leakage current</td><td> $V_{SS} \leq V_{IN} \leq V_{DD}$ </td><td>-</td><td>-</td><td>±1</td><td>μA</td></tr></table>

1. Specified by design, not tested in production.

Figure 22. High-speed external clock source AC timing diagram  
![](images/7ea31e239d1800c29fa626845ec23da09bb82c6f9678a50bb9b30f6160dd7f95.jpg)

Figure 23. Low-speed external clock source AC timing diagram  
![](images/9e6c4226292149f93be8ed3b65f9b6e85f30976ef51554684a0616350e8bf1cd.jpg)

## High-speed external clock generated from a crystal/ceramic resonator

The high-speed external (HSE) clock can be supplied with a 4 to 16 MHz crystal/ceramic resonator oscillator. All the information given in this paragraph is based on characterization results obtained with typical external components specified in Table 22. In the application, the resonator and the load capacitors have to be placed as close as possible to the oscillator pins in order to minimize output distortion and startup stabilization time. Refer to the crystal resonator manufacturer for more details on the resonator characteristics (frequency, package, accuracy).

Table 22. HSE 4-16 MHz oscillator characteristics<sup>(1)</sup> <sup>(2)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{OSC\_IN}$ </td><td>Oscillator frequency</td><td>-</td><td>4</td><td>8</td><td>16</td><td>MHz</td></tr><tr><td> $R_F$ </td><td>Feedback resistor</td><td>-</td><td>-</td><td>200</td><td>-</td><td>kΩ</td></tr><tr><td> $i_2$ </td><td>HSE driving current</td><td> $V_{DD} = 3.3\text{V}, V_{IN} = V_{SS}$  with 30 pF load</td><td>-</td><td>-</td><td>1</td><td>mA</td></tr><tr><td> $g_m$ </td><td>Oscillator transconductance</td><td>Startup</td><td>25</td><td>-</td><td>-</td><td>mA/V</td></tr><tr><td> $t_{SU(HSE)}^{(3)}$ </td><td>Startup time</td><td> $V_{DD}$  is stabilized</td><td>-</td><td>2</td><td>-</td><td>ms</td></tr></table>

1. Resonator characteristics given by the crystal/ceramic resonator manufacturer.  
2. Evaluated by characterization, not tested in production, unless otherwise specified.  
3. t<sub>SU(HSE)</sub> is the startu.p time measured from the moment it is enabled (by software) to a stabilized 8 MHz oscillation is reached. This value is measured for a standard crystal resonator and it can vary significantly with the crystal manufacturer

For ${ \mathsf { C } } _ { \mathsf { L } 1 }$ and ${ \mathsf { C } } _ { \mathsf { L } 2 }$ , it is recommended to use high-quality external ceramic capacitors in the 5 pF to 25 pF range (typ.), designed for high-frequency applications, and selected to match the requirements of the crystal or resonator (see Figure 24). ${ \mathsf { C } } _ { \mathsf { L } 1 }$ and $\mathsf { C } _ { \mathsf { L } 2 }$ are usually the same size. The crystal manufacturer typically specifies a load capacitance that is the series combination of ${ \mathsf { C } } _ { \mathsf { L } 1 }$ and ${ \mathsf { C } } _ { \mathsf { L } 2 }$ . PCB and MCU pin capacitance must be included (10 pF can be used as a rough estimate of the combined pin and board capacitance) when sizing ${ \mathsf { C } } _ { \mathsf { L } 1 }$ and

2. Refer to the note and caution paragraphs below the table, and to AN2867 “Oscillator design guide for ST microcontrollers”.

${ \mathsf { C } } _ { \mathsf { L } 2 } .$ . Refer to AN2867 “Oscillator design guide for ST microcontrollers”, available from the STMicroelectronics website www.st.com.

Figure 24. Typical application with an 8 MHz crystal  
![](images/963a1cdda05c376783120187120d5ba2f3de7152a227da498286ebe6eaf611a9.jpg)  
1. R value depends on the crystal characteristics.

## Low-speed external clock generated from a crystal/ceramic resonator

The low-speed external (LSE) clock can be supplied with a 32.768 kHz crystal/ceramic resonator oscillator. All the information given in this paragraph is based on characterization results obtained with typical external components specified in Table 23. In the application, the resonator and the load capacitors have to be placed as close as possible to the oscillator pins tortion and startup stabilization time. Refer to the crystal resonator manufacturer for more details on the resonator characteristics (frequency, package, accuracy).

Table 23. LSE oscillator characteristics $( \mathbf { f _ { L S E } } = 3 2 . 7 6 8 ~ \mathbf { k H z } ) ^ { ( 1 ) }$ (2)

<table><tr><td>Symbol</td><td>Parameter</td><td colspan="2">Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $R_F$ </td><td>Feedback resistor</td><td colspan="2">-</td><td>-</td><td>5</td><td>-</td><td>MΩ</td></tr><tr><td> $I_2$ </td><td>LSE driving current</td><td colspan="2"> $V_{DD} = 3.3 \text{ V}, V_{IN} = V_{SS}$ </td><td>-</td><td>-</td><td>1.4</td><td>μA</td></tr><tr><td> $g_m$ </td><td>Oscillator transconductance</td><td colspan="2">-</td><td>5</td><td>-</td><td>-</td><td>μA/V</td></tr><tr><td rowspan="8"> $t_{SU(LSE)}^{(3)}$ </td><td rowspan="8">Startup time</td><td rowspan="8"> $V_{DD} \text{ is stabilized}$ </td><td> $T_A = 50 °C$ </td><td>-</td><td>1.5</td><td>-</td><td rowspan="8">s</td></tr><tr><td> $T_A = 25 °C$ </td><td>-</td><td>2.5</td><td>-</td></tr><tr><td> $T_A = 10 °C$ </td><td>-</td><td>4</td><td>-</td></tr><tr><td> $T_A = 0 °C$ </td><td>-</td><td>6</td><td>-</td></tr><tr><td> $T_A = -10 °C$ </td><td>-</td><td>10</td><td>-</td></tr><tr><td> $T_A = -20 °C$ </td><td>-</td><td>17</td><td>-</td></tr><tr><td> $T_A = -30 °C$ </td><td>-</td><td>32</td><td>-</td></tr><tr><td> $T_A = -40 °C$ </td><td>-</td><td>60</td><td>-</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
3. t<sub>SU(LSE)</sub> is the startup time measured from the moment it is enabled (by softwinformation given in this paragraph is) to a stabilized 32.768 kHz oscillation is reached. This value is measured for a standard crystal and it can vary significantly with the crystal manufacturer

Note:

For $\mathsf { C } _ { L 1 }$ and $C _ { L 2 }$ it is recommended to use high-quality ceramic capacitors in the 5 to $1 5 p F$ range selected to match the requirements of the crystal or resonator. $\mathsf { C } _ { L 1 }$ and ${ \cal C } _ { L 2 } ,$ are

usually the same size. The crystal manufacturer typically specifies a load capacitance, which is the series combination of $\mathsf { C } _ { L 1 }$ and $C _ { L 2 } .$

Load capacitance $C _ { L }$ has the following formula: $C _ { L } = { \sf C } _ { \sf L 1 } x { \sf C } _ { \sf L 2 } / ( { \sf C } _ { \sf L 1 } + { \sf C } _ { \sf L 2 } ) + C _ { s t r a v } ,$ where $C _ { s t r a y }$ is the pin capacitance and board or trace PCB-related capacitance. Typically, it is between 2 and $7 p F .$

Caution: To avoid exceeding the maximum value of ${ \mathsf { C } } _ { \mathsf { L } 1 }$ and $\mathsf C _ { \mathsf { L } 2 } \left( 1 5 { \mathsf p } \mathsf F \right)$ it is strongly recommended to use a resonator with a load capacitance $\bar { \mathsf { C } _ { \mathsf { L } } } \mathsf { \leq } 7 ~ \mathsf { p F }$ . Never use a resonator with a load capacitance of 12.5 pF.

Example: when choosing a resonator with a load capacitance of $\mathsf C _ { \mathrm { L } } = 6$ pF and $\mathsf C _ { \mathsf { s t r a y } } = 2 \mathsf { p F }$ , then $\mathsf C _ { \mathsf L 1 } = \mathsf C _ { \mathsf L 2 } = 8 \mathsf p \mathsf F .$

Figure 25. Typical application with a 32.768 kHz crystal  
![](images/0bbedc7175d55424395c4cee087b9e62e92755b86d1f2e5ddb13a38aa0f6537e.jpg)

## 5.3.7 Internal clock source characteristics

The parameters given in Table 24 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

High-speed internal (HSI) RC oscillator

Table 24. HSI oscillator characteristics<sup>(1)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td colspan="2">Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{HSI}$ </td><td>Frequency</td><td colspan="2">-</td><td>-</td><td>8</td><td>-</td><td>MHz</td></tr><tr><td> $DuCy_{(HSI)}$ </td><td>Duty cycle</td><td colspan="2">-</td><td>45</td><td>-</td><td>55</td><td rowspan="6">%</td></tr><tr><td rowspan="5"> $ACC_{HSI}$ </td><td rowspan="5">Accuracy of the HSI oscillator</td><td colspan="2"> $User-trimmed with the RCC\_CR register^{(2)}$ </td><td>-</td><td>-</td><td> $1^{(3)}$ </td></tr><tr><td rowspan="4">Factory-calibrated (4)(5)</td><td> $T_A = -40 to 105 °C$ </td><td>-2</td><td>-</td><td>2.5</td></tr><tr><td> $T_A = -10 to 85 °C$ </td><td>-1.5</td><td>-</td><td>2.2</td></tr><tr><td> $T_A = 0 to 70 °C$ </td><td>-1.3</td><td>-</td><td>2</td></tr><tr><td> $T_A = 25 °C$ </td><td>-1.1</td><td>-</td><td>1.8</td></tr><tr><td> $t_{su(HSI)}^{(4)}$ </td><td>HSI oscillator startup time</td><td colspan="2">-</td><td>1</td><td>-</td><td>2</td><td>μs</td></tr><tr><td> $I_{DD(HSI)}^{(4)}$ </td><td>HSI oscillator power consumption</td><td colspan="2">-</td><td>-</td><td>80</td><td>100</td><td>μA</td></tr></table>

1. $\mathsf { V } _ { \mathsf { D D } } = 3 . 3 \mathsf { V } ,$ $\mathsf { T } _ { \mathsf { A } } = - 4 0$ to 105 °C unless otherwise specified.  
2. Refer to AN2868 “STM32F10xxx internal RC oscillator (HSI) calibration” available from www.st.com.

3. Specified by design, not tested in production.

4. Evaluated by characterization, not tested in production, unless otherwise specified.

5. The actual frequency of HSI oscillator may be impacted by a reflow, but does not drift out of the specified range.

## Low-speed internal (LSI) RC oscillator

Table 25. LSI oscillator characteristics <sup>(1)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $f_{LSI}^{(2)}$ </td><td>Frequency</td><td>30</td><td>40</td><td>60</td><td>kHz</td></tr><tr><td> $t_{su(LSI)}^{(3)}$ </td><td>LSI oscillator startup time</td><td>-</td><td>-</td><td>85</td><td>μs</td></tr><tr><td> $I_{DD(LSI)}^{(3)}$ </td><td>LSI oscillator power consumption</td><td>-</td><td>0.65</td><td>1.2</td><td>μA</td></tr></table>

1. V<sub>DD</sub> = 3 V, $\mathsf { T } _ { \mathsf { A } } = - 4 0$ to 105°C unless otherwise specified.  
2. Evaluated by characterization, not tested in production, unless otherwise specified.  
3. Specified by design, not tested in production.

## Wakeup time from low-power mode

The wakeup times given in Table 26 are measured on a wakeup phase with an 8-MHz HSI RC oscillator. The clock source used to wake up the device depends from the current operating mode:

• Stop or Standby mode: the clock source is the RC oscillator

• Sleep mode: the clock source is the clock that was set before entering Sleep mode.

All timings are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 26. Low-power mode wakeup timings

<table><tr><td>Symbol</td><td>Parameter</td><td>Typ</td><td>Unit</td></tr><tr><td> $t_{WUSLEEP}^{(1)}$ </td><td>Wakeup from Sleep mode</td><td>1.8</td><td rowspan="4">μs</td></tr><tr><td rowspan="2"> $t_{WUSTOP}^{(1)}$ </td><td>Wakeup from Stop mode (regulator in run mode)</td><td>3.6</td></tr><tr><td>Wakeup from Stop mode (regulator in low-power mode)</td><td>5.4</td></tr><tr><td> $t_{WUSTDBY}^{(1)}$ </td><td>Wakeup from Standby mode</td><td>50</td></tr></table>

1. The wakeup times are measured from the wakeup event to the point in which the user application code reads the first instruction.

## 5.3.8 PLL characteristics

The parameters given in Table 27 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 27. PLL characteristics

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td colspan="3">Value</td><td rowspan="2">Unit</td></tr><tr><td> $Min^{(1)}$ </td><td>Typ</td><td> $Max^{(1)}$ </td></tr><tr><td rowspan="2"> $f_{PLL\_IN}$ </td><td>PLL input  $clock^{(2)}$ </td><td>1</td><td>8.0</td><td>25</td><td>MHz</td></tr><tr><td>PLL input clock duty cycle</td><td>40</td><td>-</td><td>60</td><td>%</td></tr><tr><td> $f_{PLL\_OUT}$ </td><td>PLL multiplier output clock</td><td>16</td><td>-</td><td>72</td><td>MHz</td></tr><tr><td> $t_{LOCK}$ </td><td>PLL lock time</td><td>-</td><td>-</td><td>200</td><td>μs</td></tr><tr><td>Jitter</td><td>Cycle-to-cycle jitter</td><td>-</td><td>-</td><td>300</td><td>ps</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. Take care of using the appropriate multiplier factors so as to have PLL input clock values compatible with the range defined by f<sub>PLL\_OUT</sub>.

## 5.3.9 Memory characteristics

## Flash memory

The characteristics are given at $\mathsf { T } _ { \mathsf { A } } = - 4 0 \mathsf { t o } \ 1 0 5 ^ { \circ } \mathsf { C }$ unless otherwise specified.

Table 28. Flash memory characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td> $Min^{(1)}$ </td><td>Typ</td><td> $Max^{(1)}$ </td><td>Unit</td></tr><tr><td> $t_{prog}$ </td><td>16-bit programming time</td><td> $T_A = -40 \text{ to } +105 °C$ </td><td>40</td><td>52.5</td><td>70</td><td>μs</td></tr><tr><td> $t_{ERASE}$ </td><td>Page (1 KB) erase time</td><td> $T_A = -40 \text{ to } +105 °C$ </td><td>20</td><td>-</td><td>40</td><td rowspan="2">ms</td></tr><tr><td> $t_{ME}$ </td><td>Mass erase time</td><td> $T_A = -40 \text{ to } +105 °C$ </td><td>20</td><td>-</td><td>40</td></tr><tr><td rowspan="3"> $I_{DD}$ </td><td rowspan="3">Supply current</td><td>Read mode $f_{HCLK} = 72 MHz with two wait states, V_{DD} = 3.3 V$ </td><td>-</td><td>-</td><td>20</td><td rowspan="2">mA</td></tr><tr><td>Write / Erase modes $f_{HCLK} = 72 MHz, V_{DD} = 3.3 V$ </td><td>-</td><td>-</td><td>5</td></tr><tr><td>Power-down mode / Halt, $V_{DD} = 3.0 to 3.6 V$ </td><td>-</td><td>-</td><td>50</td><td>μA</td></tr><tr><td> $V_{prog}$ </td><td>Programming voltage</td><td>-</td><td>2</td><td>-</td><td>3.6</td><td>V</td></tr></table>

1. Specified by design, not tested in production.

Table 29. Flash memory endurance and data retention

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td rowspan="2">Conditions</td><td colspan="3">Value</td><td rowspan="2">Unit</td></tr><tr><td>Min(1)</td><td>Typ</td><td>Max</td></tr><tr><td> $N_{END}$ </td><td>Endurance</td><td> $T_A = -40 \text{ to } +85 °C (6 \text{ suffix versions})$  $T_A = -40 \text{ to } +105 °C (7 \text{ suffix versions})$ </td><td>10</td><td>-</td><td>-</td><td>kcycles</td></tr><tr><td rowspan="3"> $t_{RET}$ </td><td rowspan="3">Data retention</td><td> $1 \text{ kcycle}^{(2)} \text{ at } T_A = 85 °C$ </td><td>30</td><td>-</td><td>-</td><td rowspan="3">Years</td></tr><tr><td> $1 \text{ kcycle}^{(2)} \text{ at } T_A = 105 °C$ </td><td>10</td><td>-</td><td>-</td></tr><tr><td> $10 \text{ kcycles}^{(2)} \text{ at } T_A = 55 °C$ </td><td>20</td><td>-</td><td>-</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. Cycling performed over the whole temperature range.

## 5.3.10 EMC characteristics

Susceptibility tests are performed on a sample basis during device characterization.

## Functional EMS (electromagnetic susceptibility)

While a simple application is executed on the device (toggling 2 LEDs through I/O ports). the device is stressed by two electromagnetic events until a failure occurs. The failure is indicated by the LEDs:

Electrostatic discharge (ESD) (positive and negative) is applied to all device pins until a functional disturbance occurs. This test is compliant with the IEC 61000-4-2 standard.

FTB: A Burst of Fast Transient voltage (positive and negative) is applied to $\mathsf { V } _ { \mathsf { D D } }$ and $\mathsf { V } _ { \mathsf { S S } }$ through a 100 pF capacitor, until a functional disturbance occurs. This test is compliant with the IEC 61000-4-4 standard.

A device reset allows normal operations to be resumed.

The test results are given in Table 30. They are based on the EMS levels and classes defined in application note AN1709, available on www.st.com.

Table 30. EMS characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Level/Class</td></tr><tr><td> $V_{FESD}$ </td><td>Voltage limits to be applied on any I/O pin to induce a functional disturbance</td><td> $V_{DD} = 3.3 \text{ V, } T_A = +25 \text{ °C}, f_{HCLK} = 72 \text{ MHz}$ conforms to IEC 61000-4-2</td><td>2B</td></tr><tr><td> $V_{EFTB}$ </td><td>Fast transient voltage burst limits to be applied through 100 pF on  $V_{DD}$  and  $V_{SS}$  pins to induce a functional disturbance</td><td> $V_{DD} = 3.3 \text{ V, } T_A = +25 \text{ °C}, f_{HCLK} = 72 \text{ MHz}$ conforms to IEC 61000-4-4</td><td>4A</td></tr></table>

## Designing hardened software to avoid noise problems

EMC characterization and optimization are performed at component level with a typical application environment and simplified MCU software. It should be noted that good EMC performance is highly dependent on the user application and the software in particular.

Therefore it is recommended that the user applies EMC software optimization and prequalification tests in relation with the EMC level requested for his application.

## Software recommendations

The software flow must include the management of runaway conditions such as:

• Corrupted program counter

• Unexpected reset

• Critical data corruption (control registers...)

## Prequalification trials

Most of the common failures (unexpected reset and program counter corruption) can be reproduced by manually forcing a low state on the NRST pin or the Oscillator pins for 1 second.

To complete these trials, ESD stress can be applied directly on the device, over the range of specification values. When unexpected behavior is detected, the software can be hardened to prevent unrecoverable errors occurring (see application note AN1015, available on www.st.com).

## Electromagnetic Interference (EMI)

The electromagnetic field emitted by the device are monitored while a simple application is executed (toggling 2 LEDs through the I/O ports). This emission test is compliant with IEC 61967-2 standard which specifies the test board and the pin loading.

Table 31. EMI characteristics for $\mathsf { f } _ { \mathsf { H S E } } = 8$ MHz and $f _ { { \sf H C L K } } = 4 8$ MHz

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Monitored frequency band</td><td>Value</td><td>Unit</td></tr><tr><td rowspan="4"> $S_{EMI}$ </td><td rowspan="3"> $Peak^{(1)}$ </td><td rowspan="4"> $V_{DD}=3.3V, T_A=25°C, LQFP100 package compliant with IEC 61967-2$ </td><td>0.1 to 30 MHz</td><td>12</td><td rowspan="3">dBμV</td></tr><tr><td>30 to 130 MHz</td><td>22</td></tr><tr><td>130 MHz to 1GHz</td><td>23</td></tr><tr><td> $Level^{(2)}$ </td><td>0.1 MHz to 1GHz</td><td>4</td><td>-</td></tr></table>

1. Refer to AN1709 “EMI radiated test” chapter.  
2. Refer to AN1709 “EMI level classification” chapter.

Table 32. EMI characteristics for $\mathsf { f } _ { \mathsf { H S E } } = 8$ MHz and $\mathtt { f } _ { \mathsf { H C L K } } = 7 2$ MHz

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Monitored frequency band</td><td>Value</td><td>Unit</td></tr><tr><td rowspan="4"> $S_{EMI}$ </td><td rowspan="3"> $Peak^{(1)}$ </td><td rowspan="4"> $V_{DD}=3.3V, T_A=25°C, LQFP100 package compliant with IEC 61967-2$ </td><td>0.1 to 30 MHz</td><td>12</td><td rowspan="3">dBμV</td></tr><tr><td>30 to 130 MHz</td><td>19</td></tr><tr><td>130 MHz to 1GHz</td><td>29</td></tr><tr><td> $Level^{(2)}$ </td><td>0.1 MHz to 1GHz</td><td>4</td><td>-</td></tr></table>

1. Refer to AN1709 “EMI radiated test” chapter.  
2. Refer to AN1709 “EMI level classification” chapter.

## 5.3.11 Absolute maximum ratings (electrical sensitivity)

Based on three different tests (ESD, LU) using specific measurement methods, the device is stressed in order to determine its performance in terms of electrical sensitivity.

## Electrostatic discharge (ESD)

Electrostatic discharges (a positive then a negative pulse separated by 1 second) are applied to the pins of each sample according to each pin combination. The sample size depends on the number of supply pins in the device (3 parts × (n + 1) supply pins). This test conforms to the JESD22-A114/C101 standard.

Table 33. ESD absolute maximum ratings

<table><tr><td>Symbol</td><td>Ratings</td><td>Conditions</td><td>Class</td><td>Maximum value(1)</td><td>Unit</td></tr><tr><td> $V_{ESD(HBM)}$ </td><td>Electrostatic discharge voltage (human body model)</td><td> $T_A = +25 °C$ conforming to JESD22-A114</td><td>2</td><td>2000</td><td rowspan="2">V</td></tr><tr><td> $V_{ESD(CDM)}$ </td><td>Electrostatic discharge voltage (charge device model)</td><td> $T_A = +25 °C$ conforming to ANSI/ESD STM5.3.1</td><td>II</td><td>500</td></tr></table>

1. Guaranteed based on test during characterization

## Static latch-up

Two complementary static tests are required on six parts to assess the latch-up performance:

• A supply overvoltage is applied to each power supply pin

• A current injection is applied to each input, output and configurable I/O pin These tests are compliant with EIA/JESD 78A IC latch-up standard.

Table 34. Electrical sensitivities

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Class</td></tr><tr><td>LU</td><td>Static latch-up class</td><td> $T_A = +105 °C$  conforming to JESD78A</td><td>II level A</td></tr></table>

## 5.3.12 I/O current injection characteristics

As a general rule, current injection to the I/O pins, due to external voltage below V<sub>SS</sub> or above $\mathsf { V } _ { \mathsf { D D } }$ (for standard, 3 V-capable I/O pins) should be avoided during normal product operation. However, in order to give an indication of the robustness of the microcontroller in cases when abnormal injection accidentally happens, susceptibility tests are performed on a sample basis during device characterization.

## Functional susceptibilty to I/O current injection

While a simple application is executed on the device, the device is stressed by injecting current into the I/O pins programmed in floating input mode. While current is injected into the I/O pin, one at a time, the device is checked for functional failures.

The failure is indicated by an out of range parameter: ADC error above a certain limit (>5 LSB TUE), out of spec current injection on adjacent pins or other functional failure (for example reset, oscillator frequency deviation).

The test results are given in Table 35

Table 35. I/O current injection susceptibility

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Description</td><td colspan="2">Functional susceptibility</td><td rowspan="2">Unit</td></tr><tr><td>Negative injection</td><td>Positive injection</td></tr><tr><td rowspan="3"> $I_{INJ}$ </td><td>Injected current on OSC_IN32,OSC_OUT32, PA4, PA5, PC13</td><td>-0</td><td>+0</td><td rowspan="3">mA</td></tr><tr><td>Injected current on all FT pins</td><td>-5</td><td>+0</td></tr><tr><td>Injected current on any other pin</td><td>-5</td><td>+5</td></tr></table>

## 5.3.13 I/O port characteristics

## General input/output characteristics

Unless otherwise specified, the parameters given in Table 36 are derived from tests performed under the conditions summarized in Table 9. All I/Os are CMOS and TTL compliant.

Table 36. I/O static characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="3"> $V_{IL}$ </td><td rowspan="3">Low level input voltage</td><td>Standard IO input low level voltage</td><td>-</td><td>-</td><td>0.28*( $V_{DD}-2V$ )+0.8 $V^{(1)}$ </td><td rowspan="6">V</td></tr><tr><td>IO  $FT^{(3)}$  input low level voltage</td><td>-</td><td>-</td><td>0.32*( $V_{DD}-2V$ )+0.75 $V^{(1)}$ </td></tr><tr><td>All I/Os except BOOT0</td><td>-</td><td>-</td><td>0.35 $V_{DD}^{(2)}$ </td></tr><tr><td rowspan="3"> $V_{IH}$ </td><td rowspan="3">High level input voltage</td><td>Standard IO input high level voltage</td><td>0.41*( $V_{DD}-2V$ )+1.3 $V^{(1)}$ </td><td>-</td><td>-</td></tr><tr><td>IO  $FT^{(3)}$  input high level voltage</td><td>0.42*( $V_{DD}-2V$ )+1 $V^{(1)}$ </td><td>-</td><td>-</td></tr><tr><td>All I/Os except BOOT0</td><td>0.65 $V_{DD}^{(2)}$ </td><td>-</td><td>-</td></tr><tr><td rowspan="2"> $V_{hys}$ </td><td>Standard IO Schmitt trigger voltage hysteresis $^{(4)}$ </td><td>-</td><td>200</td><td>-</td><td>-</td><td rowspan="2">mV</td></tr><tr><td>IO FT Schmitt trigger voltage hysteresis $^{(4)}$ </td><td>-</td><td>5% $V_{DD}^{(5)}$ </td><td>-</td><td>-</td></tr><tr><td rowspan="2"> $I_{Ikg}$ </td><td rowspan="2">Input leakage current $^{(6)}$ </td><td> $V_{SS} \leq V_{IN} \leq V_{DD}$  Standard I/Os</td><td>-</td><td>-</td><td>±1</td><td rowspan="2">μA</td></tr><tr><td> $V_{IN} = 5V$  I/O FT</td><td>-</td><td>-</td><td>3</td></tr><tr><td> $R_{PU}$ </td><td>Weak pull-up equivalent resistor $^{(7)}$ </td><td> $V_{IN} = V_{SS}$ </td><td>30</td><td>40</td><td>50</td><td rowspan="2">kΩ</td></tr><tr><td> $R_{PD}$ </td><td>Weak pull-down equivalent resistor $^{(7)}$ </td><td> $V_{IN} = V_{DD}$ </td><td>30</td><td>40</td><td>50</td></tr><tr><td> $C_{IO}$ </td><td>I/O pin capacitance</td><td></td><td>-</td><td>5</td><td>-</td><td>pF</td></tr></table>

1. Data based on design simulation.  
2. Tested in production.  
3. FT = 5 V tolerant. To sustain a voltage higher than $\mathsf { V } _ { \mathsf { D D } } + 0 . 3 \mathsf { V }$ the internal pull-up/pull-down resistors must be disabled.  
4. Hysteresis voltage between Schmitt trigger switching levels. Evaluated by characterization, not tested in production, unless otherwise specified.  
5. With a minimum of 100 mV.  
6. Leakage can be higher than Max if negative current is injected on adjacent pins.

7. Pull-up and pull-down resistors are designed with a true resistance in series with a switchable PMOS/NMOS. This PMOS/NMOS contribution to the series resistance is minimum (\~10%).

All I/Os are CMOS and TTL compliant (no software configuration required). Their characteristics cover more than the strict CMOS-technology or TTL parameters. The coverage of these requirements is shown in Figure 26 and Figure 27 for standard I/Os, and in Figure 28 and Figure 29 for 5 V tolerant I/Os.

Figure 26. Standard I/O input characteristics - CMOS port  
![](images/7973d9eeb28d64b2e1a4579a727f7a0aed388d9a1fac129bd5d5badd214863db.jpg)

Figure 27. Standard I/O input characteristics - TTL port  
![](images/106c0e60bfdc48ac9665c5e9e052693afca74be3957cd8b4f8aae1f19415f6b6.jpg)

Figure 28. 5 V tolerant I/O input characteristics - CMOS port  
![](images/c7f2e45516bfb7c0ac6202ba9a7d3f7f0bba3cb41db004fb3d3ffb4680538199.jpg)

Figure 29. 5 V tolerant I/O input characteristics - TTL port  
![](images/b3ea0c9f88a74984f18718519531ee53b87fc3aa022660457435e9e3d07f5be1.jpg)

## Output driving current

The GPIOs (general-purpose inputs/outputs) can sink or source up to ±8 mA, and sink or source up to ±20 mA (with a relaxed $\mathsf { V } _ { \mathsf { O L } } N _ { \mathsf { O H } } )$ except PC13, PC14 and PC15, which can sink or source up to ±3 mA. When using the GPIOs PC13 to PC15 in output mode, the speed should not exceed 2 MHz with a maximum load of 30 pF.

In the user application, the number of I/O pins which can drive current must be limited to respect the absolute maximum rating specified in Section 5.2:

The sum of the currents sourced by all the I/Os on $\mathsf { V } _ { \mathsf { D D } }$ <sub>,</sub> plus the maximum Run consumption of the MCU sourced on $\mathsf { V } _ { \mathsf { D D } }$ cannot exceed the absolute maximum rating $\mathsf { I } _ { \mathsf { V D D } }$ (see Table 7).

The sum of the currents sunk by all the I/Os on $\mathsf { V } _ { \mathsf { S S } }$ plus the maximum Run consumption of the MCU sunk on $\mathsf { V } _ { \mathsf { S S } }$ cannot exceed the absolute maximum rating $\mathsf { I } _ { \mathsf { V S S } }$ (see Table 7).

## Output voltage levels

Unless otherwise specified, the parameters given in Table 37 are derived from tests performed under ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9. All I/Os are CMOS and TTL compliant.

Table 37. Output voltage characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td> $V_{OL}^{(1)}$ </td><td>Output low level voltage for an I/O pin when 8 pins are sunk at same time</td><td rowspan="2">CMOS port $^{(2)}$ ,  $I_{IO} = +8 mA$  $2.7 V < V_{DD} < 3.6 V$ </td><td>-</td><td>0.4</td><td rowspan="8">V</td></tr><tr><td> $V_{OH}^{(3)}$ </td><td>Output high level voltage for an I/O pin when 8 pins are sourced at same time</td><td> $V_{DD}-0.4$ </td><td>-</td></tr><tr><td> $V_{OL}^{(1)}$ </td><td>Output low level voltage for an I/O pin when 8 pins are sunk at same time</td><td rowspan="2">TTL port $^{(2)}$  $I_{IO} = +8 mA$  $2.7 V < V_{DD} < 3.6 V$ </td><td>-</td><td>0.4</td></tr><tr><td> $V_{OH}^{(3)}$ </td><td>Output high level voltage for an I/O pin when 8 pins are sourced at same time</td><td>2.4</td><td>-</td></tr><tr><td> $V_{OL}^{(1)(4)}$ </td><td>Output low level voltage for an I/O pin when 8 pins are sunk at same time</td><td rowspan="2"> $I_{IO} = +20 mA$  $2.7 V < V_{DD} < 3.6 V$ </td><td>-</td><td>1.3</td></tr><tr><td> $V_{OH}^{(3)(4)}$ </td><td>Output high level voltage for an I/O pin when 8 pins are sourced at same time</td><td> $V_{DD}-1.3$ </td><td>-</td></tr><tr><td> $V_{OL}^{(1)(4)}$ </td><td>Output low level voltage for an I/O pin when 8 pins are sunk at same time</td><td rowspan="2"> $I_{IO} = +6 mA$  $2 V < V_{DD} < 2.7 V$ </td><td>-</td><td>0.4</td></tr><tr><td> $V_{OH}^{(3)(4)}$ </td><td>Output high level voltage for an I/O pin when 8 pins are sourced at same time</td><td> $V_{DD}-0.4$ </td><td>-</td></tr></table>

1. The $^ { 1 } _ { ! 0 }$ current sunk by the device must always respect the absolute maximum rating specified in Table 7 and the sum of I (I/O ports and control pins) must not exceed I .  
2. TTL and CMOS outputs are compatible with JEDEC standards JESD36 and JESD52.  
3. The $^ { \vert _ { 1 0 } \vert }$ current sourced by the device must always respect the absolute maximum rating specified in Table 7 and the sum of $| _ { | 0 } \big |$ (I/O ports and control pins) must not exceed I<sub>VDD</sub>.  
4. Evaluated by characterization, not tested in production, unless otherwise specified.

## Input/output AC characteristics

The definition and values of input/output AC characteristics are given in Figure 30 and Table 38, respectively.

Unless otherwise specified, the parameters given in Table 38 are derived from tests performed under the ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 38. I/O AC characteristics<sup>(1)</sup>

<table><tr><td> $MODEx[1:0]$ bit  $value^{(1)}$ </td><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="3">10</td><td> $f_{max(IO)out}$ </td><td> $Maximum frequency^{(2)}$ </td><td> $C_L=50\ pF, V_{DD}=2\ V to 3.6\ V$ </td><td>-</td><td>2</td><td>MHz</td></tr><tr><td> $t_{f(IO)out}$ </td><td>Output high to low level fall time</td><td rowspan="2"> $C_L=50\ pF, V_{DD}=2\ V to 3.6\ V$ </td><td>-</td><td> $125^{(3)}$ </td><td rowspan="2">ns</td></tr><tr><td> $t_{r(IO)out}$ </td><td>Output low to high level rise time</td><td>-</td><td> $125^{(3)}$ </td></tr><tr><td rowspan="3">01</td><td> $f_{max(IO)out}$ </td><td> $Maximum frequency^{(2)}$ </td><td> $C_L=50\ pF, V_{DD}=2\ V to 3.6\ V$ </td><td>-</td><td>10</td><td>MHz</td></tr><tr><td> $t_{f(IO)out}$ </td><td>Output high to low level fall time</td><td rowspan="2"> $C_L=50\ pF, V_{DD}=2\ V to 3.6\ V$ </td><td>-</td><td> $25^{(3)}$ </td><td rowspan="2">ns</td></tr><tr><td> $t_{r(IO)out}$ </td><td>Output low to high level rise time</td><td>-</td><td> $25^{(3)}$ </td></tr><tr><td rowspan="9">11</td><td rowspan="3"> $F_{max(IO)out}$ </td><td rowspan="3"> $Maximum frequency^{(2)}$ </td><td> $C_L=30\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td>50</td><td rowspan="3">MHz</td></tr><tr><td> $C_L=50\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td>30</td></tr><tr><td> $C_L=50\ pF, V_{DD}=2\ V to 2.7\ V$ </td><td>-</td><td>20</td></tr><tr><td rowspan="3"> $t_{f(IO)out}$ </td><td rowspan="3">Output high to low level fall time</td><td> $C_L=30\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td> $5^{(3)}$ </td><td rowspan="6">ns</td></tr><tr><td> $C_L=50\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td> $8^{(3)}$ </td></tr><tr><td> $C_L=50\ pF, V_{DD}=2\ V to 2.7\ V$ </td><td>-</td><td> $12^{(3)}$ </td></tr><tr><td rowspan="3"> $t_{r(IO)out}$ </td><td rowspan="3">Output low to high level rise time</td><td> $C_L=30\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td> $5^{(3)}$ </td></tr><tr><td> $C_L=50\ pF, V_{DD}=2.7\ V to 3.6\ V$ </td><td>-</td><td> $8^{(3)}$ </td></tr><tr><td> $C_L=50\ pF, V_{DD}=2\ V to 2.7\ V$ </td><td>-</td><td> $12^{(3)}$ </td></tr><tr><td>-</td><td> $t_{EXTIpw}$ </td><td>Pulse width of external signals detected by the EXTI controller</td><td>-</td><td>10</td><td>-</td><td>ns</td></tr></table>

1. The I/O speed is configured using the MODEx[1:0] bits. Refer to the STM32F10xxx reference manual for a description of GPIO port configuration register.  
2. The maximum frequency is defined in Figure 30.  
3. Specified by design, not tested in production.

Figure 30. I/O AC characteristics definition  
![](images/186c0b3d2234ca56b92662d0e4b27081a0adb7df5aa2ca4f4554a22f3d415263.jpg)

## 5.3.14 NRST pin characteristics

The NRST pin input driver uses CMOS technology. It is connected to a permanent pull-up resistor, $\mathsf { R } _ { \mathsf { P U } }$ (see Table 36).

Unless otherwise specified, the parameters given in Table 39 are derived from tests performed under the ambient temperature and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Table 39. NRST pin characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $V_{IL(NRST)}^{(1)}$ </td><td>NRST Input low level voltage</td><td>-</td><td>-0.5</td><td>-</td><td>0.8</td><td rowspan="2">V</td></tr><tr><td> $V_{IH(NRST)}^{(1)}$ </td><td>NRST Input high level voltage</td><td>-</td><td>2</td><td>-</td><td> $V_{DD}+0.5$ </td></tr><tr><td> $V_{hys(NRST)}$ </td><td>NRST Schmitt trigger voltage hysteresis</td><td>-</td><td>-</td><td>200</td><td>-</td><td>mV</td></tr><tr><td> $R_{PU}$ </td><td>Weak pull-up equivalent  $resistor^{(2)}$ </td><td> $V_{IN}=V_{SS}$ </td><td>30</td><td>40</td><td>50</td><td>kΩ</td></tr><tr><td> $V_{F(NRST)}^{(1)}$ </td><td>NRST Input filtered pulse</td><td>-</td><td>-</td><td>-</td><td>100</td><td>ns</td></tr><tr><td> $V_{NF(NRST)}^{(1)}$ </td><td>NRST Input not filtered pulse</td><td>-</td><td>300</td><td>-</td><td>-</td><td>ns</td></tr></table>

1. Specified by design, not tested in production.

Figure 31. Recommended NRST pin protection  
![](images/576d30d979468fa0933f456811030b66bf6ce385e0ed0f2e6764346d1ac6b980.jpg)  
2. The reset network protects the device against parasitic resets.  
3. The user must ensure that the level on the NRST pin can go below the V<sub>IL(NRST)</sub> max level specified in Table 39, otherwise the reset is not taken into account by the device.

## 5.3.15 TIM timer characteristics

The parameters given in Table 40 are specified by design, not tested in production.

Refer to Section 5.3.12 for details on the input/output alternate function characteristics (output compare, input capture, external clock, PWM output).

Table 40. $\mathtt { T I M X } ^ { ( 1 ) }$ characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="2"> $t_{res(TIM)}$ </td><td rowspan="2">Timer resolution time</td><td>-</td><td>1</td><td>-</td><td> $t_{TIMxCLK}$ </td></tr><tr><td> $f_{TIMxCLK} = 72 MHz$ </td><td>13.9</td><td>-</td><td>ns</td></tr><tr><td rowspan="2"> $f_{EXT}$ </td><td rowspan="2">Timer external clock frequency on CH1 to CH4</td><td>-</td><td>0</td><td> $f_{TIMxCLK}/2$ </td><td>MHz</td></tr><tr><td> $f_{TIMxCLK} = 72 MHz$ </td><td>0</td><td>36</td><td>MHz</td></tr><tr><td> $Res_{TIM}$ </td><td>Timer resolution</td><td>-</td><td>-</td><td>16</td><td>bit</td></tr><tr><td rowspan="2"> $t_{COUNTER}$ </td><td rowspan="2">16-bit counter clock period when internal clock is selected</td><td>-</td><td>1</td><td>65536</td><td> $t_{TIMxCLK}$ </td></tr><tr><td> $f_{TIMxCLK} = 72 MHz$ </td><td>0.0139</td><td>910</td><td>μs</td></tr><tr><td rowspan="2"> $t_{MAX\_COUNT}$ </td><td rowspan="2">Maximum possible count</td><td>-</td><td>-</td><td>65536 × 65536</td><td> $t_{TIMxCLK}$ </td></tr><tr><td> $f_{TIMxCLK} = 72 MHz$ </td><td>-</td><td>59.6</td><td>s</td></tr></table>

1. TIMx is used as a general term to refer to the TIM1, TIM2, TIM3 and TIM4 timers.

## 5.3.16 Communications interfaces

## $1 ^ { 2 } C$ interface characteristics

The STM32F103xx performance line ${ \mathsf { I } } ^ { 2 } { \mathsf { C } }$ interface meets the requirements of the standard ${ } ^ { 1 { } ^ { 2 } \mathrm { C } }$ communication protocol with the following restrictions: the I/O pins SDA and SCL are mapped to are not “true” open-drain. When configured as open-drain, the PMOS connected between the I/O pin and V is disabled, but is still present.

The ${ \mathsf { I } } ^ { 2 } { \mathsf { C } }$ characteristics are described in Table 41. Refer also to Section 5.3.12 for more details on the input/output alternate function characteristics (SDA and SCL).

Table 41. $\mathsf { I } ^ { 2 } \mathsf { C }$ characteristics

<table><tr><td rowspan="2">Symbol</td><td rowspan="2">Parameter</td><td colspan="2">Standard mode  $I^{2}C^{(1)(2)}$ </td><td colspan="2">Fast mode  $I^{2}C^{(1)(2)}$ </td><td rowspan="2">Unit</td></tr><tr><td>Min</td><td>Max</td><td>Min</td><td>Max</td></tr><tr><td> $t_{w(SCLL)}$ </td><td>SCL clock low time</td><td>4.7</td><td>-</td><td>1.3</td><td>-</td><td rowspan="2">μs</td></tr><tr><td> $t_{w(SCLH)}$ </td><td>SCL clock high time</td><td>4.0</td><td>-</td><td>0.6</td><td></td></tr><tr><td> $t_{su(SDA)}$ </td><td>SDA setup time</td><td>250</td><td>-</td><td>100</td><td>-</td><td rowspan="4">ns</td></tr><tr><td> $t_{h(SDA)}$ </td><td>SDA data hold time</td><td>-</td><td> $3450^{(3)}$ </td><td>-</td><td> $900^{(3)}$ </td></tr><tr><td> $t_{r(SDA)}$  $t_{r(SCL)}$ </td><td>SDA and SCL rise time</td><td>-</td><td>1000</td><td>-</td><td>300</td></tr><tr><td> $t_{f(SDA)}$  $t_{f(SCL)}$ </td><td>SDA and SCL fall time</td><td>-</td><td>300</td><td>-</td><td>300</td></tr><tr><td> $t_{h(STA)}$ </td><td>Start condition hold time</td><td>4.0</td><td>-</td><td>0.6</td><td>-</td><td rowspan="2">μs</td></tr><tr><td> $t_{su(STA)}$ </td><td>Repeated Start condition setup time</td><td>4.7</td><td>-</td><td>0.6</td><td>-</td></tr><tr><td> $t_{su(STO)}$ </td><td>Stop condition setup time</td><td>4.0</td><td>-</td><td>0.6</td><td>-</td><td>μs</td></tr><tr><td> $t_{w(STO:STA)}$ </td><td>Stop to Start condition time (bus free)</td><td>4.7</td><td>-</td><td>1.3</td><td>-</td><td>μs</td></tr><tr><td> $C_b$ </td><td>Capacitive load for each bus line</td><td>-</td><td>400</td><td>-</td><td>400</td><td>pF</td></tr><tr><td> $t_{SP}$ </td><td>Pulse width of spikes suppressed by the analog filter</td><td>0</td><td> $50^{(4)}$ </td><td>0</td><td> $50^{(4)}$ </td><td>ns</td></tr></table>

1. Specified by design, not tested in production.  
<sup>2.</sup> <sup>fPCLK1</sup> <sup>must</sup> <sup>be</sup> <sup>at</sup> <sup>least</sup> <sup>2</sup> <sup>MHz</sup> <sup>to</sup> <sup>achieve</sup> <sup>standard</sup> <sup>mode</sup>I<sup>2</sup>C frequencies. It must be a multiple of 10 MHz to reach ${ \mathsf { I } } ^ { 2 } { \mathsf { C } }$ frequencies. It must be at least 4 MHz to achieve fast mode <sub>400</sub> <sub>kHz</sub> <sub>maximum</sub> <sub>I2C</sub> <sub>fast</sub> <sub>mode</sub> <sub>clock.</sub>  
3. The maximum Data hold time must be met if the interface does not stretch the low period of SCL signal.  
4. The minimum width of the spikes filtered by the analog filter is above t (max).

Figure 32. $\mathsf { I } ^ { 2 } \mathsf { C }$ bus AC waveforms and measurement circuit  
![](images/5b4da85921d9bdcd9fdb49d94ebd0a425fe898405ff2df537ed1901118a80c49.jpg)  
1. Measurement points are done at CMOS levels: $0 . 3 \mathsf { V } _ { \mathsf { D } \mathsf { D } }$ and $0 . 7 \mathsf { V } _ { \mathsf { D } \mathsf { D } }$  
2. Rs = Series protection resistors, $\mathsf { R p } = \mathsf { P u l l - u p }$ resistors, $\mathsf { V } _ { \mathsf { D D \_ p c } } = 1 2 \mathsf { C }$ bus supply.

Table 42. SCL frequency $( \$ 123,456$ MHz, $\mathsf { V } _ { \mathsf { D D \ B C } } = 3 . 3 \mathsf { V } \mathsf { 0 } ^ { ( 1 ) ( 2 ) }$

<table><tr><td rowspan="2"> $f_{SCL}$  (kHz)</td><td>I2C_CCR value</td></tr><tr><td> $R_P = 4.7 \text{ k}\Omega$ </td></tr><tr><td>400</td><td>0x801E</td></tr><tr><td>300</td><td>0x8028</td></tr><tr><td>200</td><td>0x803C</td></tr><tr><td>100</td><td>0x00B4</td></tr><tr><td>50</td><td>0x0168</td></tr><tr><td>20</td><td>0x0384</td></tr></table>

1. ${ \mathsf { R } } _ { \mathsf { P } }$ = External pull-up resistance, $\mathsf { f } _ { \mathsf { S C L } } = | ^ { 2 } \mathsf { C }$ speed,

## SPI interface characteristics

Unless otherwise specified, the parameters given in Table 43 are derived from tests performed under the ambient temperature, f frequency and $\mathsf { V } _ { \mathsf { D D } }$ supply voltage conditions summarized in Table 9.

Refer to Section 5.3.12 for more details on the input/output alternate function characteristics (NSS, SCK, MOSI, MISO).

Table 43. SPI characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td rowspan="2"> $f_{SCK}$  $1/t_{c(SCK)}$ </td><td rowspan="2">SPI clock frequency</td><td>Master mode</td><td>-</td><td>18</td><td rowspan="2">MHz</td></tr><tr><td>Slave mode</td><td>-</td><td>18</td></tr><tr><td> $t_{r(SCK)}$  $t_{f(SCK)}$ </td><td>SPI clock rise and fall time</td><td>Capacitive load: C = 30 pF</td><td>-</td><td>8</td><td>ns</td></tr><tr><td>DuCy(SCK)</td><td>SPI slave input clock duty cycle</td><td>Slave mode</td><td>30</td><td>70</td><td>%</td></tr><tr><td> $t_{su(NSS)}^{(1)}$ </td><td>NSS setup time</td><td>Slave mode</td><td>4  $t_{PCLK}$ </td><td>-</td><td rowspan="13">ns</td></tr><tr><td> $t_{h(NSS)}^{(1)}$ </td><td>NSS hold time</td><td>Slave mode</td><td>2  $t_{PCLK}$ </td><td>-</td></tr><tr><td> $t_{w(SCKH)}^{(1)}$  $t_{w(SCKL)}^{(1)}$ </td><td>SCK high and low time</td><td>Master mode,  $f_{PCLK} = 36 MHz, presc = 4$ </td><td>50</td><td>60</td></tr><tr><td rowspan="2"> $t_{su(MI)}^{(1)}$  $t_{su(SI)}^{(1)}$ </td><td rowspan="2">Data input setup time</td><td>Master mode</td><td>5</td><td>-</td></tr><tr><td>Slave mode</td><td>5</td><td>-</td></tr><tr><td> $t_{h(MI)}^{(1)}$ </td><td rowspan="2">Data input hold time</td><td>Master mode</td><td>5</td><td>-</td></tr><tr><td> $t_{h(SI)}^{(1)}$ </td><td>Slave mode</td><td>4</td><td>-</td></tr><tr><td> $t_{a(SO)}^{(1)(2)}$ </td><td>Data output access time</td><td>Slave mode,  $f_{PCLK} = 20 MHz$ </td><td>0</td><td>3  $t_{PCLK}$ </td></tr><tr><td> $t_{dis(SO)}^{(1)(3)}$ </td><td>Data output disable time</td><td>Slave mode</td><td>2</td><td>10</td></tr><tr><td> $t_{v(SO)}^{(1)}$ </td><td>Data output valid time</td><td>Slave mode (after enable edge)</td><td>-</td><td>25</td></tr><tr><td> $t_{v(MO)}^{(1)}$ </td><td>Data output valid time</td><td>Master mode (after enable edge)</td><td>-</td><td>5</td></tr><tr><td> $t_{h(SO)}^{(1)}$ </td><td rowspan="2">Data output hold time</td><td>Slave mode (after enable edge)</td><td>15</td><td>-</td></tr><tr><td> $t_{h(MO)}^{(1)}$ </td><td>Master mode (after enable edge)</td><td>2</td><td>-</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. Min time is for the minimum time to drive the output and the max time is for the maximum time to validate the data.

Figure 33. SPI timing diagram - slave mode and CPHA = 0  
![](images/b76eb5a6876a005635f95bec2e826c278bbb1ab0381ef60a0d46957df0936bde.jpg)

Figure 34. SPI timing diagram - slave mode and CPHA = 1  
![](images/255e9ec552cc723ca01aa3bc5964b31de3a1e254926e45fceb3f38149c553865.jpg)

Figure 35. SPI timing diagram - master mode  
![](images/e9ccdd93327fd894033babcbdfd5a80df4b2bbf86703c56175ffac538b7cbae5.jpg)

## USB characteristics

The USB interface is USB-IF certified (Full Speed).

Table 44. USB startup time

<table><tr><td>Symbol</td><td>Parameter</td><td>Max</td><td>Unit</td></tr><tr><td> $t_{STARTUP}^{(1)}$ </td><td>USB transceiver startup time</td><td>1</td><td>μs</td></tr></table>

1. Guaranteed by design.

Table 45. USB DC electrical characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min.(1)</td><td>Max.(1)</td><td>Unit</td></tr><tr><td colspan="6">Input levels</td></tr><tr><td> $V_{DD}$ </td><td>USB operating  $voltage^{(2)}$ </td><td></td><td>3.0(3)</td><td>3.6</td><td>V</td></tr><tr><td> $V_{DI}^{(4)}$ </td><td>Differential input sensitivity</td><td>I(USBDP, USBDM)</td><td>0.2</td><td>-</td><td rowspan="3">V</td></tr><tr><td> $V_{CM}^{(4)}$ </td><td>Differential common mode range</td><td>Includes  $V_{DI}$  range</td><td>0.8</td><td>2.5</td></tr><tr><td> $V_{SE}^{(4)}$ </td><td>Single ended receiver threshold</td><td></td><td>1.3</td><td>2.0</td></tr><tr><td colspan="6">Output levels</td></tr><tr><td> $V_{OL}$ </td><td>Static output level low</td><td> $R_L$  of 1.5 kΩ to 3.6  $V^{(5)}$ </td><td>-</td><td>0.3</td><td rowspan="2">V</td></tr><tr><td> $V_{OH}$ </td><td>Static output level high</td><td> $R_L$  of 15 kΩ to  $V_{SS}^{(5)}$ </td><td>2.8</td><td>3.6</td></tr></table>

1. All the voltages are measured from the local ground potential.  
2. To be compliant with the USB 2.0 full-speed electrical specification, the USBDP (D+) pin must be pulled up with a 1.5 kΩ resistor to a 3.0 to 3.6 V voltage range.  
3. The STM32F103xx USB functionality is ensured down to $^ { 2 . 7 \mathrm { V } , }$ but not the full USB electrical characteristics, which are degraded in the $^ { 2 . 7 }$ to $3 . 0 \lor \lor _ { \mathsf { D D } }$ voltage range.  
4. Specified by design, not tested in production.  
5. $\mathsf { R } _ { \mathsf { L } }$ is the load connected on the USB drivers.

Figure 36. USB timings: definition of data signal rise and fall time

<table><tr><td>Differential data lines</td><td>points</td></tr><tr><td>VCRS</td><td></td></tr><tr><td>Vss</td><td></td></tr></table>

Table 46. USB: Full-speed electrical characteristics<sup>(1)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Max</td><td>Unit</td></tr><tr><td colspan="6">Driver characteristics</td></tr><tr><td> $t_r$ </td><td>Rise time(2)</td><td> $C_L$ =50 pF</td><td>4</td><td>20</td><td>ns</td></tr><tr><td> $t_f$ </td><td>Fall time(2)</td><td> $C_L$ =50 pF</td><td>4</td><td>20</td><td>ns</td></tr><tr><td> $t_{rfm}$ </td><td>Rise/ fall time matching</td><td> $t_r/t_f$ </td><td>90</td><td>110</td><td>%</td></tr><tr><td> $V_{CRS}$ </td><td>Output signal crossover voltage</td><td>-</td><td>1.3</td><td>2.0</td><td>V</td></tr></table>

1. Specified by design, not tested in production.  
2. Measured from 10% to 90% of the data signal. For more detailed informations, refer to USB specification - Section 7 (version 2.0).

## 5.3.17 CAN (controller area network) interface

Refer to Section 5.3.12 for more details on the input/output alternate function characteristics (CAN\_TX and CAN\_RX).

## 5.3.18 12-bit ADC characteristics

Unless otherwise specified, the parameters given in Table 47 are derived from tests performed under the ambient temperature, f frequency and $\mathsf { V } _ { \mathsf { D D A } }$ supply voltage conditions summarized in Table 9.

Note: It is recommended to perform a calibration after each power-up.

Table 47. ADC characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Conditions</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $V_{DDA}$ </td><td>Power supply</td><td>-</td><td>2.4</td><td>-</td><td>3.6</td><td>V</td></tr><tr><td> $V_{REF+}$ </td><td>Positive reference voltage</td><td>-</td><td>2.4</td><td>-</td><td> $V_{DDA}$ </td><td>V</td></tr><tr><td> $I_{VREF}$ </td><td>Current on the  $V_{REF}$  input pin</td><td>-</td><td>-</td><td>160(1)</td><td>220(1)</td><td>μA</td></tr><tr><td> $f_{ADC}$ </td><td>ADC clock frequency</td><td>-</td><td>0.6</td><td>-</td><td>14</td><td>MHz</td></tr><tr><td> $f_{S}^{(2)}$ </td><td>Sampling rate</td><td>-</td><td>0.05</td><td>-</td><td>1</td><td>MHz</td></tr><tr><td rowspan="2"> $f_{TRIG}^{(2)}$ </td><td rowspan="2">External trigger frequency</td><td> $f_{ADC} = 14 MHz$ </td><td>-</td><td>-</td><td>823</td><td>kHz</td></tr><tr><td></td><td>-</td><td>-</td><td>17</td><td>1 /  $f_{ADC}$ </td></tr><tr><td> $V_{AIN}^{(3)}$ </td><td>Conversion voltage range</td><td></td><td>0 ( $V_{SSA}$  or  $V_{REF-tied to ground}$ )</td><td>-</td><td> $V_{REF+}$ </td><td>V</td></tr><tr><td> $R_{AIN}^{(2)}$ </td><td>External input impedance</td><td>SeeEquation 1andTable 48for details</td><td>-</td><td>-</td><td>50</td><td>kΩ</td></tr><tr><td> $R_{ADC}^{(2)}$ </td><td>Sampling switch resistance</td><td>-</td><td>-</td><td>-</td><td>1</td><td>kΩ</td></tr><tr><td> $C_{ADC}^{(2)}$ </td><td>Internal sample and hold capacitor</td><td>-</td><td>-</td><td>-</td><td>8</td><td>pF</td></tr><tr><td rowspan="2"> $t_{CAL}^{(2)}$ </td><td rowspan="2">Calibration time</td><td> $f_{ADC} = 14 MHz$ </td><td colspan="3">5.9</td><td>μs</td></tr><tr><td>-</td><td colspan="3">83</td><td>1 /  $f_{ADC}$ </td></tr><tr><td rowspan="2"> $t_{lat}^{(2)}$ </td><td rowspan="2">Injection trigger conversion latency</td><td> $f_{ADC} = 14 MHz$ </td><td>-</td><td>-</td><td>0.214</td><td>μs</td></tr><tr><td>-</td><td>-</td><td>-</td><td>3(4)</td><td>1 /  $f_{ADC}$ </td></tr><tr><td rowspan="2"> $t_{latr}^{(2)}$ </td><td rowspan="2">Regular trigger conversion latency</td><td> $f_{ADC} = 14 MHz$ </td><td>-</td><td>-</td><td>0.143</td><td>μs</td></tr><tr><td>-</td><td>-</td><td>-</td><td>2(4)</td><td>1 /  $f_{ADC}$ </td></tr><tr><td rowspan="2"> $t_{S}^{(2)}$ </td><td rowspan="2">Sampling time</td><td> $f_{ADC} = 14 MHz$ </td><td>0.107</td><td>-</td><td>17.1</td><td>μs</td></tr><tr><td>-</td><td>1.5</td><td>-</td><td>239.5</td><td>1 /  $f_{ADC}$ </td></tr><tr><td> $t_{STAB}^{(2)}$ </td><td>Power-up time</td><td>-</td><td>0</td><td>0</td><td>1</td><td>μs</td></tr><tr><td rowspan="2"> $t_{CONV}^{(2)}$ </td><td rowspan="2">Total conversion time(including sampling time)</td><td> $f_{ADC} = 14 MHz$ </td><td>1</td><td>-</td><td>18</td><td>μs</td></tr><tr><td>-</td><td colspan="3">14 to 252 ( $t_{S}$  for sampling +12.5 for successive approximation)</td><td>1 /  $f_{ADC}$ </td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. Specified by design, not tested in production.  
3. In devices delivered in VFQFPN and LQFP packages, $\mathsf { V } _ { \mathsf { R E F } } .$ is internally connected to $\mathsf { V _ { D D A } }$ and $\mathsf { \underline { { V } } _ { R E F } } .$ is internally connected to $\mathsf { V } _ { \mathsf { S S A } ; }$ Devices that come in the TFBGA64 package have a V<sub>REF+</sub> pin but no V<sub>REF-</sub> pin (V<sub>REF-</sub> is internally connected to $\dot { \mathsf { V } } _ { \mathsf { S S A } } ^ { \circ \circ \kappa } )$ , see Table 5 and Figure 7.  
4. For external triggers, a delay of $1 / \mathsf { f } _ { \mathsf { P C L K 2 } }$ must be added to the latency specified in Table 47.

Equation 1: $\mathsf { R } _ { \mathsf { A I N } }$ max formula:

$$
R _ {\text { AIN }} <   \frac {T _ {\mathrm{S}}}{f _ {\mathrm{ADC}} \times C _ {\mathrm{ADC}} \times \ln \left(2 ^ {\mathrm{N} + 2}\right)} - R _ {\mathrm{ADC}}
$$

The formula above (Equation 1) is used to determine the maximum external impedance allowed for an error below 1/4 of LSB. Here N = 12 (from 12-bit resolution).

Table 48. $\mathsf { R } _ { \mathsf { A I N } }$ max for $\mathbf { f } _ { \mathsf { A D C } } = 1 4 \ \mathsf { M H z } ^ { ( 1 ) }$

<table><tr><td> $T_s$  (cycles)</td><td> $t_S$  (μs)</td><td> $R_{AIN}$  max (kΩ)</td></tr><tr><td>1.5</td><td>0.11</td><td>0.4</td></tr><tr><td>7.5</td><td>0.54</td><td>5.9</td></tr><tr><td>13.5</td><td>0.96</td><td>11.4</td></tr><tr><td>28.5</td><td>2.04</td><td>25.2</td></tr><tr><td>41.5</td><td>2.96</td><td>37.2</td></tr><tr><td>55.5</td><td>3.96</td><td>50</td></tr><tr><td>71.5</td><td>5.11</td><td>NA</td></tr><tr><td>239.5</td><td>17.1</td><td>NA</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.

Table 49. ADC accuracy - Limited test conditions<sup>(1)</sup> <sup>(2)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td>Test conditions</td><td>Typ</td><td>Max(3)</td><td>Unit</td></tr><tr><td>ET</td><td>Total unadjusted error</td><td rowspan="5"> $f_{PCLK2}=56MHz,$  $f_{ADC}=14MHz,R_{AIN}<10kΩ,$  $V_{DDA}=3V to 3.6V,$  $T_A=25°C$ Measurements made after ADC calibration</td><td>±1.3</td><td>±2</td><td rowspan="5">LSB</td></tr><tr><td>EO</td><td>Offset error</td><td>±1.0</td><td>±1.5</td></tr><tr><td>EG</td><td>Gain error</td><td>±0.5</td><td>±1.5</td></tr><tr><td>ED</td><td>Differential linearity error</td><td>±0.7</td><td>±1.0</td></tr><tr><td>EL</td><td>Integral linearity error</td><td>±0.8</td><td>±1.5</td></tr></table>

1. ADC DC accuracy values are measured after internal calibration.

2. Injecting a negative current on any analog input pins should be avoided as this significantly reduces the accuracy of the conversion being performed on another analog input. It is recommended to add a Schottky diode (pin to ground) to analog pins that may potentially inject negative currents. Any positive injection current within the limits specified for I<sub>INJ(PIN)</sub> and ΣI<sub>INJ(PIN)</sub> in Section 5.3.12 does not affect the ADC accuracy.

3. Evaluated by characterization, not tested in production, unless otherwise specified.

![](images/7bc205b88ea0e0825ed2fa2cd92258dca8a9f0199a3db247ac84fd32a8c557b5.jpg)  
Figure 37. ADC accuracy characteristics

Table 50. ADC accuracy<sup>(1)</sup> <sup>(2)</sup> <sup>(3)</sup>

<table><tr><td>Symbol</td><td>Parameter</td><td>Test conditions</td><td>Typ</td><td>Max(4)</td><td>Unit</td></tr><tr><td>ET</td><td>Total unadjusted error</td><td rowspan="5"> $f_{PCLK2}=56 MHz, f_{ADC}=14 MHz, R_{AIN}<10 kΩ, V_{DDA}=2.4 V to 3.6 V$ Measurements made after ADC calibration</td><td>±2</td><td>±5</td><td rowspan="5">LSB</td></tr><tr><td>EO</td><td>Offset error</td><td>±1.5</td><td>±2.5</td></tr><tr><td>EG</td><td>Gain error</td><td>±1.5</td><td>±3</td></tr><tr><td>ED</td><td>Differential linearity error</td><td>±1</td><td>±2</td></tr><tr><td>EL</td><td>Integral linearity error</td><td>±1.5</td><td>±3</td></tr></table>

1. ADC DC accuracy values are measured after internal calibration.  
2. Better performance can be achieved in restricted $\mathsf { V } _ { \mathsf { D D } } .$ , frequency and temperature ranges.  
3. Injecting a negative current on any analog input pins should be avoided as this significantly reduces the accuracy of the conversion being performed on another analog input. It is recommended to add a Schottky diode (pin to ground) to standard analog pins which may potentially inject negative current. Any positive injection current within the limits specified for I<sub>INJ(PIN)</sub> and $\mathsf { \bar { Z l } } _ { \mathsf { I N J } ( \mathsf { P } ^ { \mathsf { I N } } ) }$ in Section 5.3.12 does not affect the ADC accuracy.  
4. Evaluated by characterization, not tested in production, unless otherwise specified.  
(1) Example of an actual transfer curve (2) Ideal transfer curve (3) End-point correlation line  
n = ADC resolution  
E = total unadjusted error: maximum deviation between the actual and ideal transfer curves  
E = offset error: maximum deviation between the first actual transition and the first ideal one  
E = gain error: deviation between the last ideal transition and the last actual one  
E = differential linearity error: maximum deviation between actual steps and the ideal one  
E = integral linearity error: maximum deviation between any actual transition and the end point correlation line

Figure 38. Typical connection diagram using the ADC  
![](images/67a635fb24bf4700606dcfd2b120226272aacb4e5ed61930c7febc1e063a9c6d.jpg)  
1. Refer to Table 47 for the values of R<sub>AIN</sub>, ${ \mathsf { R } } _ { \mathsf { A D C } }$ and ${ \mathsf { C } } _ { \mathsf { A D C } }$

2. $\mathsf { C } _ { \mathsf { p a r a s i t i c } }$ represents the capacitance of the PCB (dependent on soldering and PCB layout quality) plus the pad capacitance (refer to Table 36 for the value of the pad capacitance). A high C<sub>parasitic</sub> value will downgrade conversion accuracy. To remedy this, ${ \mathsf { f } } _ { \mathsf { A D C } }$ should be reduced.

3. Refer to Table 36 for the values $\mathsf { o f l } _ { \mathsf { l k g } } .$

4. Refer to Figure 14.

## General PCB design guidelines

Power supply decoupling must be performed as shown in Figure 39 or Figure 40, depending on whether ${ \mathsf { V } } _ { \mathsf { R E F } } .$ is connected to $\mathsf { V } _ { \mathsf { D D A } }$ or not. The 10 nF capacitors should be ceramic (good quality), and placed as close as possible to the chip.

Figure 39. Power supply and reference decoupling $( V _ { R E F } +$ not connected to $\mathsf { v } _ { \mathsf { D D A } } )$  
![](images/14897fb3948181ff089056c7906aa7afa7fcc9f8bf1029b577bf695787bbde0c.jpg)  
1. V<sub>REF+</sub> and V<sub>REF–</sub> inputs are available only on 100-pin packages.

Figure 40. Power supply and reference decoupling $( V _ { R E F + }$ connected to $\mathsf { v } _ { \mathsf { D D A } } )$  
![](images/eb3f51b996cbe0646352c57ef5c4fc3ea1462d2b9db740e9474163c2ee6b48e3.jpg)  
1. $V _ { \mathsf { R E F } } .$ <sub>+</sub> and $V _ { \mathsf { R E F - } }$ inputs are available only on 100-pin packages.

## 5.3.19 Temperature sensor characteristics

Table 51. TS characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Min</td><td>Typ</td><td>Max</td><td>Unit</td></tr><tr><td> $T_{L}^{(1)}$ </td><td> $V_{SENSE}$  linearity with temperature</td><td>-</td><td>±1</td><td>±2</td><td>°C</td></tr><tr><td> $Avg\_Slope^{(1)}$ </td><td>Average slope</td><td>4.0</td><td>4.3</td><td>4.6</td><td>mV/°C</td></tr><tr><td> $V_{25}^{(1)}$ </td><td>Voltage at 25 °C</td><td>1.34</td><td>1.43</td><td>1.52</td><td>V</td></tr><tr><td> $t_{START}^{(2)}$ </td><td>Startup time</td><td>4</td><td>-</td><td>10</td><td rowspan="2">μs</td></tr><tr><td> $T_{S\_temp}^{(3)(2)}$ </td><td>ADC sampling time when reading the temperature</td><td>-</td><td>-</td><td>17.1</td></tr></table>

1. Evaluated by characterization, not tested in production, unless otherwise specified.  
2. Specified by design, not tested in production.

3. Shortest sampling time can be determined in the application by multiple iterations.

## Package information

In order to meet environmental requirements, ST offers these devices in different grades of ECOPACK<sup>®</sup> packages, depending on their level of environmental compliance. ECOPACK<sup>®</sup> specifications, grade definitions and product status are available at: www.st.com. ECOPACK<sup>®</sup> is an ST trademark.

## 6.1 Device marking

Refer to technical note “Reference device marking schematics for STM32 microcontrollers and microprocessors” (TN1433), available on www.st.com, for the location of pin 1/ ball A1, as well as the location and orientation of the marking areas versus pin 1/ball A1.

Parts marked as “ES”, “E”, or accompanied by an engineering sample notification letter, are not yet qualified and therefore not approved for use in production. ST is not responsible for any consequences resulting from such use. In no event will ST be liable for the customer using any of these engineering samples in production. ST’s Quality department must be contacted prior to any decision to use these engineering samples to run a qualification activity.

## 6.2 VFQFPN36 package information (ZR)

Figure 41. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package outline  
![](images/e9d1b588b2c93b36c87c49a9d742c2153e4265a5d7f9f833d763bb56844021d1.jpg)  
1. Drawing is not to scale.

Table 52. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(1)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>0.800</td><td>0.900</td><td>1.000</td><td>0.0315</td><td>0.0354</td><td>0.0394</td></tr><tr><td>A1</td><td>-</td><td>0.020</td><td>0.050</td><td>-</td><td>0.0008</td><td>0.0020</td></tr><tr><td>A2</td><td>-</td><td>0.650</td><td>1.000</td><td>-</td><td>0.0256</td><td>0.0394</td></tr><tr><td>A3</td><td>-</td><td>0.200</td><td>-</td><td>-</td><td>0.0079</td><td>-</td></tr><tr><td>b</td><td>0.180</td><td>0.230</td><td>0.300</td><td>0.0071</td><td>0.0091</td><td>0.0118</td></tr><tr><td>D</td><td>5.875</td><td>6.000</td><td>6.125</td><td>0.2313</td><td>0.2362</td><td>0.2411</td></tr><tr><td>D2</td><td>1.750</td><td>3.700</td><td>4.250</td><td>0.0689</td><td>0.1457</td><td>0.1673</td></tr><tr><td>E</td><td>5.875</td><td>6.000</td><td>6.125</td><td>0.2313</td><td>0.2362</td><td>0.2411</td></tr><tr><td>E2</td><td>1.750</td><td>3.700</td><td>4.250</td><td>0.0689</td><td>0.1457</td><td>0.1673</td></tr><tr><td>e</td><td>0.450</td><td>0.500</td><td>0.550</td><td>0.0177</td><td>0.0197</td><td>0.0217</td></tr><tr><td>L</td><td>0.350</td><td>0.550</td><td>0.750</td><td>0.0138</td><td>0.0217</td><td>0.0295</td></tr><tr><td>K</td><td>0.250</td><td>-</td><td>-</td><td>0.0098</td><td>-</td><td>-</td></tr><tr><td>ddd</td><td>-</td><td>-</td><td>0.080</td><td>-</td><td>-</td><td>0.0031</td></tr></table>

1. Values in inches are converted from mm and rounded to 4 decimal digits.

Figure 42. VFQFPN - 36 pin, 6x6 mm, 0.5 mm pitch very thin profile fine pitch quad flat package recommended footprint  
![](images/4be0c2ffa2f4c5c4020b522debef27375d051de68ee5b212901b5e43c01bae6c.jpg)  
1. Dimensions are expressed in millimeters.

## 6.3 UFQFPN48 package information (A0B9)

This UFQFPN is a 48-lead, 7 x 7 mm, 0.5 mm pitch, ultra thin fine pitch quad flat package.

Figure 43. UFQFPN48 – Outline  
![](images/1b29037206e0c87864a496682aac500d057cb88f6c15809c5245d1ac232bc89d.jpg)  
1. Drawing is not to scale.  
2. All leads/pads should also be soldered to the PCB to improve the lead/pad solder joint life.  
3. There is an exposed die pad on the underside of the UFQFPN48 package. It is recommended to connect and solder this back-side pad to PCB ground.

A0B9\_UFQFPN48\_FP\_V3

Table 53. UFQFPN48 – Mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(1)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>0.500</td><td>0.550</td><td>0.600</td><td>0.0197</td><td>0.0217</td><td>0.0236</td></tr><tr><td>A1</td><td>0.000</td><td>0.020</td><td>0.050</td><td>0.0000</td><td>0.0008</td><td>0.0020</td></tr><tr><td>A3</td><td>-</td><td>0.152</td><td>-</td><td>-</td><td>0.0060</td><td>-</td></tr><tr><td>b</td><td>0.200</td><td>0.250</td><td>0.300</td><td>0.0079</td><td>0.0098</td><td>0.0118</td></tr><tr><td> $D^{(2)}$ </td><td>6.900</td><td>7.000</td><td>7.100</td><td>0.2717</td><td>0.2756</td><td>0.2795</td></tr><tr><td>D1</td><td>5.400</td><td>5.500</td><td>5.600</td><td>0.2126</td><td>0.2165</td><td>0.2205</td></tr><tr><td> $D2^{(3)}$ </td><td>5.500</td><td>5.600</td><td>5.700</td><td>0.2165</td><td>0.2205</td><td>0.2244</td></tr><tr><td> $E^{(2)}$ </td><td>6.900</td><td>7.000</td><td>7.100</td><td>0.2717</td><td>0.2756</td><td>0.2795</td></tr><tr><td>E1</td><td>5.400</td><td>5.500</td><td>5.600</td><td>0.2126</td><td>0.2165</td><td>0.2205</td></tr><tr><td> $E2^{(3)}$ </td><td>5.500</td><td>5.600</td><td>5.700</td><td>0.2165</td><td>0.2205</td><td>0.2244</td></tr><tr><td>e</td><td>-</td><td>0.500</td><td>-</td><td>-</td><td>0.0197</td><td>-</td></tr><tr><td>L</td><td>0.300</td><td>0.400</td><td>0.500</td><td>0.0118</td><td>0.0157</td><td>0.0197</td></tr><tr><td>ddd</td><td>-</td><td>-</td><td>0.080</td><td>-</td><td>-</td><td>0.0031</td></tr></table>

1. Values in inches are converted from mm and rounded to four decimal digits.  
2. Dimensions D and E do not include mold protrusion, not exceed 0.15 mm.  
3. Dimensions D2 and E2 are not in accordance with JEDEC.

Figure 44. UFQFPN48 – Footprint example  
![](images/9fb11134043664733bd1dbe62ecbb127ece459e340c11c5533e7268d56590a96.jpg)  
1. Dimensions are expressed in millimeters.

## 6.4 LFBGA100 package information (H0)

Figure 45. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package outline  
![](images/580fab92d25506741bba855fb3cb9b6a6aee20ce62da0ba9172b2f8ca215acf3.jpg)  
1. Drawing is not to scale.

Table 54. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(1)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Typ</td><td>Min</td><td>Max</td></tr><tr><td>A</td><td>-</td><td>-</td><td>1.700</td><td></td><td></td><td>0.0669</td></tr><tr><td>A1</td><td>0.270</td><td>-</td><td>-</td><td>0.0106</td><td></td><td></td></tr><tr><td>A2</td><td>-</td><td>0.300</td><td>-</td><td></td><td>0.0118</td><td></td></tr><tr><td>A4</td><td>-</td><td>-</td><td>0.800</td><td>-</td><td>-</td><td>0.0315</td></tr><tr><td>b</td><td>0.450</td><td>0.500</td><td>0.550</td><td>0.0177</td><td>0.0197</td><td>0.0217</td></tr><tr><td>D</td><td>9.850</td><td>10.000</td><td>10.150</td><td>0.3878</td><td>0.3937</td><td>0.3996</td></tr><tr><td>D1</td><td>-</td><td>7.200</td><td>-</td><td>-</td><td>0.2835</td><td>-</td></tr><tr><td>E</td><td>9.850</td><td>10.000</td><td>10.150</td><td>0.3878</td><td>0.3937</td><td>0.3996</td></tr><tr><td>E1</td><td>-</td><td>7.200</td><td>-</td><td>-</td><td>0.2835</td><td>-</td></tr><tr><td>e</td><td>-</td><td>0.800</td><td>-</td><td>-</td><td>0.0315</td><td>-</td></tr><tr><td>F</td><td>-</td><td>1.400</td><td>-</td><td>-</td><td>0.0551</td><td>-</td></tr><tr><td>ddd</td><td>-</td><td>-</td><td>0.120</td><td>-</td><td>-</td><td>0.0047</td></tr><tr><td>eee</td><td>-</td><td>-</td><td>0.150</td><td>-</td><td>-</td><td>0.0059</td></tr><tr><td>fff</td><td>-</td><td>-</td><td>0.080</td><td>-</td><td>-</td><td>0.0031</td></tr></table>

1. Values in inches are converted from mm and rounded to 4 decimal digits.

Figure 46. LFBGA100 – 100-ball low profile fine pitch ball grid array, 10 x 10 mm, 0.8 mm pitch, package recommended footprint

<table><tr><td></td></tr></table>

Table 55. LFBGA100 recommended PCB design rules (0.8 mm pitch BGA)

<table><tr><td>Dimension</td><td>Recommended values</td></tr><tr><td>Pitch</td><td>0.8</td></tr><tr><td>Dpad</td><td>0.500 mm</td></tr><tr><td>Dsm</td><td>0.570 mm typ. (depends on the soldermask registration tolerance)</td></tr><tr><td>Stencil opening</td><td>0.500 mm</td></tr><tr><td>Stencil thickness</td><td>Between 0.100 mm and 0.125 mm</td></tr><tr><td>Pad trace width</td><td>0.120 mm</td></tr></table>

## 6.5 LQFP100 package information (1L)

This LQFP is 100 lead, 14 x 14 mm low-profile quad flat package.

Note: See list of notes in the notes section.

Figure 47. LQFP100 - Outline<sup>(15)</sup>  
![](images/19c44f2e206e604c8a976971c63cbb4f23bb4c1016a2f90f489da1d598b8bc83.jpg)

Table 56. LQFP100 - Mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(14)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>-</td><td>1.50</td><td>1.60</td><td>-</td><td>0.0590</td><td>0.0630</td></tr><tr><td> $A1^{(12)}$ </td><td>0.05</td><td>-</td><td>0.15</td><td>0.0019</td><td>-</td><td>0.0059</td></tr><tr><td>A2</td><td>1.35</td><td>1.40</td><td>1.45</td><td>0.0531</td><td>0.0551</td><td>0.0570</td></tr><tr><td> $b^{(9)(11)}$ </td><td>0.17</td><td>0.22</td><td>0.27</td><td>0.0067</td><td>0.0087</td><td>0.0106</td></tr><tr><td> $b1^{(11)}$ </td><td>0.17</td><td>0.20</td><td>0.23</td><td>0.0067</td><td>0.0079</td><td>0.0090</td></tr><tr><td> $c^{(11)}$ </td><td>0.09</td><td>-</td><td>0.20</td><td>0.0035</td><td>-</td><td>0.0079</td></tr><tr><td> $c1^{(11)}$ </td><td>0.09</td><td>-</td><td>0.16</td><td>0.0035</td><td>-</td><td>0.0063</td></tr><tr><td> $D^{(4)}$ </td><td colspan="3">16.00 BSC</td><td colspan="3">0.6299 BSC</td></tr><tr><td> $D1^{(2)(5)}$ </td><td colspan="3">14.00 BSC</td><td colspan="3">0.5512 BSC</td></tr><tr><td> $E^{(4)}$ </td><td colspan="3">16.00 BSC</td><td colspan="3">0.6299 BSC</td></tr><tr><td> $E1^{(2)(5)}$ </td><td colspan="3">14.00 BSC</td><td colspan="3">0.5512 BSC</td></tr><tr><td>e</td><td colspan="3">0.50 BSC</td><td colspan="3">0.0197 BSC</td></tr><tr><td>L</td><td>0.45</td><td>0.60</td><td>0.75</td><td>0.177</td><td>0.0236</td><td>0.0295</td></tr><tr><td> $L1^{(1)(11)}$ </td><td colspan="3">1.00</td><td>-</td><td>0.0394</td><td>-</td></tr><tr><td> $N^{(13)}$ </td><td colspan="6">100</td></tr><tr><td>θ</td><td>0°</td><td>3.5°</td><td>7°</td><td>0°</td><td>3.5°</td><td>7°</td></tr><tr><td>θ1</td><td>0°</td><td>-</td><td>-</td><td>0°</td><td>-</td><td>-</td></tr><tr><td>θ2</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>θ3</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>R1</td><td>0.08</td><td>-</td><td>-</td><td>0.0031</td><td>-</td><td>-</td></tr><tr><td>R2</td><td>0.08</td><td>-</td><td>0.20</td><td>0.0031</td><td>-</td><td>0.0079</td></tr><tr><td>S</td><td>0.20</td><td>-</td><td>-</td><td>0.0079</td><td>-</td><td>-</td></tr><tr><td> $aaa^{(1)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $bbb^{(1)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $ccc^{(1)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr><tr><td> $ddd^{(1)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr></table>

## Notes:

1. Dimensioning and tolerancing schemes conform to ASME Y14.5M-1994.

2. The Top package body size may be smaller than the bottom package size by as much as 0.15 mm.

3. Datums A-B and D to be determined at datum plane H.

4. To be determined at seating datum plane C.

5. Dimensions D1 and E1 do not include mold flash or protrusions. Allowable mold flash or protrusions is “0.25 mm” per side. D1 and E1 are Maximum plastic body size dimensions including mold mismatch.

6. Details of pin 1 identifier are optional but must be located within the zone indicated.

7. All Dimensions are in millimeters.

8. No intrusion allowed inwards the leads.

9. Dimension “b” does not include dambar protrusion. Allowable dambar protrusion shall not cause the lead width to exceed the maximum “b” dimension by more than 0.08 mm. Dambar cannot be located on the lower radius or the foot. Minimum space between protrusion and an adjacent lead is 0.07 mm for 0.4 mm and 0.5 mm pitch packages.

10. Exact shape of each corner is optional.

11. These dimensions apply to the flat section of the lead between 0.10 mm and 0.25 mm from the lead tip.

12. A1 is defined as the distance from the seating plane to the lowest point on the package body.

13. “N” is the number of terminal positions for the specified body size.

14. Values in inches are converted from mm and rounded to 4 decimal digits.

15. Drawing is not to scale.

Figure 48. LQFP100 - Footprint example  
![](images/a12fd084604cb76ef47a40ed5aadbe4bcb306d1b850fd7b0f9adf724eada609c.jpg)  
1L\_LQFP100\_FP\_V1  
1. Dimensions are expressed in millimeters.

## 6.6 UFBGA100 package information (A0C2)

This UFBGA is a 100-ball, 7 x 7 mm, 0.50 mm pitch, ultra fine pitch ball grid array package. See list of notes in the notes section.

Figure 49. UFBGA100 - Outline<sup>(13)</sup>  
![](images/547f72a1442910db1f398a294830815b89a47828ff58ffb7b3290179562aced4.jpg)

Table 57. UFBGA100 - Mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3"> $millimeters^{(1)}$ </td><td colspan="3"> $inches^{(12)}$ </td></tr><tr><td>Min.</td><td>Typ.</td><td>Max.</td><td>Min.</td><td>Typ.</td><td>Max.</td></tr><tr><td> $A^{(2)(3)}$ </td><td>-</td><td>-</td><td>0.60</td><td>-</td><td>-</td><td>0.0236</td></tr><tr><td> $A1^{(4)}$ </td><td>0.05</td><td>-</td><td>-</td><td>0.0020</td><td>-</td><td>-</td></tr><tr><td>A2</td><td>-</td><td>0.43</td><td>-</td><td>-</td><td>0.0169</td><td>-</td></tr><tr><td> $b^{(5)}$ </td><td>0.23</td><td>0.28</td><td>0.33</td><td>0.0090</td><td>0.0110</td><td>0.0130</td></tr><tr><td> $D^{(6)}$ </td><td colspan="3">7.00 BSC</td><td colspan="3">0.2756 BSC</td></tr><tr><td>D1</td><td colspan="3">5.50 BSC</td><td colspan="3">0.2165 BSC</td></tr><tr><td>E</td><td colspan="3">7.00 BSC</td><td colspan="3">0.2756 BSC</td></tr><tr><td>E1</td><td colspan="3">5.50 BSC</td><td colspan="3">0.2165 BSC</td></tr><tr><td> $e^{(9)}$ </td><td colspan="3">0.50 BSC</td><td colspan="3">0.0197 BSC</td></tr><tr><td> $N^{(11)}$ </td><td colspan="6">100</td></tr><tr><td> $SD^{(12)}$ </td><td colspan="3">0.25 BSC</td><td colspan="3">0.0098 BSC</td></tr><tr><td> $SE^{(12)}$ </td><td colspan="3">0.25 BSC</td><td colspan="3">0.0098 BSC</td></tr><tr><td>aaa</td><td colspan="3">0.15</td><td colspan="3">0.0059</td></tr><tr><td>ccc</td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td>ddd</td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr><tr><td>eee</td><td colspan="3">0.15</td><td colspan="3">0.0059</td></tr><tr><td>fff</td><td colspan="3">0.05</td><td colspan="3">0.0020</td></tr></table>

## Notes:

1. Dimensioning and tolerancing schemes conform to ASME Y14.5M-2009 apart European projection.

2. UFBGA stands for ulta profile fine pitch ball grid array: 0.50 mm < A ≤ 0.65 mm / fine pitch e < 1.00 mm.

3. The profile height, A, is the distance from the seating plane to the highest point on the package. It is measured perpendicular to the seating plane.

4. A1 is defined as the distance from the seating plane to the lowest point on the package body.

5. Dimension b is measured at the maximum diameter of the terminal (ball) in a plane parallel to primary datum C.

6. BSC stands for BASIC dimensions. It corresponds to the nominal value and has no tolerance. For tolerances refer to form and position table. On the drawing these dimensions are framed.

7. Primary datum C is defined by the plane established by the contact points of three or more solder balls that support the device when it is placed on top of a planar surface.

8. The terminal (ball) A1 corner must be identified on the top surface of the package by using a corner chamfer, ink or metalized markings, or other feature of package body or integral heat slug. A distinguish feature is allowable on the bottom surface of the package to identify the terminal A1 corner. Exact shape of each corner is optional.

![](images/c8861036dc282d3a6cda621332c9bc2711caef6947a5220e26eecd9d65dca53d.jpg)

9. e represents the solder ball grid pitch.

10. N represents the total number of balls on the BGA.

11. Basic dimensions SD and SE are defined with respect to datums A and B. It defines the position of the centre ball(s) in the outer row or column of a fully populated matrix.

12. Values in inches are converted from mm and rounded to 4 decimal digits.

13. Drawing is not to scale.

Figure 50. UFBGA100 - Footprint example

BGA\_WLCSP\_FT\_V1

Table 58. UFBGA100 - Example of PCB design rules (0.5 mm pitch BGA)

<table><tr><td>Dimension</td><td>Values</td></tr><tr><td>Pitch</td><td>0.50 mm</td></tr><tr><td>Dpad</td><td>0.280 mm</td></tr><tr><td>Dsm</td><td>0.370 mm typ. (depends on the solder mask registration tolerance)</td></tr><tr><td>Stencil opening</td><td>0.280 mm</td></tr><tr><td>Stencil thickness</td><td>Between 0.100 mm and 0.125 mm</td></tr></table>

## 6.7 LQFP64 package information (5W)

This LQFP is 64-pin, 10 x 10 mm low-profile quad flat package.

Note: See list of notes in the notes section.

Figure 51. LQFP64 - Outline<sup>(15)</sup>  
![](images/dad0b33270c3f564ff7e4043f192b2165f98c0a4fab49ef7ea82ae64970e536a.jpg)

Table 59. LQFP64 - Mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(14)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>-</td><td>-</td><td>1.60</td><td>-</td><td>-</td><td>0.0630</td></tr><tr><td> $A1^{(12)}$ </td><td>0.05</td><td>-</td><td>0.15</td><td>0.0020</td><td>-</td><td>0.0059</td></tr><tr><td>A2</td><td>1.35</td><td>1.40</td><td>1.45</td><td>0.0531</td><td>0.0551</td><td>0.0570</td></tr><tr><td> $b^{(9)(11)}$ </td><td>0.17</td><td>0.22</td><td>0.27</td><td>0.0067</td><td>0.0087</td><td>0.0106</td></tr><tr><td> $b1^{(11)}$ </td><td>0.17</td><td>0.20</td><td>0.23</td><td>0.0067</td><td>0.0079</td><td>0.0091</td></tr><tr><td> $c^{(11)}$ </td><td>0.09</td><td>-</td><td>0.20</td><td>0.0035</td><td>-</td><td>0.0079</td></tr><tr><td> $c1^{(11)}$ </td><td>0.09</td><td>-</td><td>0.16</td><td>0.0035</td><td>-</td><td>0.0063</td></tr><tr><td> $D^{(4)}$ </td><td colspan="3">12.00 BSC</td><td colspan="3">0.4724 BSC</td></tr><tr><td> $D1^{(2)(5)}$ </td><td colspan="3">10.00 BSC</td><td colspan="3">0.3937 BSC</td></tr><tr><td> $E^{(4)}$ </td><td colspan="3">12.00 BSC</td><td colspan="3">0.4724 BSC</td></tr><tr><td> $E1^{(2)(5)}$ </td><td colspan="3">10.00 BSC</td><td colspan="3">0.3937 BSC</td></tr><tr><td>e</td><td colspan="3">0.50 BSC</td><td colspan="3">0.1970 BSC</td></tr><tr><td>L</td><td>0.45</td><td>0.60</td><td>0.75</td><td>0.0177</td><td>0.0236</td><td>0.0295</td></tr><tr><td>L1</td><td colspan="3">1.00 REF</td><td colspan="3">0.0394 REF</td></tr><tr><td> $N^{(13)}$ </td><td colspan="6">64</td></tr><tr><td>θ</td><td>0°</td><td>3.5°</td><td>7°</td><td>0°</td><td>3.5°</td><td>7°</td></tr><tr><td>θ1</td><td>0°</td><td>-</td><td>-</td><td>0°</td><td>-</td><td>-</td></tr><tr><td>θ2</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>θ3</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>R1</td><td>0.08</td><td>-</td><td>-</td><td>0.0031</td><td>-</td><td>-</td></tr><tr><td>R2</td><td>0.08</td><td>-</td><td>0.20</td><td>0.0031</td><td>-</td><td>0.0079</td></tr><tr><td>S</td><td>0.20</td><td>-</td><td>-</td><td>0.0079</td><td>-</td><td>-</td></tr><tr><td> $aaa^{(1)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $bbb^{(1)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $ccc^{(1)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr><tr><td> $ddd^{(1)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr></table>

## Notes:

1. Dimensioning and tolerancing schemes conform to ASME Y14.5M-1994.

2. The Top package body size may be smaller than the bottom package size by as much as 0.15 mm.

3. Datums A-B and D to be determined at datum plane H.

4. To be determined at seating datum plane C.

5. Dimensions D1 and E1 do not include mold flash or protrusions. Allowable mold flash or protrusions is “0.25 mm” per side. D1 and E1 are Maximum plastic body size dimensions including mold mismatch.

6. Details of pin 1 identifier are optional but must be located within the zone indicated.

7. All Dimensions are in millimeters.

8. No intrusion allowed inwards the leads.

9. Dimension “b” does not include dambar protrusion. Allowable dambar protrusion shall not cause the lead width to exceed the maximum “b” dimension by more than 0.08 mm. Dambar cannot be located on the lower radius or the foot. Minimum space between protrusion and an adjacent lead is 0.07 mm for 0.4 mm and 0.5 mm pitch packages.

10. Exact shape of each corner is optional.

11. These dimensions apply to the flat section of the lead between 0.10 mm and 0.25 mm from the lead tip.

12. A1 is defined as the distance from the seating plane to the lowest point on the package body.

13. “N” is the number of terminal positions for the specified body size.

14. Values in inches are converted from mm and rounded to 4 decimal digits.

15. Drawing is not to scale.

Figure 52. LQFP64 - Footprint example  
![](images/09eb7e57facbcdefa0e4957239f45f8def411e28f7db7ec41f5cd0907b82f246.jpg)  
1. Dimensions are expressed in millimeters.

## 6.8 TFBGA64 package information (R8)

Figure 53. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch thin profile fine pitch ball grid array package outline  
![](images/f2893c5726284f0bf59cc5ce409cf80e662d329e498dbbaa1bf1e800335d7819.jpg)  
1. Drawing is not to scale.

Table 60. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch, thin profile fine pitch ball grid array package mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(1)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>-</td><td>-</td><td>1.200</td><td>-</td><td>-</td><td>0.0472</td></tr><tr><td>A1</td><td>0.150</td><td>-</td><td>-</td><td>0.0059</td><td>-</td><td>-</td></tr><tr><td>A2</td><td>-</td><td>0.200</td><td>-</td><td>-</td><td>0.0079</td><td>-</td></tr><tr><td>A4</td><td>-</td><td>-</td><td>0.600</td><td>-</td><td>-</td><td>0.0236</td></tr><tr><td>b</td><td>0.250</td><td>0.300</td><td>0.350</td><td>0.0098</td><td>0.0118</td><td>0.0138</td></tr><tr><td>D</td><td>4.850</td><td>5.000</td><td>5.150</td><td>0.1909</td><td>0.1969</td><td>0.2028</td></tr><tr><td>D1</td><td>-</td><td>3.500</td><td>-</td><td>-</td><td>0.1378</td><td>-</td></tr><tr><td>E</td><td>4.850</td><td>5.000</td><td>5.150</td><td>0.1909</td><td>0.1969</td><td>0.2028</td></tr><tr><td>E1</td><td>-</td><td>3.500</td><td>-</td><td>-</td><td>0.1378</td><td>-</td></tr><tr><td>e</td><td>-</td><td>0.500</td><td>-</td><td>-</td><td>0.0197</td><td>-</td></tr><tr><td>F</td><td>-</td><td>0.750</td><td>-</td><td>-</td><td>0.0295</td><td>-</td></tr><tr><td>ddd</td><td>-</td><td>-</td><td>0.080</td><td>-</td><td>-</td><td>0.0031</td></tr><tr><td>eee</td><td>-</td><td>-</td><td>0.150</td><td>-</td><td>-</td><td>0.0059</td></tr><tr><td>fff</td><td>-</td><td>-</td><td>0.050</td><td>-</td><td>-</td><td>0.0020</td></tr></table>

1. Values in inches are converted from mm and rounded to 4 decimal digits.

Figure 54. TFBGA64 – 64-ball, 5 x 5 mm, 0.5 mm pitch, thin profile fine pitch ball grid array  
, recommended footprint

<table><tr><td></td><td>Dpad</td><td>Dsm</td></tr></table>

Table 61. TFBGA64 recommended PCB design rules (0.5 mm pitch BGA)

<table><tr><td>Dimension</td><td>Recommended values</td></tr><tr><td>Pitch</td><td>0.5</td></tr><tr><td>Dpad</td><td>0.280 mm</td></tr><tr><td>Dsm</td><td>0.370 mm typ. (depends on the soldermask registration tolerance)</td></tr><tr><td>Stencil opening</td><td>0.280 mm</td></tr><tr><td>Stencil thickness</td><td>Between 0.100 mm and 1.125 mm</td></tr><tr><td>Pad trace width</td><td>0.100 mm</td></tr></table>

## 6.9 LQFP48 package information (5B)

This LQFP is a 48-pin, 7 x 7 mm low-profile quad flat package

Note: See list of notes in the notes section.

Figure 55. LQFP48 – Outline<sup>(15)</sup>  
![](images/62345aecc5577aedf48646523b559d5063e994b34f35cbd90418ea05a1753953.jpg)

Table 62. LQFP48 – Mechanical data

<table><tr><td rowspan="2">Symbol</td><td colspan="3">millimeters</td><td colspan="3"> $inches^{(14)}$ </td></tr><tr><td>Min</td><td>Typ</td><td>Max</td><td>Min</td><td>Typ</td><td>Max</td></tr><tr><td>A</td><td>-</td><td>-</td><td>1.60</td><td>-</td><td>-</td><td>0.0630</td></tr><tr><td> $A1^{(12)}$ </td><td>0.05</td><td>-</td><td>0.15</td><td>0.0020</td><td>-</td><td>0.0059</td></tr><tr><td>A2</td><td>1.35</td><td>1.40</td><td>1.45</td><td>0.0531</td><td>0.0551</td><td>0.0571</td></tr><tr><td> $b^{(9)(11)}$ </td><td>0.17</td><td>0.22</td><td>0.27</td><td>0.0067</td><td>0.0087</td><td>0.0106</td></tr><tr><td> $b1^{(11)}$ </td><td>0.17</td><td>0.20</td><td>0.23</td><td>0.0067</td><td>0.0079</td><td>0.0090</td></tr><tr><td> $c^{(11)}$ </td><td>0.09</td><td>-</td><td>0.20</td><td>0.0035</td><td>-</td><td>0.0079</td></tr><tr><td> $c1^{(11)}$ </td><td>0.09</td><td>-</td><td>0.16</td><td>0.0035</td><td>-</td><td>0.0063</td></tr><tr><td> $D^{(4)}$ </td><td colspan="3">9.00 BSC</td><td colspan="3">0.3543 BSC</td></tr><tr><td> $D1^{(2)(5)}$ </td><td colspan="3">7.00 BSC</td><td colspan="3">0.2756 BSC</td></tr><tr><td> $E^{(4)}$ </td><td colspan="3">9.00 BSC</td><td colspan="3">0.3543 BSC</td></tr><tr><td> $E1^{(2)(5)}$ </td><td colspan="3">7.00 BSC</td><td colspan="3">0.2756 BSC</td></tr><tr><td>e</td><td colspan="3">0.50 BSC</td><td colspan="3">0.1970 BSC</td></tr><tr><td>L</td><td>0.45</td><td>0.60</td><td>0.75</td><td>0.0177</td><td>0.0236</td><td>0.0295</td></tr><tr><td>L1</td><td colspan="3">1.00 REF</td><td colspan="3">0.0394 REF</td></tr><tr><td> $N^{(13)}$ </td><td colspan="6">48</td></tr><tr><td>θ</td><td>0°</td><td>3.5°</td><td>7°</td><td>0°</td><td>3.5°</td><td>7°</td></tr><tr><td>θ1</td><td>0°</td><td>-</td><td>-</td><td>0°</td><td>-</td><td>-</td></tr><tr><td>θ2</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>θ3</td><td>10°</td><td>12°</td><td>14°</td><td>10°</td><td>12°</td><td>14°</td></tr><tr><td>R1</td><td>0.08</td><td>-</td><td>-</td><td>0.0031</td><td>-</td><td>-</td></tr><tr><td>R2</td><td>0.08</td><td>-</td><td>0.20</td><td>0.0031</td><td>-</td><td>0.0079</td></tr><tr><td>S</td><td>0.20</td><td>-</td><td>-</td><td>0.0079</td><td>-</td><td>-</td></tr><tr><td> $aaa^{(1)(7)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $bbb^{(1)(7)}$ </td><td colspan="3">0.20</td><td colspan="3">0.0079</td></tr><tr><td> $ccc^{(1)(7)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr><tr><td> $ddd^{(1)(7)}$ </td><td colspan="3">0.08</td><td colspan="3">0.0031</td></tr></table>

## Notes:

1. Dimensioning and tolerancing schemes conform to ASME Y14.5M-1994.

2. The Top package body size may be smaller than the bottom package size by as much as 0.15 mm.

3. Datums A-B and D to be determined at datum plane H.

4. To be determined at seating datum plane C.

5. Dimensions D1 and E1 do not include mold flash or protrusions. Allowable mold flash or protrusions is “0.25 mm” per side. D1 and E1 are Maximum plastic body size dimensions including mold mismatch.

6. Details of pin 1 identifier are optional but must be located within the zone indicated.

7. All Dimensions are in millimeters.

8. No intrusion allowed inwards the leads.

9. Dimension “b” does not include dambar protrusion. Allowable dambar protrusion shall not cause the lead width to exceed the maximum “b” dimension by more than 0.08 mm. Dambar cannot be located on the lower radius or the foot. Minimum space between protrusion and an adjacent lead is 0.07 mm for 0.4 mm and 0.5 mm pitch packages.

10. Exact shape of each corner is optional.

11. These dimensions apply to the flat section of the lead between 0.10 mm and 0.25 mm from the lead tip.

12. A1 is defined as the distance from the seating plane to the lowest point on the package body.

13. “N” is the number of terminal positions for the specified body size.

14. Values in inches are converted from mm and rounded to 4 decimal digits.

15. Drawing is not to scale.

Figure 56. LQFP48 – Footprint example  
![](images/28bd634774fe5737cab11ef72ecda73ef6e6b82fa411a5d782353386d6d4ad6b.jpg)  
1. Dimensions are expressed in millimeters.

## 6.10

## Thermal characteristics

The maximum chip junction temperature $( \mathsf { T } _ { \mathsf { J } } \mathsf { m } \mathsf { a } \mathsf { x } )$ must never exceed the values given in Table 9: General operating conditions.

The maximum chip-junction temperature, ${ \sf T } _ { \sf J }$ max, in degrees Celsius, may be calculated using the following equation:

$$
T _ {J} \max = T _ {A} \max + (P _ {D} \max \times \Theta_ {J A})
$$

where:

$\mathsf { T } _ { \mathsf { A } }$ max is the maximum ambient temperature ${ \mathrm { i n } } \ { } ^ { \circ } { \mathsf { C } } ,$

$\Theta _ { \mathsf { J A } }$ is the package junction-to-ambient thermal resistance, in ${ } ^ { \circ } { \mathsf { C } } N ,$

$\mathsf { P } _ { \mathsf { D } }$ max is the sum of $\mathsf { P } _ { \mathsf { I N T } }$ max and $\mathsf { P } _ { | / \mathsf { O } }$ max $( \mathsf { P } _ { \mathsf { D } } \mathsf { m a x } = \mathsf { P } _ { \mathsf { I N T } } \mathsf { m a x } + \mathsf { P } _ { \mathsf { I / O } } \mathsf { m a x } ) ,$

$\mathsf { P } _ { \mathsf { I N T } }$ max is the product of $1 _ { \mathsf { D D } }$ and $\mathsf { V } _ { \mathsf { D D } } .$ , expressed in Watts. This is the maximum chip internal power.

$\mathsf { P } _ { | / \mathsf { O } }$ max represents the maximum power dissipation on output pins where:

$$
P _ {I / O} \max = \Sigma (V _ {O L} \times I _ {O L}) + \Sigma ((V _ {D D} - V _ {O H}) \times I _ {O H}),
$$

taking into account the actual $\mathsf { V } _ { \mathsf { O L } } / \mathsf { I } _ { \mathsf { O L } }$ and $\mathsf { V } _ { \mathsf { O H } } / \mathsf { I } _ { \mathsf { O H } }$ of the I/Os at low and high level in the application.

Table 63. Package thermal characteristics

<table><tr><td>Symbol</td><td>Parameter</td><td>Value</td><td>Unit</td></tr><tr><td rowspan="8"> $\Theta_{JA}$ </td><td>Thermal resistance junction-ambientLFBGA100 - 10 × 10 mm / 0.8 mm pitch</td><td>44</td><td rowspan="8">°C/W</td></tr><tr><td>Thermal resistance junction-ambientLQFP100 - 14 × 14 mm / 0.5 mm pitch</td><td>46</td></tr><tr><td>Thermal resistance junction-ambientUFBGA100 - 7 × 7 mm / 0.5 mm pitch</td><td>59</td></tr><tr><td>Thermal resistance junction-ambientLQFP64 - 10 × 10 mm / 0.5 mm pitch</td><td>45</td></tr><tr><td>Thermal resistance junction-ambientTFBGA64 - 5 × 5 mm / 0.5 mm pitch</td><td>65</td></tr><tr><td>Thermal resistance junction-ambientLQFP48 - 7 × 7 mm / 0.5 mm pitch</td><td>55</td></tr><tr><td>Thermal resistance junction-ambientUFQFPN 48 - 7 × 7 mm / 0.5 mm pitch</td><td>32</td></tr><tr><td>Thermal resistance junction-ambientVFQFPN 36 - 6 × 6 mm / 0.5 mm pitch</td><td>18</td></tr></table>

## 6.10.1 Reference document

JESD51-2 Integrated Circuits Thermal Test Method Environment Conditions - Natural Convection (Still Air). Available from www.jedec.org.

## 6.10.2 Selecting the product temperature range

When ordering the microcontroller, the temperature range is specified in the ordering information scheme shown in Section 7.

Each temperature range suffix corresponds to a specific guaranteed ambient temperature at maximum dissipation and, to a specific maximum junction temperature.

As applications do not commonly use the STM32F103xx at maximum dissipation, it is useful to calculate the exact power consumption and junction temperature to determine which temperature range will be best suited to the application.

The following examples show how to calculate the temperature range needed for a given application.

## Example 1: High-performance application

Assuming the following application conditions:

Maximum ambient temperature $\mathsf { T } _ { \mathsf { A m a x } } = 8 2 ^ { \circ } \mathsf { C }$ (measured according to JESD51-2), $\mathsf { I } _ { \mathsf { D D m a x } } = 5 0 \mathsf { m A } , \mathsf { V } _ { \mathsf { D D } } = 3 . 5 \mathsf { V } ,$ maximum 20 I/Os used at the same time in output at low level with $\mathrm { | _ { O L } = 8 \ m A , V _ { O L } = 0 . 4 V }$ and maximum 8 I/Os used at the same time in output at low level with $\mathrm { \Delta } _ { \mathrm { l _ { O L } } } = 2 0 \ \mathrm { m A } , \mathrm { V _ { O L } } = 1 . 3 \ \mathrm { V }$

$$
P _ {\text { INTmax }} = 5 0 \mathrm{mA} \times 3. 5 \mathrm{V} = 1 7 5 \mathrm{mW}
$$

$$
P _ {I O m a x} = 2 0 \times 8 \mathrm{mA} \times 0. 4 \mathrm{V} + 8 \times 2 0 \mathrm{mA} \times 1. 3 \mathrm{V} = 2 7 2 \mathrm{mW}
$$

$$
\text {This gives:} P _ {\mathrm{INTmax}} = 1 7 5 \mathrm{mW} \text {and} P _ {\mathrm{IOmax}} = 2 7 2 \mathrm{mW}:
$$

$$
P _ {D \max} = 1 7 5 + 2 7 2 = 4 4 7 \mathrm{mW}
$$

Thus: $\mathsf { P } _ { \mathsf { D m a x } } = 4 4 7 \mathsf { m W }$

Using the values obtained in Table $6 3 \top _ { \mathsf { J m a x } }$ is calculated as follows:

$$
- \quad \text { For   LQFP100, } 4 6 ^ {\circ} \mathrm{C} / \mathrm{W}
$$

$$
T _ {J \max} = 8 2 ^ {\circ} C + (4 6 ^ {\circ} C / W \times 4 4 7 m W) = 8 2 ^ {\circ} C + 2 0. 6 ^ {\circ} C = 1 0 2. 6 ^ {\circ} C
$$

This is within the range of the suffix 6 version parts $( - 4 0 < \mathsf { T } _ { \mathsf { J } } < 1 0 5 ^ { \circ } \mathsf { C } )$

In this case, parts must be ordered at least with the temperature range suffix 6 (see Section 7).

## Example 2: High-temperature application

Using the same rules, it is possible to address applications that run at high ambient temperatures with a low dissipation, as long as junction temperature $\mathsf { T } _ { \mathsf { J } }$ remains within the specified range.

Assuming the following application conditions:

Maximum ambient temperature $\mathsf { T } _ { \mathsf { A m a x } } = 1 1 5 ^ { \circ } \mathsf { C }$ (measured according to JESD51-2), $\mathsf { I } _ { \mathsf { D D m a x } } = 2 0 \mathsf { m A } , \mathsf { V } _ { \mathsf { D D } } = 3 . 5 \mathsf { V } ,$ maximum 20 I/Os used at the same time in output at low level with I<sub>OL</sub> = 8 mA, V<sub>OL</sub>= 0.4 V

$$
P _ {\text {INTmax}} = 2 0 \mathrm{mA} \times 3. 5 \mathrm{V} = 7 0 \mathrm{mW}
$$

$$
P _ {I O m a x} = 2 0 \times 8 \mathrm{mA} \times 0. 4 \mathrm{V} = 6 4 \mathrm{mW}
$$

This gives: $\mathsf { P } _ { \mathsf { I N T m a x } } = 7 0 \mathsf { m W a n d } \mathsf { P } _ { 1 0 \mathsf { m a x } } = 6 4 \mathsf { m W } .$

$$
P _ {D m a x} = 7 0 + 6 4 = 1 3 4 \mathrm{mW}
$$

Thus: $\mathsf { P } _ { \mathsf { D m a x } } = 1 3 4 \mathsf { m W }$

Using the values obtained in Table $6 3 \top _ { \mathsf { J m a x } }$ is calculated as follows:

$$
- \quad \text { For   LQFP100, } 4 6 ^ {\circ} \mathrm{C/W}
$$

$$
T _ {J \max} = 1 1 5 ^ {\circ} \mathrm{C} + (4 6 ^ {\circ} \mathrm{C} / \mathrm{W} \times 1 3 4 \mathrm{mW}) = 1 1 5 ^ {\circ} \mathrm{C} + 6. 2 ^ {\circ} \mathrm{C} = 1 2 1. 2 ^ {\circ} \mathrm{C}
$$

This is within the range of the suffix 7 version parts $( - 4 0 < \mathsf { T } _ { \mathsf { J } } < 1 2 5 ^ { \circ } \mathsf { C } )$

In this case, parts must be ordered at least with the temperature range suffix 7 (see Section 7).

Figure 57. LQFP100 $\mathsf { P } _ { \mathsf { D } }$ max vs. $\boldsymbol { \mathsf { T } } _ { \mathsf { A } }$  
![](images/3564a7038c10ed585c6bc7b6cc105b2b48d082de864de15284c2d42af0f8f27b.jpg)

## 7

## Ordering information scheme

![](images/603ff30cb1e3e2f9057974aa5ac85d8d22a35af1466f6901ad6a41108691db25.jpg)

x = Blank for standard product and R for customer dedicated code

For a list of available options (speed, package, etc.) or for further information on any aspect of this device, contact your nearest ST sales office.

## Important security notice

The STMicroelectronics group of companies (ST) places a high value on product security, which is why the ST product(s) identified in this documentation may be certified by various security certification bodies and/or may implement our own security measures as set forth herein. However, no level of security certification and/or built-in security measures can guarantee that ST products are resistant to all forms of attacks. As such, it is the responsibility of each of ST's customers to determine if the level of security provided in an ST product meets the customer needs both in relation to the ST product alone, as well as when combined with other components and/or software for the customer end product or application. In particular, take note that:

ST products may have been certified by one or more security certification bodies, such as Platform Security Architecture (www.psacertified.org) and/or Security Evaluation standard for IoT Platforms (www.trustcb.com). For details concerning whether the ST product(s) referenced herein have received security certification along with the level and current status of such certification, either visit the relevant certification standards website or go to the relevant product page on www.st.com for the most up to date information. As the status and/or level of security certification for an ST product can change from time to time, customers should re-check security certification status/level as needed. If an ST product is not shown to be certified under a particular security standard, customers should not assume it is certified.

Certification bodies have the right to evaluate, grant and revoke security certification in relation to ST products. These certification bodies are therefore independently responsible for granting or revoking security certification for an ST product, and ST does not take any responsibility for mistakes, evaluations, assessments, testing, or other activity carried out by the certification body with respect to any ST product.

Industry-based cryptographic algorithms (such as AES, DES, or MD5) and other open standard technologies which may be used in conjunction with an ST product are based on standards which were not developed by ST. ST does not take responsibility for any flaws in such cryptographic algorithms or open technologies or for any methods which have been or may be developed to bypass, decrypt or crack such algorithms or technologies.

While robust security testing may be done, no level of certification can absolutely guarantee protections against all attacks, including, for example, against advanced attacks which have not been tested for, against new or unidentified forms of attack, or against any form of attack when using an ST product outside of its specification or intended use, or in conjunction with other components or software which are used by customer to create their end product or application. ST is not responsible for resistance against such attacks. As such, regardless of the incorporated security features and/or any information or support that may be provided by ST, each customer is solely responsible for determining if the level of attacks tested for meets their needs, both in relation to the ST product alone and when incorporated into a customer end product or application.

All security features of ST products (inclusive of any hardware, software, documentation, and the like), including but not limited to any enhanced security features added by ST, are provided on an "AS IS" BASIS. AS SUCH, TO THE EXTENT PERMITTED BY APPLICABLE LAW, ST DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, unless the applicable written and signed contract terms specifically provide otherwise.

## Revision history

Table 64. Document revision history

<table><tr><td>Date</td><td>Revision</td><td>Changes</td></tr><tr><td>01-Jun-2007</td><td>1</td><td>Initial release.</td></tr><tr><td>20-Jul-200718-Oct-200722-Nov-2007</td><td>234</td><td>Flash memory size modified in Note 9, Note 5, Note 7, Note 7 and BGA100 pins added to Table 5: Medium-density STM32F103xx pin definitions.Figure 3: STM32F103xx performance line LFBGA100 ballout added. $T_{HSE}$  changed to  $T_{LSE}$  in Figure 23: Low-speed external clock source AC timing diagram.  $V_{BAT}$  ranged modified in Power supply schemes. $t_{SU(LSE)}$  changed to  $t_{SU(HSE)}$  in Table 22: HSE 4-16 MHz oscillator characteristics.  $I_{DD(HSI)}$  max value added to Table 24: HSI oscillator characteristics.Sample size modified and machine model removed in Electrostatic discharge (ESD).Number of parts modified and standard reference updated in Static latch-up. 25 °C and 85 °C conditions removed and class name modified in Table 33: Electrical sensitivities.  $R_{PU}$  and  $R_{PD}$  min and max values added to Table 35: I/O static characteristics.  $R_{PU}$  min and max values added to Table 38: NRST pin characteristics.Figure 32: I2C bus AC waveforms and measurement circuit and Figure 31: Recommended NRST pin protection corrected.Notes removed below Table 9, Table 38, Table 44. $I_{DD}$  typical values changed in Table 11: Maximum current consumption in Run and Sleep modes. Table 39: TIMx characteristics modified. $t_{STAB}$ ,  $V_{REF+}$  value,  $t_{lat}$  and  $f_{TRIG}$  added to Table 46: ADC characteristics.In Table: typical endurance and data retention for  $T_A = 85 °C$  added, data retention for  $T_A = 25 °C$  removed. $V_{BG}$  changed to  $V_{REFINT}$  in Table 12: Embedded internal reference voltage. Document title changed. Controller area network (CAN) section modified.Figure 14: Power supply scheme modified.Features on page 1 list optimized. Small text changes.STM32F103CBT6, STM32F103T6 and STM32F103T8 root part numbers added (see Table 2: STM32F103xx medium-density device features and peripheral counts)VFQFPN36 package added (see Section 6: Package information). All packages are ECOPACK® compliant. Package mechanical data inch values are calculated from mm and rounded to 4 decimal digits (see Section 6: Package information).Table 5: Medium-density STM32F103xx pin definitions updated and clarified.Table 26: Low-power mode wakeup timings updated.Tamin corrected in Table 12: Embedded internal reference voltage.Note 2 added below Table 22: HSE 4-16 MHz oscillator characteristics. $V_{ESD(CDM)}$  value added to Table 32: ESD absolute maximum ratings.Note 4 added and  $V_{OH}$  parameter description modified in Table 36: Output voltage characteristics.Note 1 modified under Table 37: I/O AC characteristics.Equation 1 and Table 47:  $R_{AIN}$  max for  $f_{ADC} = 14$  MHz added to Section 5.3.18: 12-bit ADC characteristics. $V_{AIN}$ ,  $t_S$  max,  $t_{CONV}$ ,  $V_{REF+}$  min and  $t_{lat}$  max modified, notes modified and  $t_{latr}$  added in Table 46: ADC characteristics.Figure 37: ADC accuracy characteristics updated. Note 1 modified below Figure 38: Typical connection diagram using the ADC.Electrostatic discharge (ESD) on page 59 modified.Number of TIM4 channels modified in Figure 1: STM32F103xx performance line block diagram.Maximum current consumption Table 13, Table 14 and Table 15 updated. $V_{hys}$  modified in Table 35: I/O static characteristics.Table 49: ADC accuracy updated.  $V_{FESD}$  value added in Table 30: EMS characteristics. Values corrected, note 2 modified and note 3 removed in Table 26: Low-power mode wakeup timings.Table 16: Typical and maximum current consumptions in Stop and Standby modes: Typical values added for  $V_{DD}/V_{BAT} = 2.4$  V, Note 2 modified, Note 2 added.Table 21: Typical current consumption in Standby mode added.On-chip peripheral current consumption on page 49 added.ACC $_{HSI}$  values updated in Table 24: HSI oscillator characteristics. $V_{prog}$  added to Table 28: Flash memory characteristics.Upper option byte address modified in Figure 11: Memory map.Typical  $f_{LSI}$  value added in Table 25: LSI oscillator characteristics and internal RC value corrected from 32 to 40 kHz in entire document. $T_{S\_temp}$  added to Table 50: TS characteristics.  $N_{END}$  modified in Table. $T_{S\_vrefint}$  added to Table 12: Embedded internal reference voltage.Handling of unused pins specified in General input/output characteristics on page 61. All I/Os are CMOS and TTL compliant.Figure 39: Power supply and reference decoupling ( $V_{REF+}$  not connected to  $V_{DDA}$ ) modified. $t_{JITTER}$  and  $f_{VCO}$  removed from Table 27: PLL characteristics. Appendix A: Important notes on page 81 added.Added Figure 16, Figure 17, Figure 19 and Figure 21.Document status promoted from preliminary data to datasheet. STM32F103xx is USB certified. Small text changes. Power supply schemes on page 15 modified. Number of communication peripherals corrected for STM32F103Tx and number of GPIOs corrected for LQFP package in Table 2: STM32F103xx medium-density device features and peripheral counts. Main function and default alternate function modified for PC14 and PC15 in, Note 6 added and Remap column added in Table 5: Medium-density STM32F103xx pin definitions. $V_{DD}-V_{SS}$  ratings and Note 1 modified in Table 6: Voltage characteristics, Note 1 modified in Table 7: Current characteristics. Note 1 and Note 2 added in Table 11: Embedded reset and power control block characteristics. $I_{DD}$  value at 72 MHz with peripherals enabled modified in Table 14: Maximum current consumption in Run mode, code with data processing running from RAM. $I_{DD}$  value at 72 MHz with peripherals enabled modified in Table 15: Maximum current consumption in Sleep mode, code running from Flash or RAM on page 43. $I_{DD\_VBAT}$  typical value at 2.4 V modified and  $I_{DD\_VBAT}$  maximum values added in Table 16: Typical and maximum current consumptions in Stop and Standby modes. Note added in Table 17 on page 47 and Table 18 on page 48. ADC1 and ADC2 consumption and notes modified in Table 19: Peripheral current consumption. $t_{SU(HSE)}$  and  $t_{SU(LSE)}$  conditions modified in Table 22 and Table 23, respectively.Maximum values removed from Table 26: Low-power mode wakeup timings.  $t_{RET}$  conditions modified in Table. Figure 14: Power supply scheme corrected.Figure 20: Typical current consumption in Stop mode, with regulator in Low-power mode added.Note removed below Figure 33: SPI timing diagram - slave mode and CPHA = 0. Note added below Figure 34: SPI timing diagram - slave mode and CPHA = 1(1).Details on unused pins removed from General input/output characteristics on page 61.Table 42: SPI characteristics updated. Table 43: USB startup time added. $V_{AIN}$ ,  $t_{lat}$  and  $t_{latr}$  modified, note added and  $I_{Ikg}$  removed in Table 46: ADC characteristics. Test conditions modified and note added in Table 49: ADC accuracy. Note added below Table 47 and Table 50.Inch values corrected in Table 55: LQPF100 mechanical data, Table 58: LQFP64 mechanical data and Table 60: LQFP48, 7 x 7 mm, 48-pin low-profile quad flat package mechanical data. $\Theta_{JA}$  value for VFQFPN36 package added in Table 62: Package thermal characteristics.Order codes replaced by Section 7: Ordering information scheme.MCU 's operating conditions modified in Typical current consumption on page 46. Avg_Slope and  $V_{25}$  modified in Table 50: TS characteristics. I2C interface characteristics on page 68 modified.Impedance specified in A.4: Voltage glitch on ADC input 0 on page 81.</td></tr><tr><td>14-Mar-2008</td><td>5</td><td>Figure 2: Clock tree on page 12 added.Maximum  $T_J$  value given in Table 8: Thermal characteristics on page 37.CRC feature added (see CRC (cyclic redundancy check) calculation unit on page 9 and Figure 11: Memory map on page 34 for address). $I_{DD}$  modified in Table 16: Typical and maximum current consumptions in Stop and Standby modes.ACC $_{HSI}$  modified in Table 24: HSI oscillator characteristics on page 54, note 2 removed. $P_D$ ,  $T_A$  and  $T_J$  added,  $t_{prog}$  values modified and  $t_{prog}$  description clarified in Table 28: Flash memory characteristics on page 56. $t_{RET}$  modified in Table. $V_{NF(NRST)}$  unit corrected in Table 38: NRST pin characteristics on page 66.Table 42: SPI characteristics on page 70 modified. $I_{VREF}$  added to Table 46: ADC characteristics on page 74.Table 48: ADC accuracy - Limited test conditions added. Table 49: ADC accuracy modified.LQFP100 package specifications updated (see Section 6: Package information on page 79).Recommended LQFP100, LQFP 64, LQFP48 and VFQFPN36 footprints added (see Figure 55, Figure 60, Figure 64 and Figure 44).Section 6.9: Thermal characteristics on page 104 modified, Section 6.9.1 and Section 6.9.2 added.Appendix A: Important notes on page 81 removed.</td></tr><tr><td>21-Mar-2008</td><td>6</td><td>Small text changes. Figure 11: Memory map clarified.In Table: $N_{END}$  tested over the whole temperature range, cycling conditions specified for  $t_{RET}$ ,  $t_{RET}$  min modified at  $T_A = 55 °C$  $V_{25}$ , Avg_Slope and  $T_L$  modified in Table 50: TS characteristics.CRC feature removed.</td></tr><tr><td>22-May-2008</td><td>7</td><td>CRC feature added back. Small text changes. Section 1: Introduction modified. Section 2.2: Full compatibility throughout the family added. $I_{DD}$  at  $T_A$  max = 105 °C added to Table 16: Typical and maximum current consumption in Stop and Standby modes on page 44. $I_{DD\_VBAT}$  removed from Table 21: Typical current consumption in Standby mode on page 47.Values added to Table 41: SCL frequency ( $f_{PCLK1} = 36 MHz$ ,  $V_{DD\_I2C} = 3.3 V$ ) on page 69.Figure 33: SPI timing diagram - slave mode and CPHA = 0 on page 71 modified. Equation 1 corrected. $t_{RET}$  at  $T_A = 105 °C$  modified in Table on page 57. $V_{USB}$  added to Table 44: USB DC electrical characteristics on page 73.Figure 65: LQFP100  $P_D$  max vs.  $T_A$  on page 106 modified.Axx option added to Table 63: Ordering information scheme on page 110.</td></tr><tr><td>21-Jul-2008</td><td>8</td><td>Power supply supervisor updated and  $V_{DDA}$  added to Table 9: General operating conditions.Capacitance modified in Figure 14: Power supply scheme on page 36.Table notes revised in Section 5: Electrical characteristics.Table 16: Typical and maximum current consumptions in Stop and Standby modes modified.Data added to Table 16: Typical and maximum current consumptions in Stop and Standby modes and Table 21: Typical current consumption in Standby mode removed. $f_{HSE\_ext}$  modified in Table 20: High-speed external user clock characteristics on page 50.  $f_{PLL\_IN}$  modified in Table 27: PLL characteristics on page 56.Minimum SDA and SCL fall time value for Fast mode removed from Table 40: I2C characteristics on page 68, note 1 modified. $t_{h(NSS)}$  modified in Table 42: SPI characteristics on page 70 and Figure 33: SPI timing diagram - slave mode and CPHA = 0 on page 71. $C_{ADC}$  modified in Table 46: ADC characteristics on page 74 and Figure 38: Typical connection diagram using the ADC modified.Typical  $T_{S\_temp}$  value removed from Table 50: TS characteristics on page 78.LQFP48 package specifications updated (see Table 60 and Table 64),Section 6: Package information revised.Axx option removed from Table 63: Ordering information scheme on page 110.Small text changes.</td></tr><tr><td>22-Sep-2008</td><td>9</td><td>STM32F103x6 part numbers removed (see Table 63: Ordering information scheme). Small text changes.General-purpose timers (TIMx) and Advanced-control timer (TIM1) on page 18 updated.Notes updated in Table 5: Medium-density STM32F103xx pin definitions on page 28.Note 2 modified below Table 6: Voltage characteristics on page 37,  $|\Delta V_{DDx}|$  min and  $|\Delta V_{DDx}|$  min removed.Measurement conditions specified in Section 5.3.5: Supply current characteristics on page 40. $I_{DD}$  in standby mode at 85 °C modified in Table 16: Typical and maximum current consumptions in Stop and Standby modes on page 44.General input/output characteristics on page 61 modified. $f_{HCLK}$  conditions modified in Table 30: EMS characteristics on page 58. $\Theta_{JA}$  and pitch value modified for LFBGA100 package in Table 62:Package thermal characteristics. Small text changes.</td></tr><tr><td>23-Apr-2009</td><td>10</td><td>I/O information clarified on page 1.Figure 3: STM32F103xx performance line LFBGA100 ballout modified.Figure 11: Memory map modified.Table 4: Timer feature comparison added.PB4, PB13, PB14, PB15, PB3/TRACESWO moved from Default column to Remap column in Table 5: Medium-density STM32F103xx pin definitions. $P_D$  for LFBGA100 corrected in Table 9: General operating conditions.Note modified in Table 13: Maximum current consumption in Run mode, code with data processing running from Flash and Table 15: Maximum current consumption in Sleep mode, code running from Flash or RAM.Table 20: High-speed external user clock characteristics and Table 21: Low-speed external user clock characteristics modified.Figure 20 shows a typical curve (title modified).  $ACC_{HSI}$  max values modified in Table 24: HSI oscillator characteristics.TFBGA64 package added (see Table 59 and Table 60). Small text changes.</td></tr><tr><td>22-Sep-2009</td><td>11</td><td>Note 5 updated and Note 4 added in Table 5: Medium-density STM32F103xx pin definitions. $V_{RERINT}$  and  $T_{Coeff}$  added to Table 12: Embedded internal reference voltage.  $I_{DD\_VBAT}$  value added to Table 16: Typical and maximum current consumptions in Stop and Standby modes. Figure 18: Typical current consumption on  $V_{BAT}$  (RTC on) added. $f_{HSE\_ext}$  min modified in Table 20: High-speed external user clock characteristics. $C_{L1}$  and  $C_{L2}$  replaced by C in Table 22: HSE 4-16 MHz oscillator characteristics and Table 23: LSE oscillator characteristics ( $f_{LSE} = 32.768$  kHz), notes modified and moved below the tables. Table 24: HSI oscillator characteristics modified. Conditions removed from Table 26: Low-power mode wakeup timings.Note 1 modified below Figure 24: Typical application with an 8 MHz crystal.IEC 1000 standard updated to IEC 61000 and SAE J1752/3 updated to IEC 61967-2 in Section 5.3.10: EMC characteristics on page 57.Jitter added to Table 27: PLL characteristics.Table 42: SPI characteristics modified. $C_{ADC}$  and  $R_{AIN}$  parameters modified in Table 46: ADC characteristics. $R_{AIN}$  max values modified in Table 47:  $R_{AIN}$  max for  $f_{ADC} = 14$  MHz.Figure 47: LFBGA100 outline updated.</td></tr><tr><td>03-Jun-2010</td><td>12</td><td>Added STM32F103TB devices.Added VFQFPN48 package.Updated note 2 below Table 40:  $I^2C$  characteristicsUpdated Figure 32:  $I^2C$  bus AC waveforms and measurement circuitUpdated Figure 31: Recommended NRST pin protectionUpdated Section 5.3.12: I/O current injection characteristics</td></tr><tr><td>19-Apr-2011</td><td>13</td><td>Updated footnotes below Table 6: Voltage characteristics on page 37 and Table 7: Current characteristics on page 37Updated tw min in Table 20: High-speed external user clock characteristics on page 50Updated startup time in Table 23: LSE oscillator characteristics ( $f_{LSE} = 32.768$  kHz) on page 53Added Section 5.3.12: I/O current injection characteristicsUpdated Section 5.3.13: I/O port characteristics</td></tr><tr><td>07-Dec-2012</td><td>14</td><td>Added UFBGA100 7 x 7 mm.Updated Figure 59: LQFP64, 10 x 10 mm, 64-pin low-profile quad flat package outline to add pin 1 identification.</td></tr><tr><td>14-May-2013</td><td>15</td><td>Replaced VQFN48 package with UQFN48 in cover page packages, Table 2: STM32F103xx medium-density device features and peripheral counts, Figure 9: STM32F103xx performance line UFQFPN48 pinout, Table 2: STM32F103xx medium-density device features and peripheral counts, Table 56: UFBGA100 mechanical data, Table 63: Ordering information scheme and updated Table 62: Package thermal characteristicsAdded footnote for TFBGA ADC channels in Table 2: STM32F103xx medium-density device features and peripheral countsUpdated ‘All GPIOs are high current...’ in Section 2.3.21: GPIOs (general-purpose inputs/outputs)Updated Table 5: Medium-density STM32F103xx pin definitionsCorrected Sigma letter in Section 5.1.1: Minimum and maximum valuesRemoved the first sentence in Section 5.3.16: Communications interfacesAdded ‘ $V_{IN}$ ’ in Table 9: General operating conditionsUpdated first sentence in Output driving currentAdded note 5. in Table 24: HSI oscillator characteristicsUpdated ‘ $V_{IL}$ ’ and ‘ $V_{IH}$ ’ in Table 35: I/O static characteristicsAdded notes to Figure 26: Standard I/O input characteristics - CMOS port, Figure 27: Standard I/O input characteristics - TTL port, Figure 28: 5 V tolerant I/O input characteristics - CMOS port and Figure 29: 5 V tolerant I/O input characteristics - TTL portUpdated Figure 32:  $I^{2}C$  bus AC waveforms and measurement circuitUpdated notes 2 and 3,removed note “the device must internally...” in Table 40:  $I^{2}C$  characteristicsUpdated title of Table 41: SCL frequency ( $f_{PCLK1} = 36$  MHz,  $V_{DD\_I2C} = 3.3$  V)Updated note 2. in Table 49: ADC accuracyUpdated Figure 53: UFBGA100 - 100-ball, 7 x 7 mm, 0.50 mm pitch, ultra fine pitch ball grid array package outline and Table 56: UFBGA100 mechanical dataUpdated Figure 47: LFBGA100 outline and Table 53: LFBGA100 mechanical dataUpdated Figure 60: TFBGA64 - 8 x 8 active ball array, 5 x 5 mm, 0.5 mm pitch, package outline and Table 59: TFBGA64 - 8 x 8 active ball array, 5 x 5 mm, 0.5 mm pitch, package mechanical data</td></tr><tr><td>05-Aug-2013</td><td>16</td><td>Updated the reference for ‘ $V_{ESD(CDM)}$ ’ in Table 32: ESD absolute maximum ratingsCorrected ‘tf(IO)out’ in Figure 30: I/O AC characteristics definitionUpdated Table 52: UFQFPN48 mechanical data</td></tr><tr><td>21-Aug-2015</td><td>17</td><td>Updated Table 3: STM32F103xx family removing the note.Updated Table 63: Ordering information scheme removing the note.Updated Section 6: Package information and added Section: Marking of engineering samples for all packages.Updated  $I^{2}C$  characteristics, added  $t_{SP}$  parameter and note 4 in Table 40:  $I^{2}C$  characteristics.Updated Figure 32:  $I^{2}C$  bus AC waveforms and measurement circuit swapping SCLL and SCLH.Updated Figure 33: SPI timing diagram - slave mode and CPHA = 0.Updated min/max value notes replacing ‘Guaranteed by design, not tested in production” by “guaranteed by design”.Updated min/max value notes replacing ‘based on characterization, not tested in production” by “Guaranteed based on test during characterization”.Updated Table 19: Peripheral current consumption.</td></tr><tr><td>29-Mar-2022</td><td>18</td><td>Updated Table 5: Medium-density STM32F103xx pin definitions.Updated Figure 37: ADC accuracy characteristics, Figure 38: Typical connection diagram using the ADC and its footnotes.Minor text edits across the whole document.</td></tr><tr><td>18-Sep-2023</td><td>19</td><td>Updated Features.Updated Section 1: Introduction.Updated Figure 11: Memory map.Updated Table 24: HSE 4-16 MHz oscillator characteristics and Table 25: LSE oscillator characteristics ( $f_{LSE} = 32.768 \text{ kHz}$ ).Updated Table 33: EMI characteristics for fHSE = 8 MHz and fHCLK = 48 MHz and created Table 34: EMI characteristics for fHSE = 8 MHz and fHCLK = 72 MHz.Updated Figure 33: SPI timing diagram - slave mode and CPHA = 0, Figure 34: SPI timing diagram - slave mode and CPHA = 1, and Figure 35: SPI timing diagram - master mode.Updated Table 46: USB startup time.Added Section 8: Important security notice.Updated all packages in Section 6: Package information.</td></tr><tr><td>28-Jul-2025</td><td>20</td><td>Updated Section 7: Ordering information scheme.</td></tr></table>

## IMPORTANT NOTICE – PLEASE READ CAREFULLY

STMicroelectronics NV and its subsidiaries (“ST”) reserve the right to make changes, corrections, enhancements, modifications, and improvements to ST products and/or to this document at any time without notice. Purchasers should obtain the latest relevant information on ST products before placing orders. ST products are sold pursuant to ST’s terms and conditions of sale in place at the time of order acknowledgement.

Purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or the design of Purchasers’ products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

ST and the ST logo are trademarks of ST. For additional information about ST trademarks, please refer to www.st.com/trademarks. All other product or service names are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2025 STMicroelectronics – All rights reserved