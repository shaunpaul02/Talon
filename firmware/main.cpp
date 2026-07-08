#include <Wire.h>

// Multiplexer Control Pins (S0, S1, S2, S3)
const int muxPins[] = {19, 23, 5, 17}; 
const int SIG_PIN = 32; // Multiplexer Output (ADC1)

// Piezo Haptic/Audio Pin
const int PIEZO_PIN = 18;                   
const int PWM_CHANNEL = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  // Set up Mux digital control pins
  for (int i = 0; i < 4; i++) {
    pinMode(muxPins[i], OUTPUT);
  }
  
  // Initialize Piezo PWM channel
  ledcSetup(PWM_CHANNEL, 2000, 8); // 2kHz resonant frequency
  ledcAttachPin(PIEZO_PIN, PWM_CHANNEL);
  
  // Wake up MPU-6050 via I2C
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);
}

// Function to route the Multiplexer to a specific sensor channel
int readMuxChannel(int channel) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(muxPins[i], (channel >> i) & 1);
  }
  delayMicroseconds(10); // Let the voltage stabilize
  return analogRead(SIG_PIN);
}

// Function: Sharp Tactile Confirmation (Click Mode)
void triggerHapticClick() {
  ledcWrite(PWM_CHANNEL, 255); 
  delay(15);                     // Sharp 15ms mechanical hit
  ledcWrite(PWM_CHANNEL, 0);
}

// Function: Bone Conductive Tone (Audio Mode)
void playAudioTone() {
  ledcWriteTone(PWM_CHANNEL, 2500); // 2.5kHz tone
  delay(200);
  ledcWriteTone(PWM_CHANNEL, 0);    
}

void loop() {
  Serial.print("DATA,");
  
  // Read all 12 analog sensors sequentially through the Multiplexer
  // C0-C5: Joints | C6-C8: Flex | C9-C11: Pressure
  for (int i = 0; i < 12; i++) {
    Serial.print(readMuxChannel(i));
    Serial.print(",");
  }
  
  // Read Spatial Orientation (MPU-6050)
  Wire.beginTransmission(0x68);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true);
  
  int16_t ax = (Wire.read() << 8) | Wire.read(); 
  int16_t ay = (Wire.read() << 8) | Wire.read(); 
  int16_t az = (Wire.read() << 8) | Wire.read(); 
  
  Serial.print(ax); Serial.print(",");
  Serial.print(ay); Serial.print(",");
  Serial.print(az); Serial.println(); // End packet
  
  // Listen for PC Commands to fire the Piezo
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'C') triggerHapticClick();
    else if (command == 'A') playAudioTone();
  }
  
  delay(20); // ~50Hz refresh rate
}
