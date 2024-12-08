CLOUD DATA FETCHING TAPED RFID PIN:

#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h> // For parsing JSON data
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
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
void setup()
{
    Serial.begin(115200);
    // Initialize RFID
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("Scan an RFID tag...");
    // Connect to Wi-Fi
    WiFi.begin(WIFI_SS, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
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
    config.token_status_callback = tokenStatusCallback;  // For debugging purposes
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}
void loop()
{
    // Check for RFID card presence
    if (!rfid.PICC_IsNewCardPresent())
    {
        return;
    }
    // Read RFID card
    if (!rfid.PICC_ReadCardSerial())
    {
        return;
    }
    // Get UID as a string
    String scannedUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        scannedUID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
    }
    Serial.println("Scanned UID: " + scannedUID);
    // Match the scanned UID with predefined UIDs
    for (int i = 0; i < sizeof(uids) / sizeof(uids[0]); i++)
    {
        if (scannedUID.equalsIgnoreCase(uids[i]))
        {
            Serial.println("User Found: " + String(uids[i]));
            fetchData(uids[i]); // Fetch data for the matched UID
            break;
        }
    }
    // Halt PICC
    rfid.PICC_HaltA();
}
void fetchData(const char* uid) {
    // Set the paths for amount and name
    String pathAmount = "UID/" + String(uid) + "/amount";
    String pathName = "UID/" + String(uid) + "/name";
    String pathpin = "UID/" + String(uid) + "/pin";
   


    // Fetch and print the amount
    printAmount(pathAmount.c_str(), uid);
    // Fetch and print the name
    printName(pathName.c_str(), uid);
    printpin(pathpin.c_str(), uid);
}
void printAmount(const char* path, const char* uid)
{
    // Retrieve and print the amount associated with the UID
    if (Firebase.RTDB.getInt(&fbdo, path))
    {
        int amount = fbdo.intData(); // Assign amount from Firebase
        Serial.println("Amount for " + String(uid) + ": " + String(amount)); // Print amount on a separate line
    }
    else
    {
        Serial.println("Error getting amount for " + String(uid) + ": " + fbdo.errorReason());
    }
}
void printName(const char* path, const char* uid)
{
    // Retrieve and print the name associated with the UID
    if (Firebase.RTDB.get(&fbdo, path))
    {
        Serial.print("\n NAME: " + String(fbdo.stringData())); // Debug output
        if (fbdo.dataType() == "string")
        {
            const char* name = fbdo.stringData().c_str(); // Use stringData() for string retrieval
            Serial.println("\n Name for " + String(uid) + ": " + String(name)); // Print name on a separate line
        }
        else
        {
            Serial.println("\n Name data is not a string for " + String(uid));
        }
    }
    else
    {
        Serial.println("Error getting name for " + String(uid) + ": " + fbdo.errorReason());
    }
}
void printpin(const char* path, const char* uid)
{
    // Retrieve and print the amount associated with the UID
    if (Firebase.RTDB.getInt(&fbdo, path))
    {
        int pin = fbdo.intData(); // Assign amount from Firebase
        Serial.println("\n pin  for " + String(uid) + ": " + String(pin)); // Print amount on a separate line
    }
    else
    {
        Serial.println("Error getting pin for " + String(uid) + ": " + fbdo.errorReason());
    }
}
OUTPUT:
20:56:06.196 -> Scanned UID: 3351e534


20:56:06.196 -> User Found: 3351e534


20:56:09.098 -> Amount for 3351e534: 5000


20:56:09.702 -> 


20:56:09.702 ->  NAME: Tharani


20:56:09.702 ->  Name for 3351e534: 


20:56:10.086 -> 


20:56:10.086 ->  pin  for 3351e534: 1234
