Fetch the Name, Amount and Pin Display in LCD
Code:
Pic:

#include <pic.h>
#include <stdio.h>

// Configuration bits
__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_OFF & CP_OFF & BOREN_ON & LVP_OFF & CPD_OFF & WRT_OFF & DEBUG_OFF);
#define _XTAL_FREQ 20000000  // Crystal frequency (20MHz)

// Define control pins for the LCD
#define rs RD2  // Register select
#define en RD3  // Enable
#define rw GND  // Ground pin

// Function prototypes
void lcd_init();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(const unsigned char *s);
void uart_init(void);
char uart_receive(void);
void uart_receive_string(char *buffer, unsigned char size);
void lcd_delay();

// Global variable
char received_string[64]; // Buffer to store the received string (increased size)

void main() {
    // Port initialization
    TRISD = 0;     // Set PORTD as output for LCD
    uart_init();
    lcd_init();

    while (1) {
        // Receive a string from the UART
        uart_receive_string(received_string, sizeof(received_string));
        
        cmd(0x01); // Clear display

        // Custom parsing of the received string
        char name[16], amount[16], pin[16];
        int i = 0, j = 0;

        // Extract name
        while (received_string[i] != ',' && received_string[i] != '\0') {
            name[j++] = received_string[i++];
        }
        name[j] = '\0'; // Null-terminate the name
        i++; // Move past the comma

        // Extract amount
        j = 0;
        while (received_string[i] != ',' && received_string[i] != '\0') {
            amount[j++] = received_string[i++];
        }
        amount[j] = '\0'; // Null-terminate the amount
        i++; // Move past the comma

        // Extract pin
        j = 0;
        while (received_string[i] != '\0' && received_string[i] != '\n' && received_string[i] != '\r') {
            pin[j++] = received_string[i++];
        }
        pin[j] = '\0'; // Null-terminate the pin

        // Display name on the first line
        cmd(0x80); // Move cursor to the first line
        show(name); // Display name

        // Display amount on the second line
        cmd(0xC0); // Move cursor to the second line
        show(amount); // Display amount

        // Display pin on the third line
        cmd(0x94); // Move cursor to the third line
        show(pin); // Display pin

        __delay_ms(2000); // Delay to see the messages
    }
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
    rs = 0; // Command mode
    PORTD &= 0x0F; // Clear upper nibble
    PORTD |= (a & 0xF0); // Send high nibble
    en = 1; // Enable pulse
    lcd_delay();
    en = 0;

    PORTD &= 0x0F; // Clear upper nibble
    PORTD |= (a << 4); // Send low nibble
    en = 1; // Enable pulse
    lcd_delay();
    en = 0;
}

void dat(unsigned char b) {
    rs = 1; // Data mode
    PORTD &= 0x0F; // Clear upper nibble
    PORTD |= (b & 0xF0); // Send high nibble
    en = 1; // Enable pulse
    lcd_delay();
    en = 0;

    PORTD &= 0x0F; // Clear upper nibble
    PORTD |= (b << 4); // Send low nibble
    en = 1; // Enable pulse
    lcd_delay();
    en = 0;
}

void show(const unsigned char *s) {
    unsigned char line = 0; // Track the current line
    unsigned char count = 0; // Track character count on the current line

    while (*s) {
        // If we reach the end of a line (16 characters), move to the next line
        if (count >= 16) {
            count = 0;
            line++;
            // Move to the start of the next line, if within bounds
            if (line < 4) {
                cmd(0xC0 + (line * 0x40)); // Move to line 2, 3, or 4
            } else {
                // If we exceed 4 lines, stop processing
                break;
            }
        }

        // Send the character to the LCD
        dat(*s++);
        count++; // Increment the character count
    }
}

void lcd_delay() {
    unsigned int lcd_delay;
    for (lcd_delay = 0; lcd_delay < 1000; lcd_delay++);
}

void uart_init(void) {
    TRISC6 = 0;  // TX pin set as output
    TRISC7 = 1;  // RX pin set as input
    SPBRG = 129;  // Baud rate 9600 for 20MHz
    BRGH = 1;     // High baud rate
    SYNC = 0;     // Asynchronous mode
    SPEN = 1;     // Enable serial port
    TXEN = 1;     // Enable transmission
    CREN = 1;     // Enable continuous reception
    TX9 = 0;      // 8-bit transmission
    RX9 = 0;      // 8-bit reception
}

char uart_receive(void) {
    while (!RCIF); // Wait for reception to complete
    return RCREG;  // Return received data
}

void uart_receive_string(char *buffer, unsigned char size) {
    unsigned char i = 0;
    char ch;

    while (i < size - 1) { // Leave space for null terminator
        ch = uart_receive(); // Receive one character
        if (ch == '\n' || ch == '\r') { // Check for newline or carriage return
            buffer[i] = '\0'; // Null-terminate the string
            break; // Exit loop if end of string
        }
        buffer[i++] = ch; // Store character in buffer
    }
    buffer[i] = '\0'; // Null-terminate the string
}
 
Ardiuno:

#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <SoftwareSerial.h>

// RFID Setup
#define SS_PIN 2 // SDA
#define RST_PIN 0 // RST
MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi and Firebase Setup
#define WIFI_SS "Galaxy A10s5584"
#define WIFI_PASSWORD "isaitharani"
#define API_KEY "AIzaSyCi-2z5tjB8LLBbMCZeuewGi6tSQtF9iPg"
#define DATABASE_URL "https://smart-way-payment-default-rtdb.firebaseio.com"
#define USER_EMAIL "thirisha1227@gmail.com"
#define USER_PASSWORD "Thirisha@2711"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Updated UIDs for lookup
const char* uids[] = {"af14e91c", "3351e534", "8f13ea1f"}; // Add your UIDs for lookup

// Define RX and TX pins for communication with PIC
#define RX_PIN 3
#define TX_PIN 1
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600); // Communication with PIC

    // Initialize RFID
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("Scan an RFID tag...");

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SS, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Set up Firebase configuration
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    // Check for RFID card presence
    if (!rfid.PICC_IsNewCardPresent()) {
        return;
    }
    // Read RFID card
    if (!rfid.PICC_ReadCardSerial()) {
        return;
    }

    // Get UID as a string
    String scannedUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        scannedUID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
    }
    Serial.println("Scanned UID: " + scannedUID);

    // Match the scanned UID with predefined UIDs
    for (int i = 0; i < sizeof(uids) / sizeof(uids[0]); i++) {
        if (scannedUID.equalsIgnoreCase(uids[i])) {
            Serial.println("User Found: " + String(uids[i]));
            fetchData(uids[i]); // Fetch data for the matched UID
            break;
        }
    }

    // Halt PICC
    rfid.PICC_HaltA();
}
void fetchData(const char* uid) {
    // Set the paths for amount, name, and pin
    String pathAmount = "UID/" + String(uid) + "/amount";
    String pathName = "UID/" + String(uid) + "/name";
    String pathPin = "UID/" + String(uid) + "/pin";

    // Fetch and print the amount, name, and pin
    String name = fetchName(pathName.c_str(), uid);
    int amount = fetchAmount(pathAmount.c_str(), uid);
    int pin = fetchPin(pathPin.c_str(), uid);
    // Send formatted data to PIC
    mySerial.print(String(name) + "," + String(amount) + "," + String(pin) + "\n");
}

String fetchName(const char* path, const char* uid) {
    // Retrieve and return the name associated with the UID
    if (Firebase.RTDB.get(&fbdo, path)) {
        if (fbdo.dataType() == "string") {
            return fbdo.stringData().c_str(); // Use stringData() for string retrieval
        }
    } else {
        Serial.println("Error getting name for " + String(uid) + ": " + fbdo.errorReason());
    }
    return "Unknown"; // Default if not found
}
int fetchAmount(const char* path, const char* uid) {
    // Retrieve and return the amount associated with the UID
    if (Firebase.RTDB.getInt(&fbdo, path)) {
        return fbdo.intData(); // Return amount from Firebase
    } else {
        Serial.println("Error getting amount for " + String(uid) + ": " + fbdo.errorReason());
    }
    return 0; // Default if not found
}
int fetchPin(const char* path, const char* uid) {
    // Retrieve and return the pin associated with the UID
    if (Firebase.RTDB.getInt(&fbdo, path)) {
        return fbdo.intData(); // Return pin from Firebase
    } else {
        Serial.println("Error getting pin for " + String(uid) + ": " + fbdo.errorReason());
    }
    return 0; // Default if not found
}`

