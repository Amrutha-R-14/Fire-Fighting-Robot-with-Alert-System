#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "wifi id ";
const char* password = "wifi password";

// Twilio account details
const char* account_sid = "your accound sid";
const char* auth_token = "";
const char* twilio_number = "your twilio number";
const char* to_number = "your phone number to which sms is to be sent";

// Define Serial2 TX and RX pins
#define RXD2 16  // RX pin for ESP32
#define TXD2 17  // TX pin for ESP32

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Initialize Serial2 with defined pins

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void loop() {
  if (Serial2.available()) {
    
    char receivedChar = Serial2.read();
    String alertMessage = String(receivedChar); // Read a single character
    // Convert to String
     // Remove any unwanted whitespace
    

    if (alertMessage == "F") {
      Serial.println("🔥 Fire Alert Received!");
      sendSMS("🔥 Fire Detected! Stay Safe!");
      delay(1000);
    } 
    
  }
}


void sendSMS(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://api.twilio.com/2010-04-01/Accounts/";
    url += account_sid;
    url += "/Messages.json";

    String data = "To=" + String(to_number) + "&From=" + String(twilio_number) + "&Body=" + message;
    String auth = String(account_sid) + ":" + String(auth_token);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.setAuthorization(account_sid, auth_token);

    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("📩 Message Sent!");
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("❌ Error sending message: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("⚠ Wi-Fi not connected!");
  }
}
