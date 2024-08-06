#include <HardwareBLESerial.h>
#include <SPI.h>

HardwareBLESerial &bleSerial = HardwareBLESerial::getInstance();

void setup() {
  Serial.begin(57600);
  Serial1.begin(57600);
  while (!Serial);
  while (!Serial1);

  // Start the BLE so we can get the mac
  if (!BLE.begin()) {
    Serial.println("[DEBUG] Starting BLE failed!");
    while (1);
  }

  // Create the full BLE name by appending the last three octets to "DMRCHAT-"
  String localAddress = BLE.address();
  String bleName = "DMRCHAT-";
  bleName += localAddress;

  Serial.println("[DEBUG] Initializing BLE.");
  if (!bleSerial.beginAndSetupBLE(bleName.c_str())) {
    while (true) {
      Serial.println("[DEBUG] Failed to initialize.");
      delay(1000);
    }
    Serial.println("[DEBUG] Initialized BLE.");
  }

  // wait for a central device to connect
  Serial.println("[DEBUG] Waiting for device to connect ...");
  while (!bleSerial);
  Serial.println("[DEBUG] Device connected!");
}

void loop() {
  static String bleBuffer = "";
  static String serialBuffer = "";

  if (!bleSerial) {
    Serial.println("[DEBUG] Device disconnected!");
    Serial.println("[DEBUG] Waiting for another connection ...");
    while (!bleSerial); // wait for central device to connect again
    Serial.println("[DEBUG] Device connected!");
  } else {
    // this must be called regularly to perform BLE updates
    bleSerial.poll();

    // whatever is written to BLE UART appears in the Serial Monitor
    while (bleSerial.available() > 0) {
      char c = bleSerial.read();
      bleBuffer.concat(c);
      if (c == 0x10) {
        Serial1.write(bleBuffer.c_str());
        bleBuffer = "";
      }
    }

    // whatever is written in Serial Monitor appears in BLE UART
    while (Serial1.available() > 0) {
      char c = Serial1.read();
      serialBuffer.concat(c);
      if (c == 0x10) {  // ensure we have a full packet
        // ble requires us to send 1 char at a time
        for (size_t i = 0; i < serialBuffer.length(); i++) {
          bleSerial.write((uint8_t)serialBuffer[i]);
        }
        serialBuffer = "";
      }
    }
  }
}
