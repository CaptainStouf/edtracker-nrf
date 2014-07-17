EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:edtracker_nrf
LIBS:nrf24le1
LIBS:edtracker_nrf-cache
EELAYER 24 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MPU-6050 U2
U 1 1 53A8761E
P 4800 4350
F 0 "U2" H 4800 4700 60  0000 C CNN
F 1 "MPU-6050" H 4800 4000 60  0000 C CNN
F 2 "" H 4800 4350 60  0000 C CNN
F 3 "" H 4800 4350 60  0000 C CNN
	1    4800 4350
	1    0    0    -1  
$EndComp
$Comp
L NRF24LE1_PROG U1
U 1 1 53A87671
P 1900 2550
F 0 "U1" H 1900 2200 60  0000 C CNN
F 1 "NRF24LE1_PROG" H 1900 2900 60  0000 C CNN
F 2 "" H 1850 2450 60  0000 C CNN
F 3 "" H 1850 2450 60  0000 C CNN
	1    1900 2550
	1    0    0    -1  
$EndComp
$Comp
L UART_CONN U3
U 1 1 53A876E3
P 6550 2400
F 0 "U3" H 6550 2200 60  0000 C CNN
F 1 "UART" H 6550 2600 60  0000 C CNN
F 2 "" H 6400 2375 60  0000 C CNN
F 3 "" H 6400 2375 60  0000 C CNN
	1    6550 2400
	1    0    0    -1  
$EndComp
$Comp
L NRF24LE1_SMALL M1
U 1 1 53A87AFB
P 4400 2650
F 0 "M1" H 4350 1900 60  0000 C CNN
F 1 "NRF24LE1_SMALL" H 4400 3450 60  0000 C CNN
F 2 "" H 4150 2850 60  0000 C CNN
F 3 "" H 4150 2850 60  0000 C CNN
	1    4400 2650
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR01
U 1 1 53A87CCB
P 7050 2200
F 0 "#PWR01" H 7050 2300 30  0001 C CNN
F 1 "VCC" H 7050 2300 30  0000 C CNN
F 2 "" H 7050 2200 60  0000 C CNN
F 3 "" H 7050 2200 60  0000 C CNN
	1    7050 2200
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR02
U 1 1 53A87D52
P 1200 2200
F 0 "#PWR02" H 1200 2300 30  0001 C CNN
F 1 "VCC" H 1200 2300 30  0000 C CNN
F 2 "" H 1200 2200 60  0000 C CNN
F 3 "" H 1200 2200 60  0000 C CNN
	1    1200 2200
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR03
U 1 1 53A87DA0
P 4200 4050
F 0 "#PWR03" H 4200 4150 30  0001 C CNN
F 1 "VCC" H 4200 4150 30  0000 C CNN
F 2 "" H 4200 4050 60  0000 C CNN
F 3 "" H 4200 4050 60  0000 C CNN
	1    4200 4050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 53A87E30
P 3500 3400
F 0 "#PWR04" H 3500 3400 30  0001 C CNN
F 1 "GND" H 3500 3330 30  0001 C CNN
F 2 "" H 3500 3400 60  0000 C CNN
F 3 "" H 3500 3400 60  0000 C CNN
	1    3500 3400
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 53A87E47
P 4200 4300
F 0 "#PWR05" H 4200 4300 30  0001 C CNN
F 1 "GND" H 4200 4230 30  0001 C CNN
F 2 "" H 4200 4300 60  0000 C CNN
F 3 "" H 4200 4300 60  0000 C CNN
	1    4200 4300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 53A87E57
P 7050 2600
F 0 "#PWR06" H 7050 2600 30  0001 C CNN
F 1 "GND" H 7050 2530 30  0001 C CNN
F 2 "" H 7050 2600 60  0000 C CNN
F 3 "" H 7050 2600 60  0000 C CNN
	1    7050 2600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 53A87E67
P 10900 5200
F 0 "#PWR07" H 10900 5200 30  0001 C CNN
F 1 "GND" H 10900 5130 30  0001 C CNN
F 2 "" H 10900 5200 60  0000 C CNN
F 3 "" H 10900 5200 60  0000 C CNN
	1    10900 5200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR08
U 1 1 53A87E77
P 1200 2900
F 0 "#PWR08" H 1200 2900 30  0001 C CNN
F 1 "GND" H 1200 2830 30  0001 C CNN
F 2 "" H 1200 2900 60  0000 C CNN
F 3 "" H 1200 2900 60  0000 C CNN
	1    1200 2900
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR09
U 1 1 53A87E87
P 3500 3100
F 0 "#PWR09" H 3500 3200 30  0001 C CNN
F 1 "VCC" H 3500 3200 30  0000 C CNN
F 2 "" H 3500 3100 60  0000 C CNN
F 3 "" H 3500 3100 60  0000 C CNN
	1    3500 3100
	1    0    0    -1  
$EndComp
Text Label 2450 2400 0    60   ~ 0
FSCK
Text Label 2450 2600 0    60   ~ 0
FMISO
Text Label 2450 2700 0    60   ~ 0
FCSN
Text Label 5150 2750 0    60   ~ 0
FMOSI
Text Label 5150 2550 0    60   ~ 0
FSCK
$Comp
L LED D1
U 1 1 53A99C66
P 5750 1050
F 0 "D1" H 5750 1150 50  0000 C CNN
F 1 "LED" H 5750 950 50  0000 C CNN
F 2 "" H 5750 1050 60  0000 C CNN
F 3 "" H 5750 1050 60  0000 C CNN
	1    5750 1050
	1    0    0    -1  
$EndComp
$Comp
L LED D2
U 1 1 53A99CDF
P 5750 1350
F 0 "D2" H 5750 1450 50  0000 C CNN
F 1 "LED" H 5750 1250 50  0000 C CNN
F 2 "" H 5750 1350 60  0000 C CNN
F 3 "" H 5750 1350 60  0000 C CNN
	1    5750 1350
	1    0    0    -1  
$EndComp
$Comp
L LED D3
U 1 1 53A99CF8
P 5750 1650
F 0 "D3" H 5750 1750 50  0000 C CNN
F 1 "LED" H 5750 1550 50  0000 C CNN
F 2 "" H 5750 1650 60  0000 C CNN
F 3 "" H 5750 1650 60  0000 C CNN
	1    5750 1650
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 53A99DAC
P 6400 1050
F 0 "R1" V 6480 1050 40  0000 C CNN
F 1 "R" V 6407 1051 40  0000 C CNN
F 2 "" V 6330 1050 30  0000 C CNN
F 3 "" H 6400 1050 30  0000 C CNN
	1    6400 1050
	0    1    1    0   
$EndComp
$Comp
L R R2
U 1 1 53A99E23
P 6400 1350
F 0 "R2" V 6480 1350 40  0000 C CNN
F 1 "R" V 6407 1351 40  0000 C CNN
F 2 "" V 6330 1350 30  0000 C CNN
F 3 "" H 6400 1350 30  0000 C CNN
	1    6400 1350
	0    1    1    0   
$EndComp
$Comp
L R R3
U 1 1 53A99E55
P 6400 1650
F 0 "R3" V 6480 1650 40  0000 C CNN
F 1 "R" V 6407 1651 40  0000 C CNN
F 2 "" V 6330 1650 30  0000 C CNN
F 3 "" H 6400 1650 30  0000 C CNN
	1    6400 1650
	0    1    1    0   
$EndComp
$Comp
L GND #PWR010
U 1 1 53A9AAE4
P 6900 1850
F 0 "#PWR010" H 6900 1850 30  0001 C CNN
F 1 "GND" H 6900 1780 30  0001 C CNN
F 2 "" H 6900 1850 60  0000 C CNN
F 3 "" H 6900 1850 60  0000 C CNN
	1    6900 1850
	1    0    0    -1  
$EndComp
Text Label 4000 4450 0    60   ~ 0
SCL
Text Label 4000 4550 0    60   ~ 0
SDA
Text Label 3400 2250 0    60   ~ 0
SCL
Text Label 3400 2350 0    60   ~ 0
SDA
$Comp
L IRF7606PBF Q1
U 1 1 53A9CF7B
P 9050 4250
F 0 "Q1" H 8885 4535 40  0000 C CNN
F 1 "TPS1101D" H 9050 4015 40  0000 C CNN
F 2 "MICRO8" H 9050 4450 30  0000 C CIN
F 3 "" V 9050 4250 60  0000 C CNN
	1    9050 4250
	1    0    0    -1  
$EndComp
$Comp
L BATTERY BT2
U 1 1 53A9ABA8
P 10350 4700
F 0 "BT2" H 10350 4900 50  0000 C CNN
F 1 "BAT_EXT" H 10350 4510 50  0000 C CNN
F 2 "" H 10350 4700 60  0000 C CNN
F 3 "" H 10350 4700 60  0000 C CNN
	1    10350 4700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR011
U 1 1 53A9B643
P 8550 4550
F 0 "#PWR011" H 8550 4550 30  0001 C CNN
F 1 "GND" H 8550 4480 30  0001 C CNN
F 2 "" H 8550 4550 60  0000 C CNN
F 3 "" H 8550 4550 60  0000 C CNN
	1    8550 4550
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR012
U 1 1 53A9B90A
P 7200 3900
F 0 "#PWR012" H 7200 4000 30  0001 C CNN
F 1 "VCC" H 7200 4000 30  0000 C CNN
F 2 "" H 7200 3900 60  0000 C CNN
F 3 "" H 7200 3900 60  0000 C CNN
	1    7200 3900
	1    0    0    -1  
$EndComp
$Comp
L CP1 C2
U 1 1 53A9BE51
P 6750 4350
F 0 "C2" H 6800 4450 50  0000 L CNN
F 1 "CP1" H 6800 4250 50  0000 L CNN
F 2 "" H 6750 4350 60  0000 C CNN
F 3 "" H 6750 4350 60  0000 C CNN
	1    6750 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 2350 6200 2350
Wire Wire Line
	9350 4150 10050 4150
Wire Wire Line
	9650 4250 9350 4250
Wire Wire Line
	3500 3100 3500 3150
Wire Wire Line
	3500 3150 3650 3150
Wire Wire Line
	3650 3250 3500 3250
Wire Wire Line
	3500 3250 3500 3400
Wire Wire Line
	1400 2700 1200 2700
Wire Wire Line
	1200 2700 1200 2900
Wire Wire Line
	1200 2200 1200 2400
Wire Wire Line
	1200 2400 1400 2400
Wire Wire Line
	10650 4150 10900 4150
Wire Wire Line
	10900 4150 10900 5200
Wire Wire Line
	7050 2200 7050 2350
Wire Wire Line
	7050 2350 6900 2350
Wire Wire Line
	6900 2450 7050 2450
Wire Wire Line
	7050 2450 7050 2600
Wire Wire Line
	2400 2400 2700 2400
Wire Wire Line
	2400 2600 2850 2600
Wire Wire Line
	2400 2700 2950 2700
Wire Wire Line
	2300 2850 3650 2850
Wire Wire Line
	2450 2950 3650 2950
Wire Wire Line
	1000 2500 1400 2500
Wire Wire Line
	1100 2600 1400 2600
Wire Wire Line
	4350 4150 4200 4150
Wire Wire Line
	4200 4150 4200 4050
Wire Wire Line
	4350 4250 4200 4250
Wire Wire Line
	4200 4250 4200 4300
Wire Wire Line
	5150 2650 5650 2650
Wire Wire Line
	5150 2750 5500 2750
Wire Wire Line
	5150 2550 6000 2550
Wire Wire Line
	3650 2050 2850 2050
Wire Wire Line
	3650 2150 2950 2150
Wire Wire Line
	2950 2150 2950 2700
Wire Wire Line
	5150 2050 5250 2050
Wire Wire Line
	5250 2050 5250 1050
Wire Wire Line
	5250 1050 5550 1050
Wire Wire Line
	5550 1350 5350 1350
Wire Wire Line
	5350 1350 5350 2150
Wire Wire Line
	5350 2150 5150 2150
Wire Wire Line
	5150 2250 5450 2250
Wire Wire Line
	5450 2250 5450 1650
Wire Wire Line
	5450 1650 5550 1650
Wire Wire Line
	5950 1050 6150 1050
Wire Wire Line
	6150 1350 5950 1350
Wire Wire Line
	5950 1650 6150 1650
Wire Wire Line
	6650 1050 6900 1050
Wire Wire Line
	6900 1050 6900 1850
Wire Wire Line
	6650 1350 6900 1350
Connection ~ 6900 1350
Wire Wire Line
	6650 1650 6900 1650
Connection ~ 6900 1650
Wire Wire Line
	3150 4450 4350 4450
Wire Wire Line
	3250 4550 4350 4550
Wire Wire Line
	3150 2250 3650 2250
Wire Wire Line
	3250 2350 3650 2350
Wire Wire Line
	5650 2650 5650 4550
Wire Wire Line
	5650 4550 5250 4550
Wire Wire Line
	3250 2350 3250 4550
Wire Wire Line
	3150 4450 3150 2250
Wire Wire Line
	6000 3150 6150 3150
Wire Wire Line
	2450 2950 2450 3050
Wire Wire Line
	2450 3050 1100 3050
Wire Wire Line
	1100 3050 1100 2600
Wire Wire Line
	1000 2500 1000 3150
Wire Wire Line
	1000 3150 2300 3150
Wire Wire Line
	2300 3150 2300 2850
Wire Wire Line
	9350 4050 9650 4050
Wire Wire Line
	9650 4050 9650 4700
Wire Wire Line
	9650 4350 9350 4350
Connection ~ 9650 4250
Connection ~ 9650 4150
Wire Wire Line
	10650 4700 10900 4700
Connection ~ 10900 4700
Wire Wire Line
	7200 4400 8750 4400
Wire Wire Line
	6350 4050 7350 4050
Wire Wire Line
	8550 4050 8750 4050
Wire Wire Line
	6750 4150 6750 4050
Wire Wire Line
	6750 4550 6750 4650
Wire Wire Line
	6350 4650 7200 4650
$Comp
L C C1
U 1 1 53A9C242
P 6350 4350
F 0 "C1" H 6350 4450 40  0000 L CNN
F 1 "C" H 6356 4265 40  0000 L CNN
F 2 "" H 6388 4200 30  0000 C CNN
F 3 "" H 6350 4350 60  0000 C CNN
	1    6350 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 4150 6350 4050
Connection ~ 6750 4050
Wire Wire Line
	6350 4650 6350 4550
Connection ~ 6750 4650
Wire Wire Line
	6750 3150 6900 3150
Wire Wire Line
	6900 3150 6900 3450
$Comp
L GND #PWR013
U 1 1 53AABDAA
P 6900 3450
F 0 "#PWR013" H 6900 3450 30  0001 C CNN
F 1 "GND" H 6900 3380 30  0001 C CNN
F 2 "" H 6900 3450 60  0000 C CNN
F 3 "" H 6900 3450 60  0000 C CNN
	1    6900 3450
	1    0    0    -1  
$EndComp
$Comp
L SWITCH_INV SW2
U 1 1 53AAC4C3
P 7850 4050
F 0 "SW2" H 7650 4200 50  0000 C CNN
F 1 "SWITCH_INV" H 7700 3900 50  0000 C CNN
F 2 "" H 7850 4050 60  0000 C CNN
F 3 "" H 7850 4050 60  0000 C CNN
	1    7850 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	8550 4400 8550 4550
Wire Wire Line
	7200 3900 7200 4050
Connection ~ 7200 4050
Wire Wire Line
	8550 4250 8750 4250
Wire Wire Line
	8350 4150 8750 4150
Wire Wire Line
	8550 4050 8550 4250
Connection ~ 8550 4150
Wire Wire Line
	7200 4650 7200 4400
Connection ~ 8550 4400
$Comp
L SW_PUSH SW1
U 1 1 53AB20EC
P 6450 3150
F 0 "SW1" H 6600 3260 50  0000 C CNN
F 1 "SW_PUSH" H 6450 3070 50  0000 C CNN
F 2 "" H 6450 3150 60  0000 C CNN
F 3 "" H 6450 3150 60  0000 C CNN
	1    6450 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 2550 6000 3150
Wire Wire Line
	2850 2050 2850 2600
Wire Wire Line
	2400 2500 2700 2500
Text Label 2450 2500 0    60   ~ 0
FMOSI
$Comp
L BATTERY BT1
U 1 1 53C5091B
P 10350 4150
F 0 "BT1" H 10350 4350 50  0000 C CNN
F 1 "BAT_CR123" H 10350 3960 50  0000 C CNN
F 2 "" H 10350 4150 60  0000 C CNN
F 3 "" H 10350 4150 60  0000 C CNN
	1    10350 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9650 4700 10050 4700
Connection ~ 9650 4350
Text Label 2550 2850 0    60   ~ 0
PROG
Text Label 2550 2950 0    60   ~ 0
RESET
$EndSCHEMATC
