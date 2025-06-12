##Project Description

Our project, PIC Based Car Battery Voltage Monitoring System, is designed to measure and display the voltage 
level of a car battery in real time. The purpose of the system is to help users monitor their car battery health 
and avoid unexpected failures. The system uses a PIC16F877A microcontroller as the main controller.

The core working principle involves using a voltage divider circuit to scale down the battery voltage to a safe 
level for the microcontroller's ADC (Analog-to-Digital Converter) input. The ADC module of the PIC reads this 
scaled voltage and converts it into a digital value. This value is then displayed on a 16x2 LCD screen for the 
user to see.

When powered on, the system continuously reads the battery voltage, converts it using the ADC, and updates the 
LCD display. This allows the user to see the real-time voltage status of the car battery.

---

****System Workflow****

1. Power-On: When the system is powered on, the microcontroller initializes all the ports
and modules (ADC, Timer, LCD).
2. Input Reading: The system continuously reads the analog voltage from the battery using
the ADC interface.
3. Processing: The digital value is calculated to determine the actual battery voltage.
4. Output: The voltage is displayed on an LCD screen, and if necessary, a buzzer or warning
LED is activated.

---

****Interfaces Used****

GPIO (General Purpose Input/Output):
▪ Used to control the LCD display, LED, and buzzer.
▪ Also used to read digital signals if any button interface is added.
ADC (Analog-to-Digital Converter):
▪ Converts the analog voltage from the battery into a digital value that can be processed by
the microcontroller.

---

**Summary**

This program runs on a PIC16F877A microcontroller and is used to check a car battery’s
voltage and show the result on an LCD screen. It reads the battery voltage using an analog input
(RA0/AN0), calculates the actual voltage, and displays both the voltage and battery condition on
the LCD. The LCD works in 4-bit mode, and the timing is controlled using simple delays (like
waiting 1 second), not timers. The program updates the screen only when the value changes, so it
doesn’t flicker.

---

**Circuit Design & Output**

[Result](result.jpg)