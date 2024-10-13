#include <pic.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000 // Define your oscillator frequency

// Configuration bits
__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_OFF & CP_OFF & BOREN_ON & LVP_OFF & CPD_OFF & WRT_OFF & DEBUG_OFF);

// LCD control pin definitions
#define rs RC0   // RS pin
#define rw RC1   // RW pin
#define en RC2   // EN pin

// Data pins for LCD connected to PORTD
#define DATA_PORT PORTD
#define DATA_TRIS TRISD

// Keypad connections
#define KEYPAD_PORT PORTB
#define KEYPAD_TRIS TRISB

// Function prototypes
void lcd_init();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(const unsigned char *s);
void lcd_delay();
void clear_display();
void home_menu();
void enter_amount();
void calculate_total(unsigned char amount);
char keypad_scan();
void switch_case_menu(char key);
void display_total(unsigned char total);

// Global variable for total sales
unsigned char total_sales = 0;

void main() {
    // Set data direction
    DATA_TRIS = 0x00;  // PORTD as output for LCD data
    TRISC = 0x00;      // PORTC as output for control signals
    KEYPAD_TRIS = 0xF0; // Lower nibble for keypad output, upper nibble for input

    lcd_init();
    clear_display();

    while (1) {
        home_menu();
        __delay_ms(2000);
        clear_display();

        char key;
        while ((key = keypad_scan()) == '\0'); // Wait for key press
        switch_case_menu(key);
    }
}

void home_menu() {
    cmd(0x80); // Move cursor to the first line
    show("Home Menu"); // Display header
    cmd(0xC0); // Move cursor to the second line
    show("Press key:"); // Prompt for key press
}

void enter_amount() {
    cmd(0x80); // Move cursor to the first line
    show("Enter Amount:"); // Prompt user to enter amount

    unsigned char amount = 0;
    char key;
    int i = 0;

    // Collect user input
    while (i < 3) { // Limit input to 3 digits
        while ((key = keypad_scan()) == '\0'); // Wait for key press
        if (key >= '0' && key <= '9') {
            amount = amount * 10 + (key - '0'); // Convert char to int
            cmd(0xC0 + i); // Move cursor to second line
            dat(key); // Display entered digit
            i++;
        } else if (key == 'C') {
            amount = 0; // Clear amount
            clear_display();
            return; // Exit amount entry
        } else if (key == 'X') {
            // Remove last digit
            if (i > 0) {
                amount /= 10; // Divide by 10 to remove last digit
                cmd(0xC0 + i - 1); // Move cursor to last entered digit position
                dat(' '); // Clear last digit
                i--;
            }
        }
    }
    total_sales += amount; // Add to total sales
    calculate_total(amount);
}

void calculate_total(unsigned char amount) {
    cmd(0x80); // Move cursor to the first line
    show("Total: ");
    display_total(total_sales);
    __delay_ms(2000);
    clear_display();
}

void display_total(unsigned char total) {
    char buffer[16];
    sprintf(buffer, "Total: %u", total);
    show(buffer); // Display total amount
}

char keypad_scan() {
    const char keys[4][4] = {
        {'1', '2', '3', '+'},
        {'4', '5', '6', 'C'},
        {'7', '8', '9', '0'},
        {'H', 'S', '=', 'X'}
    };

    // Scan the keypad
    for (int row = 0; row < 4; row++) {
        KEYPAD_PORT = ~(1 << row); // Activate one row at a time
        for (int col = 0; col < 4; col++) {
            if (!(KEYPAD_PORT & (1 << (col + 4)))) { // Check if button is pressed
                while (!(KEYPAD_PORT & (1 << (col + 4)))); // Wait for release
                return keys[row][col]; // Return the key pressed
            }
        }
    }
    return '\0'; // No key pressed
}

void switch_case_menu(char key) {
    switch (key) {
        case 'H':
            clear_display(); // Logic for previous page (not implemented)
            break;
        case 'S':
            enter_amount(); // Enter amount for calculation
            break;
        case 'X':
            // Logic for reducing a digit (handled in enter_amount)
            break;
        case 'C':
            clear_display(); // Clear display
            break;
        default:
            break;
    }
}

void lcd_init() {
    cmd(0x02); // Initialize LCD in 4-bit mode
    cmd(0x28); // 4-bit mode, 2-line display
    cmd(0x0C); // Display on, cursor off
    cmd(0x06); // Increment cursor
    cmd(0x80); // Set cursor to first line, first position
}

void cmd(unsigned char a) {
    rs = 0; // Command mode
    rw = 0; // Write mode
    DATA_PORT &= 0x0F; // Clear higher nibble
    DATA_PORT |= (a & 0xF0); // Send higher nibble
    en = 1; // Enable
    lcd_delay();
    en = 0; // Disable
    lcd_delay();
    
    DATA_PORT &= 0x0F; // Clear higher nibble
    DATA_PORT |= (a << 4 & 0xF0); // Send lower nibble
    en = 1; // Enable
    lcd_delay();
    en = 0; // Disable
    lcd_delay();
}

void dat(unsigned char b) {
    rs = 1; // Data mode
    rw = 0; // Write mode
    DATA_PORT &= 0x0F; // Clear higher nibble
    DATA_PORT |= (b & 0xF0); // Send higher nibble
    en = 1; // Enable
    lcd_delay();
    en = 0; // Disable
    lcd_delay();
    
    DATA_PORT &= 0x0F; // Clear higher nibble
    DATA_PORT |= (b << 4 & 0xF0); // Send lower nibble
    en = 1; // Enable
    lcd_delay();
    en = 0; // Disable
    lcd_delay();
}

void show(const unsigned char *s) {
    while (*s) {
        dat(*s++); // Send characters to LCD
    }
}

void lcd_delay() {
    for (unsigned int i = 0; i < 1000; i++); // Simple delay loop
}

void clear_display() {
    cmd(0x01); // Clear display
    lcd_delay();
}
