EESchema Schematic File Version 4
EELAYER 30 0
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
Entry Wire Line
	5750 5950 5850 6050
$Comp
L Switch:SW_MEC_5G SW2
U 1 1 63328D49
P 5500 3350
F 0 "SW2" H 5350 3300 50  0000 C CNN
F 1 "Switch" H 5600 3300 50  0000 C CNN
F 2 "" H 5500 3550 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=488" H 5500 3550 50  0001 C CNN
	1    5500 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 3450 5000 3450
Wire Wire Line
	4900 3400 4900 3450
$Comp
L Device:R_Potentiometer RV1
U 1 1 633274EA
P 4900 3250
F 0 "RV1" H 4830 3296 50  0000 R CNN
F 1 "R_10k" H 4830 3205 50  0000 R CNN
F 2 "" H 4900 3250 50  0001 C CNN
F 3 "~" H 4900 3250 50  0001 C CNN
	1    4900 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3150 5200 3150
$Comp
L Switch:ToggleSwitch SW1
U 1 1 633252C2
P 5350 2950
F 0 "SW1" H 5500 2950 50  0000 C CNN
F 1 "ToggleSwitch" H 5500 2850 50  0000 C CNN
F 2 "" H 5350 2950 50  0001 C CNN
F 3 "" H 5350 2950 50  0001 C CNN
	1    5350 2950
	1    0    0    -1  
$EndComp
$Comp
L power:-BATT #PWR?
U 1 1 63322873
P 5000 3450
F 0 "#PWR?" H 5000 3300 50  0001 C CNN
F 1 "-BATT" H 5015 3623 50  0000 C CNN
F 2 "" H 5000 3450 50  0001 C CNN
F 3 "" H 5000 3450 50  0001 C CNN
	1    5000 3450
	-1   0    0    1   
$EndComp
$Comp
L power:+BATT #PWR?
U 1 1 633221ED
P 5000 3150
F 0 "#PWR?" H 5000 3000 50  0001 C CNN
F 1 "+BATT" H 5015 3323 50  0000 C CNN
F 2 "" H 5000 3150 50  0001 C CNN
F 3 "" H 5000 3150 50  0001 C CNN
	1    5000 3150
	1    0    0    -1  
$EndComp
Connection ~ 5000 3450
Wire Wire Line
	5050 3250 5700 3250
Wire Wire Line
	5000 3450 5700 3450
Wire Wire Line
	5300 3350 5000 3350
Wire Wire Line
	5000 3350 5000 3450
Wire Wire Line
	5500 3150 5700 3150
NoConn ~ 4900 3050
Wire Wire Line
	4900 3100 4900 3050
$Comp
L MCU_Module:Arduino_Mega_(messenger) U1
U 1 1 6331FB6F
P 5900 3000
F 0 "U1" H 6028 2751 50  0000 L CNN
F 1 "Arduino_Mega" H 6028 2660 50  0000 L CNN
F 2 "" H 5900 3000 50  0001 C CNN
F 3 "" H 5900 3000 50  0001 C CNN
	1    5900 3000
	1    0    0    -1  
$EndComp
$EndSCHEMATC
