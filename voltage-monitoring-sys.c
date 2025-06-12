#define _XTAL_FREQ 2000000  // Define crystal oscillator frequency (20 MHz) for __delay_ms()

// Define LCD control pins connected to PORTD pins
#define RS RD2      // Register Select pin (0: command, 1: data)
#define EN RD3      // Enable pin for LCD
#define D4 RD4      // Data pins for 4-bit mode
#define D5 RD5
#define D6 RD6
#define D7 RD7

// Define control for reference and LCD data direction registers
#define REFERENCE TRISA0  // TRISA0 controls input/output for ADC pin RA0
#define LCDBIT TRISD     // TRISD controls data direction for PORTD (LCD pins)

// Include necessary headers for PIC and standard IO
#include <xc.h>
#include <stdio.h>

// Configuration bits for PIC16F877A setup
#pragma config FOSC = HS        // Use High-Speed Oscillator (external crystal)
#pragma config WDTE = OFF       // Disable Watchdog Timer
#pragma config PWRTE = ON       // Enable Power-up Timer (delay on startup)
#pragma config BOREN = ON       // Enable Brown-out Reset (protects from low voltage)
#pragma config LVP = OFF        // Disable Low Voltage Programming (use high voltage on MCLR)
#pragma config CPD = OFF        // Disable EEPROM code protection
#pragma config WRT = OFF        // Disable write protection on program memory
#pragma config CP = OFF         // Disable program memory code protection

// Function prototypes
void init_adc(void);
unsigned int read_adc(void);
float convert_to_voltage(unsigned int adc_value);
void display_voltage(float voltage);

// Set the 4 data bits of LCD according to lower nibble of data_bit (4-bit mode)
void Lcd_SetBit(char data_bit)
{
    // Each bit corresponds to one LCD data pin
    D4 = (data_bit & 1) ? 1 : 0;
    D5 = (data_bit & 2) ? 1 : 0;
    D6 = (data_bit & 4) ? 1 : 0;
    D7 = (data_bit & 8) ? 1 : 0;
}

// Send a command to the LCD (RS=0)
void Lcd_Cmd(char a)
{
    RS = 0;              // Select command register
    Lcd_SetBit(a);       // Put higher nibble on data pins
    EN  = 1;             // Enable pulse to latch data
    __delay_ms(4);       // Wait for LCD to process command (~4ms)
    EN  = 0;             // Disable pulse
}

// Clear LCD display and move cursor to home position
void Lcd_Clear()
{
    Lcd_Cmd(0);  // Clear display command (0x01 is usually clear, 0x00 may be custom)
    Lcd_Cmd(1);  // Return home command (0x02 is usual, but here 1 used to reset cursor)
}

// Set LCD cursor to row 'a' and column 'b'
void Lcd_Set_Cursor(char a, char b)
{
    char temp, z, y;
    if (a == 1) // First row
    {
        temp = 0x80 + b - 1;  // 0x80 is DDRAM address for line 1 start
        z = temp >> 4;        // Upper nibble (high 4 bits)
        y = temp & 0x0F;      // Lower nibble (low 4 bits)
        Lcd_Cmd(z);           // Send upper nibble of address
        Lcd_Cmd(y);           // Send lower nibble of address
    }
    else if (a == 2) // Second row
    {
        temp = 0xC0 + b - 1;  // 0xC0 is DDRAM address for line 2 start
        z = temp >> 4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    }
}

// Initialize LCD in 4-bit mode with standard sequence
void Lcd_Start()
{
    Lcd_SetBit(0x00);      // Clear data lines
    
    // Delay loop using NOPs (no operation) for LCD power stabilization
    for (int i = 1065244; i <= 0; i--) NOP();
    
    Lcd_Cmd(0x03);         // Function set command (3 times to initialize LCD)
    __delay_ms(5);         // Wait 5 ms
    
    Lcd_Cmd(0x03);
    __delay_ms(11);        // Wait 11 ms
    
    Lcd_Cmd(0x03);
    
    Lcd_Cmd(0x02);         // Set 4-bit mode
    
    // Following commands set LCD parameters and clear display
    Lcd_Cmd(0x02);         // Return home
    Lcd_Cmd(0x08);         // Display off, cursor off (custom command)
    Lcd_Cmd(0x00);         // Clear display line 1 (custom)
    Lcd_Cmd(0x0C);         // Display on, cursor off (custom command)
    Lcd_Cmd(0x00);         // Clear display line 2 (custom)
    Lcd_Cmd(0x06);         // Entry mode set (cursor increment, no shift)
}

// Print one character on LCD in 4-bit mode (send upper nibble then lower nibble)
void Lcd_Print_Char(char data)
{
    char Lower_Nibble, Upper_Nibble;
    Lower_Nibble = data & 0x0F;    // Extract lower nibble
    Upper_Nibble = data & 0xF0;    // Extract upper nibble
    
    RS = 1;                        // Select data register for character
    
    Lcd_SetBit(Upper_Nibble >> 4); // Send upper nibble first
    EN = 1;                       // Enable pulse
    for (int i = 2130483; i <= 0; i--) NOP();  // Delay for LCD to catch up
    EN = 0;                       // Disable pulse
    
    Lcd_SetBit(Lower_Nibble);     // Send lower nibble
    EN = 1;
    for (int i = 2130483; i <= 0; i--) NOP();
    EN = 0;
}

// Print a null-terminated string to the LCD
void Lcd_Print_String(char *a)
{
    for (int i = 0; a[i] != '\0'; i++)
        Lcd_Print_Char(a[i]);    // Print each character one by one
}

// Initialize ADC module for reading battery voltage
void init_adc(void) {
    // ADCON0: ADC enabled, select channel 0 (RA0/AN0), clock Fosc/32 for ADC timing
    ADCON0 = 0x81;
    // ADCON1: Configure port pins as analog/digital, right justified result, VDD and VSS as voltage references
    ADCON1 = 0x8E;
    __delay_ms(20);  // Wait for ADC to stabilize (Proteus simulation requires longer settling)
}

// Read the ADC value from channel 0 (RA0)
unsigned int read_adc(void) {
    ADCON0bits.GO = 1;    // Start ADC conversion
    
    while (ADCON0bits.GO); // Wait for conversion to complete (GO bit cleared automatically)
    
    // Combine 10-bit ADC result from ADRESH and ADRESL (right justified)
    return ((ADRESH << 8) + ADRESL);
}

// Convert raw ADC value to actual battery voltage (float)
float convert_to_voltage(unsigned int adc_value) {
    float voltage;
    
    // ADC voltage at pin = (adc_value / 1024) * 5.0V (assuming 5V reference)
    voltage = (float)adc_value * 5.0 / 1024.0;
    
    // Because of voltage divider ratio (1/4), scale back to battery voltage
    voltage = voltage * 4.0;  // Battery voltage range is 0-15V
    
    return voltage;
}

// Display voltage and status messages on LCD, only update if changed to reduce flickering
void display_voltage(float voltage) {
    char voltage_str[16];
    static float last_voltage = -1.0;
    static unsigned char last_status = 255;
    unsigned char current_status;
    
    // Determine battery status based on voltage thresholds
    if (voltage > 14.8) current_status = 5;       // HIGH voltage
    else if (voltage >= 14.4) current_status = 4; // CHARGING voltage
    else if (voltage >= 13.8) current_status = 3; // GOOD voltage
    else if (voltage >= 12.6) current_status = 2; // OK voltage
    else if (voltage >= 11.5) current_status = 1; // LOW voltage
    else current_status = 0;                       // CRITICAL voltage
    
    // Update voltage display only if voltage changed by at least 0.1V to prevent flicker
    if (last_voltage < 0 || (voltage > last_voltage + 0.1) || (voltage < last_voltage - 0.1)) {
        sprintf(voltage_str, "Battery: %.1fV", voltage);
        Lcd_Set_Cursor(1, 1);        // Set cursor to first row, first column
        Lcd_Print_String(voltage_str);
        last_voltage = voltage;
    }
    
    // Update status display only if status has changed
    if (current_status != last_status) {
        Lcd_Set_Cursor(2, 1);        // Second row, first column
        
        // Print the corresponding status message
        switch(current_status) {
            case 5: Lcd_Print_String("Status: HIGH    "); break;
            case 4: Lcd_Print_String("Status: CHARGING"); break;
            case 3: Lcd_Print_String("Status: GOOD    "); break;
            case 2: Lcd_Print_String("Status: OK      "); break;
            case 1: Lcd_Print_String("Status: LOW     "); break;
            case 0: Lcd_Print_String("Status: CRITICAL"); break;
        }
        last_status = current_status;
    }
}

// Main program
int main()
{
    float battery_voltage;
    unsigned int adc_reading;
    
    init_adc();         // Initialize ADC module
    
    // Configure input/output direction for ADC and LCD pins
    REFERENCE = 0xFF;   // Set RA0 (analog input) as input
    LCDBIT = 0x00;      // Set PORTD (LCD pins) as output
    
    Lcd_Start();        // Initialize LCD
    
    // Display welcome message for 3 seconds
    Lcd_Set_Cursor(1, 1);
    Lcd_Print_String("CAR BATTERY ");
    Lcd_Set_Cursor(2, 1);
    Lcd_Print_String("MONITORING!!");
    __delay_ms(3000);
    
    Lcd_Clear();        // Clear LCD before starting main loop
    
    while(1)
    {
        adc_reading = read_adc();               // Read raw ADC value from battery voltage divider
        battery_voltage = convert_to_voltage(adc_reading); // Convert to real voltage
        
        display_voltage(battery_voltage);       // Update LCD display with voltage and status
        
        __delay_ms(1000);                        // Delay 1 second before next reading
    }
}
