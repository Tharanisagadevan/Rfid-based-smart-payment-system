LCD AND KEYPAD INTERFACING :
#include <pic.h>

// Configuration bits
__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_OFF & CP_OFF & BOREN_ON & LVP_OFF & CPD_OFF & WRT_OFF & DEBUG_OFF);
#define _XTAL_FREQ 20000000  // Crystal frequency (20MHz)

// Define control pins for the LCD
#define rs RD2  // Register select
#define en RD3  // Enable
#define rw GND  // Ground pin

// Define pins for the keypad
#define R1 RB0
#define R2 RB1
#define R3 RB2
#define R4 RB3
#define C1 RB4
#define C2 RB5
#define C3 RB6
#define C4 RB7

// Function prototypes
void lcd_init();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(const unsigned char *s);
void lcd_delay();
void clear_input_str(char *str, unsigned char size);
void int_to_str(unsigned int value, char *str);
unsigned char key();
void keyinit();
void power_by();
void scan_RFID();
void welcome();
void purchase();
void balance();
void total();
void pay_page();
void pin();
void pay_done();
void pay_fail();
void thank_you();

unsigned char keypad[4][4] = {
    {'1', '2', '3', 'H'},
    {'4', '5', '6', 'P'},
    {'7', '8', '9', 'S'},
    {'C', '0', '=', '+'}
};

// Global variables
unsigned char rowloc, colloc;
unsigned int total_amount = 0;

void main() {
    unsigned char b;

    TRISD = 0;     // Set PORTD as output for LCD
    TRISB = 0xF0;  // Set upper nibble as input for keypad
    lcd_init();
    keyinit();
    
    while (1) {
        b = key();
        if (b == 'C') {
            cmd(0x01);  // Clear the display
            continue;    // Skip the rest of the loop to avoid extra delays
        }
        if (b == '1') {
            power_by(); 
        } 
        if (b == '2') {
            scan_RFID();
        }  
        if (b == '3') {
            welcome();
        } 
        if (b == '4') {
            purchase();
        }
        if (b == '5') {
            balance();
        }
        if (b == '6') {
            total();
        }
        if (b == '7') {
            pay_page();
        }
        if (b == '8') {
            pin();
        }
        if (b == '9') {
            pay_done();
        }
        if (b == '0') {
            pay_fail();
        }
        if (b == 'H') {  
            thank_you();
        }
        __delay_ms(100); // Added a delay for button debounce
    }
}

void power_by() {
    cmd(0x01); // Clear display
    cmd(0x81); // Move cursor to the first line
    show("BYTES IN BITS");
    cmd(0xC2);
    show("Smart RFID");
    cmd(0x91);
    show("Payment system");
    cmd(0xD0);
    show("-Fire developers");
}

void scan_RFID() {
    cmd(0x01);
    cmd(0x85);
    show("Scan your");
    cmd(0xc2);
    show("RFID card");
    cmd(0x90);
    show("**************");
    cmd(0xD0);
    show("-------------->");
}

void welcome() {
    cmd(0x01);
    cmd(0x80);
    show("WELCOME");
    cmd(0xC0);
    show("1.Purchase:");
    cmd(0x90);
    show("2.Balance");
    cmd(0xD0);
    show("3.Exit");
    __delay_ms(1000);
}

void purchase() {
    cmd(0x01);
    cmd(0x80);
    show("Enter Amount:");

    unsigned char digits[5] = "    ";  // Buffer for the display
    unsigned char count = 0;
    unsigned char b;

    while (1) {
        b = key();
        if (b >= '0' && b <= '9' && count < 4) {
            digits[count++] = b; // Store the digit
            dat(b); // Display the digit
        } else if (b == '+' || b == '-') {
            if (count > 0) {
                // Convert entered digits to an integer
                unsigned int entered_value = 0;
                for (unsigned char i = 0; i < count; i++) {
                    entered_value = entered_value * 10 + (digits[i] - '0');
                }

                if (b == '+') {
                    total_amount += entered_value;
                } else if (b == '-') {
                    total_amount -= entered_value;
                }
            }
            count = 0; // Reset count after operation
            clear_input_str(digits, sizeof(digits)); // Clear digits after operation
        } else if (b == '=') {
            // Display total amount
            cmd(0x01); // Clear display
            cmd(0x80);
            show("Total Amount:");
            cmd(0xC0);
            char total_str[6]; // Enough space for total and null terminator
            int_to_str(total_amount, total_str);
            show(total_str); // Display total amount
            lcd_delay(); // Short delay
            cmd(0x80); // Move cursor back to the top
            __delay_ms(1000);
            break; // Exit the purchase mode
        }
    }
}

void balance() {
    cmd(0x01);
    cmd(0x80);
    show("User balance:");
    // Here you can display the actual balance if you have that variable
    __delay_ms(1000);
}

void total() {
    cmd(0x01);
    cmd(0x80);
    show("Total amount:");
    char total_str[6];
    int_to_str(total_amount, total_str);
    cmd(0xC0);
    show(total_str);
    __delay_ms(1000);
}

void pay_page() {
    cmd(0x01);
    cmd(0x80);
    show("Payment page");
    cmd(0xc0);
    show("tharani");
    __delay_ms(1000);
}

void pin() { 
    cmd(0x01);
    cmd(0x80);
    show("PIN:");
    cmd(0xc0);
    show("1234");
    __delay_ms(1000);
}

void pay_done() { 
    cmd(0x01);
    cmd(0x80);
    show("Payment Done");
    __delay_ms(1000);
}

void pay_fail() {
    cmd(0x01);
    cmd(0x80);
    show("Payment Fail");
    __delay_ms(1000);
}

void thank_you() {
    cmd(0x01);
    cmd(0x80);
    show("Thank You");
    cmd(0xc0);
    show("Come Again");
    __delay_ms(1000);
}

void lcd_init() {
    cmd(0x02);  // Return home
    cmd(0x28);  // 4-bit mode, 2 lines, 5x7 dots
    cmd(0x0E);  // Display on, cursor blinking
    cmd(0x06);  // Increment cursor
    cmd(0x80);  // Move cursor to the first line
    lcd_delay(); // Added delay after initialization
}

void cmd(unsigned char a) {
    rs = 0; 
    PORTD &= 0x0F;
    PORTD |= (a & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
    PORTD &= 0x0F;
    PORTD |= (a << 4 & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
}

void dat(unsigned char b) {
    rs = 1; 
    PORTD &= 0x0F;
    PORTD |= (b & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
    PORTD &= 0x0F;
    PORTD |= (b << 4 & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
}

void show(const unsigned char *s) {
    while (*s) {
        dat(*s++);
    }
}

void lcd_delay() {
    unsigned int lcd_delay;
    for (lcd_delay = 0; lcd_delay <= 1000; lcd_delay++);
}

void clear_input_str(char *str, unsigned char size) {
    for (unsigned cn.har i = 0; i < size; i++) {
        str[i] = ' ';  // Fill with spaces
    }
    str[size] = '\0';  // Null-terminate the string
}

void int_to_str(unsigned int value, char *str) {
    unsigned char i = 0;
    if (value == 0) {
        str[i++] = '0';
    } else {
        while (value > 0) {
            str[i++] = (value % 10) + '0';  // Get the last digit
            value /= 10;                     // Remove the last digit
        }
    }
    str[i] = '\0';  // Null-terminate the string

    // Reverse the string
    for (unsigned char j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

void keyinit() {
    OPTION_REG |= 0x7F;  // Enable pull-ups
}

unsigned char key() {
    PORTB = 0x00;  // Clear PORTB
    while (C1 && C2 && C3 && C4);  // Wait for key press
    while (!C1 || !C2 || !C3 || !C4) {
        R1 = 0; R2 = R3 = R4 = 1;
        if (!C1 || !C2 || !C3 || !C4) {
            rowloc = 0;
            break;
        }
        R2 = 0; R1 = 1;
        if (!C1 || !C2 || !C3 || !C4) {
            rowloc = 1;
            break;
        }
        R3 = 0; R2 = 1;
        if (!C1 || !C2 || !C3 || !C4) {
            rowloc = 2;
            break;
        }
        R4 = 0; R3 = 1;
        if (!C1 || !C2 || !C3 || !C4) {
            rowloc = 3;
            break;
        }
    }
    
    if (C1 == 0 && C2 != 0 && C3 != 0 && C4 != 0)
        colloc = 0;
    else if (C1 != 0 && C2 == 0 && C3 != 0 && C4 != 0)
        colloc = 1;
    else if (C1 != 0 && C2 != 0 && C3 == 0 && C4 != 0)
        colloc = 2;
    else if (C1 != 0 && C2 != 0 && C3 != 0 && C4 == 0)
        colloc = 3;
    
    while (C1 == 0 || C2 == 0 || C3 == 0 || C4 == 0);
    return (keypad[rowloc][colloc]);
}

