// Simple RFM69HCW Test Sketch for Teensy 4.1 using the RadioHead Library
//
// This code is designed to be run on two identical setups.
// It sends a packet to a destination node every second and listens for incoming packets.
// When a packet is received, it's printed to the Serial monitor along with the RSSI.
//
// WIRING:
// Teensy 4.1      -> RFM69HCW Module
// ------------------------------------
// GND             -> GND
// 3.3V            -> 3V
// Pin 13 (SCK)    -> SCK
// Pin 12 (MISO)   -> MISO
// Pin 11 (MOSI)   -> MOSI
// Pin 10 (CS)     -> CS  (Configurable below)
// Pin 2  (IRQ)    -> G0  (Configurable below)
// Pin 3  (RST)    -> RST (Configurable below)

#include <Arduino.h>
#include <RH_RF69.h>
#include <SPI.h>

// #################### CONFIGURATION ####################
//
// STEP 1:
// --------
// UNCOMMENT the line below for the FIRST board you program.
// COMMENT OUT the line below for the SECOND board you program.
//
#define IS_NODE_1

// STEP 2:
// --------
// Set the radio frequency.
// MAKE SURE BOTH NODES ARE ON THE SAME FREQUENCY.
#define RF69_FREQ 433.0

// STEP 3:
// --------
// Define the pins used for CS, IRQ, and RST.
#define RFM69_CS 10
#define RFM69_IRQ digitalPinToInterrupt(2)
#define RFM69_RST 3
// #######################################################

// Define Node Addresses
#ifdef IS_NODE_1
#define MY_ADDRESS 1 // This node's address
#else
#define MY_ADDRESS 2 // This node's address
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_IRQ);

// Variable to track the last send time
long lastSendTime = 0;

void setup() {
  Serial.begin(9600);

  // For Teensy 3.x and T4.x the following format is required to operate correctly
  // pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Teensy RFM69 Client!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  //----- END TEENSY CONFIG

  if (!rf69.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  // No encryption
  if (!rf69.setFrequency(433.0))
    Serial.println("setFrequency failed");

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  rf69.setTxPower(14);

  // The encryption key has to be the same as the one in the second board
  uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
}

void loop() {
  // --- RECEIVE LOGIC ---
  if (rf69.available()) {
    // A message has been received, let's process it
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf69.recv(buf, &len)) {
      // Null-terminate the received buffer to safely print it as a string
      buf[len] = '\0';

      Serial.print("Received [");
      Serial.print(len);
      Serial.print(" bytes]: ");
      Serial.print((char *)buf);
      Serial.print(" | RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);

    } else {
      Serial.println("Receive failed");
    }
  }

  // --- SEND LOGIC ---
  // Send a message every 2 seconds
  if (millis() - lastSendTime > 2000) {
    lastSendTime = millis(); // Update the last send time

    char message[32];
    snprintf(message, sizeof(message), "Hello from Node #%d", MY_ADDRESS);

    Serial.print("Sending: ");
    Serial.println(message);

    // Send the message. This is a non-blocking call.
    rf69.send((uint8_t *)message, strlen(message));

    // Wait for the packet to be sent. This is a blocking call.
    rf69.waitPacketSent();
  }
}
