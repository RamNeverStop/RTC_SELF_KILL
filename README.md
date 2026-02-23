# ESP32-S3 DS3231 RTC Alarm-Based Power Control

An intelligent power management system using the ESP32-S3 and DS3231 Real-Time Clock (RTC) module with integrated multi-rail BUCK converter. Features alarm-based wake-up, automatic power shutdown, battery-backed timekeeping, EEPROM storage, and efficient voltage regulation (2.5V, 3.8V, 3.3V rails). Perfect for ultra-low power applications, data logging, and scheduled operations.

## Hardware Requirements

- **Microcontroller**: ESP32-S3
- **RTC Module**: DS3231 (High-precision I2C RTC)
- **EEPROM**: CAT24C02WI-G (2Kbit I2C EEPROM)
- **BUCK Converter**: TPS65262-1 (Triple-output DC-DC converter)
- **Power Management**: BS-08-B2AA002 (Load switch)
- **Backup Battery**: CR1220 coin cell (3V)
- **Diodes**: LL4148-M-18 (SOD-123)
- **LED Indicator**: BC847 SMD with status LED
- **Inductors**: Multiple (10μH, 33μH) for BUCK outputs

## Features

- ⏰ **Alarm-Based Wake/Sleep**
  - Set alarm to wake ESP32 from deep sleep
  - Automatic power shutdown after operations
  - RTC continues running on backup battery

- 🔋 **Ultra-Low Power**
  - ESP32 powered OFF between operations
  - DS3231 draws < 2μA on backup battery
  - Total system sleep current < 5μA

- 📅 **Accurate Timekeeping**
  - Temperature-compensated crystal oscillator (TCXO)
  - ±2ppm accuracy (±1 minute per year)
  - Battery backup maintains time during power loss

- 💾 **Non-Volatile Storage**
  - 2Kbit EEPROM for configuration/logs
  - Survives power cycles
  - I2C addressable memory

- 🔔 **Interrupt-Driven**
  - Hardware interrupt on alarm trigger
  - No polling required
  - INT/SQW pin generates wake signal

## Complete Power System Architecture

### Power Distribution Overview

**See Complete_Power_Schematic.png and RTC_Schematic.png for full circuit diagrams**

The system features a sophisticated multi-rail power architecture designed for efficiency and battery backup:

#### TPS65262-1 Triple-Output BUCK Converter (U13)
The heart of the power system, this integrated DC-DC converter generates three regulated voltage rails from a single input:

**Input Power:**
- VCC_LVIN1: Main input voltage (typically 3.7V-5V from battery or USB)
- Input capacitors: C67 (100μF), C58, C21, C23 (10μF each) for stability
- FB2 ferrite bead for noise filtering on AVDD_PB

**Output Rails:**

**2.5V Rail (COMP1):**
- Output inductor: L1 (33μH @ 0.5A)
- Output capacitors: C24 (0.047μF), C25, C26, C33 (22μF), C70, C71 (100μF, 0.1μF)
- Feedback: R15, R16, R17 resistor network
- Enable: BST1 control signal
- Load: VCC_4G_PB (4G module power)

**3.8V Rail (FB1):**
- Output inductor: (shown in schematic)
- Output capacitors: C80 (150pF), C25, C26, C33 (22μF)
- Feedback: R16, R17 resistor network  
- Enable: BUCK_EN1 control signal
- Load: 4G module via 4G_EN_PB

**3.3V Rail (BST2, FB3):**
- Primary rail for ESP32-S3 and peripherals
- Output inductor: L2 (33μH @ 0.5A)
- Output capacitors: C43 (0.047μF), C40, C38, C34 (10μF, 10μF, 22μF), C72, C73 (100μF, 0.1μF)
- Feedback: R22, R19, R21 resistor network
- Enable: BST2 control signal
- Load: VCC_MCU_PB (ESP32-S3 main power)

**3.3V SD Card/Audio LDO Rail:**
- Additional 3.3V rail for SD card and audio circuits
- Inductor: L3 (33μH @ 0.5A)
- Output capacitors: C44 (0.047μF), C46, C47, C49 (10μF, 10μF, 22μF), C74, C75 (100μF, 0.1μF)
- Feedback: R23, R35, R26 resistor network
- Enable: BST3 control signal
- Load: VCC_BUCK_LDO_PB

**Key Features:**
- Switching frequency: Configurable via external components
- Efficiency: >90% typical across all rails
- Current capability: 0.5A per rail
- Synchronous rectification for low heat
- Independent enable controls for each rail
- Integrated soft-start and current limiting

#### Power Sequencing and Control

**Enable Signal Hierarchy:**
```
BUCK_EN2 (Main enable)
    ↓
TPS65262-1 BUCK Converter
    ├→ 2.5V (BST1) → 4G Module
    ├→ 3.8V (BUCK_EN1) → 4G Module High Power
    ├→ 3.3V (BST2) → ESP32-S3 (VCC_MCU_PB)
    └→ 3.3V (BST3) → SD Card/Audio (VCC_BUCK_LDO_PB)
```

**Testpoint Locations (for debugging):**
- U10: TESTPOINT on VBAT_SW line
- Solder jumpers: J2, J6, J7 for rail configuration
- NM (Not Mounted) positions for optional components

#### RTC and Backup Power Section

**DS3231 Real-Time Clock (U16):**
- Location: Lower left corner of schematic (RTC section)
- Power input: MID_VCC (from load switch)
- Interface: I2C (SCL pin 15, SDA pin 14)
- Alarm output: INT/SQW pin (active LOW on alarm, connected to SQWINT net)
- Backup power: VBAT pin 13 connected to battery backup circuit
- Crystal: Integrated 32.768 kHz TCXO (32KHZ pin 1)
- Ground: Pin 13 (DS3231 GND)

**CAT24C02WI-G EEPROM (U17):**
- Location: Adjacent to DS3231 in RTC section
- Capacity: 2Kbit (256 bytes) for configuration/log storage
- Interface: I2C (shares bus with DS3231)
  - SCL: Pin 6 → SCL_PW
  - SDA: Pin 5 → SDA_PW
- Address configuration: A0, A1, A2 (pins 1-3) tied to MID_VCC via pull-ups
- Default address: 0x50 (all address pins HIGH)
- Power: VCC pin 8 connected to MID_VCC
- Ground: VSS pin 4

**BS-08-B2AA002 Load Switch:**
- Location: Center of RTC section (U18)
- Function: Controls MID_VCC power to RTC and EEPROM
- Enable control: Connected to SQWINT signal (RTC alarm triggers this)
- Input: BUCK_EN2 via resistors R55, R56, R60
- Output: MID_VCC rail
- LED indicator: BC847 transistor (U18) with series resistor R9
- When enabled: LED lights, MID_VCC active
- When disabled: System powers down, RTC runs on battery

**Battery Backup System:**
- Battery: BT (CR1220 coin cell, 3V nominal)
- Primary path: BUCK_EN2 → D1 (LL4148-M-18) → DS3231 VBAT
- Secondary path: Battery → D2 (LL4148-M-18) → DS3231 VBAT
- Diode function: Prevents backfeeding between power sources
- Automatic switchover: When MID_VCC drops, battery takes over seamlessly
- Battery life: 1-2 years typical (RTC draws <2μA on backup)

**SQWINT Signal Network:**
- Origin: DS3231 INT/SQW pin (pin 3)
- Connection: Feeds to SQWINT net (shown in top portion)
- Function: Alarm interrupt signal (active LOW)
- Pull-up: Internal or external (typically 10kΩ)
- Destination: ESP32-S3 GPIO 2 (CLOCK_INTERRUPT_PIN in code)
- Also controls: Load switch enable for power management

#### Power Flow Architecture

```
Input (3.7-5V)
    ↓
[TPS65262-1 BUCK Converter]
    ├→ 2.5V → LEN1 → MIC_EN_PB (4G module)
    ├→ 3.8V → BUCK_EN1 → 4G_EN_PB (4G module high power)
    ├→ 3.3V → BST2 → VCC_MCU_PB (ESP32-S3)
    └→ 3.3V → BST3 → SD_EN_PB (SD card/audio)

MID_VCC Generation:
BUCK_EN2 → [BS-08 Load Switch] → MID_VCC
                ↓
         (enabled by SQWINT)
                ↓
        MID_VCC → DS3231 VCC
        MID_VCC → EEPROM VCC

Backup Path:
BUCK_EN2/Battery → Diodes (D1/D2) → DS3231 VBAT
```

**Power Domains:**
- **Always-On Domain**: DS3231 VBAT (battery-backed, <2μA)
- **Controlled Domain**: MID_VCC (load switch controlled, 200μA)
- **Main Domain**: VCC_MCU_PB (ESP32-S3, 50-150mA when active)
- **Peripheral Domain**: 4G, SD card, audio (application-dependent)

**Voltage Regulation Summary:**

| Rail | Voltage | Load | Enable | Notes |
|------|---------|------|--------|-------|
| 2.5V | 2.5V | 4G Module | BST1 | LDO input or direct 4G power |
| 3.8V | 3.8V | 4G Module | BUCK_EN1 | High power 4G operations |
| 3.3V | 3.3V | ESP32-S3 | BST2 | Main MCU power |
| 3.3V | 3.3V | SD/Audio | BST3 | Isolated from MCU noise |
| MID_VCC | ~3.3V | RTC/EEPROM | SQWINT | Load switch output |
| VBAT | 3.0V | DS3231 | Always | Battery backup |

## Pin Configuration

### I2C Bus Connections

| ESP32-S3 Pin | Function | DS3231 Pin | EEPROM Pin |
|--------------|----------|------------|------------|
| Default SDA  | SDA      | SDA (14)   | SDA (5)    |
| Default SCL  | SCL      | SCL (15)   | SCL (6)    |
| 3.3V         | Power    | VCC (16)   | VCC (8)    |
| GND          | Ground   | GND (13)   | VSS (4)    |

### Control Pins

| ESP32-S3 Pin | Function | Connected To | Description |
|--------------|----------|--------------|-------------|
| GPIO 2       | INT      | DS3231 INT/SQW | Alarm interrupt input |
| GPIO 1       | DONE     | Load Switch EN | Power control output |

### DS3231 I2C Address
- **Default**: 0x68 (7-bit address)
- **Fixed**: Cannot be changed

### EEPROM I2C Address
- **Base**: 0x50
- **Configurable**: A0, A1, A2 pins set address
- **Current**: 0x50 (all address pins LOW)

## How It Works

### System Operation Cycle

```
1. RTC Alarm triggers → INT/SQW goes LOW
2. Load switch enables → Power flows to ESP32
3. ESP32 boots up
4. Read current time from RTC
5. Perform task (log data, send message, etc.)
6. Set next alarm (30 seconds in this example)
7. Pull DONE (GPIO 1) LOW → Power cuts off
8. RTC continues on backup battery
9. Wait for next alarm...
```

### Alarm Trigger Mechanism

**DS3231 INT/SQW Pin Behavior:**
- **Normal**: HIGH (pulled up)
- **Alarm Match**: Goes LOW
- **After Clear**: Returns HIGH

**ESP32 Response:**
```cpp
1. Interrupt detected on GPIO 2 (FALLING edge)
2. ISR (onAlarm) executes
3. Clear alarm flag → INT/SQW returns HIGH
4. Ready for next alarm
```

### Power Control

**DONE Pin (GPIO 1) States:**
- **HIGH**: Load switch ON → System powered
- **LOW**: Load switch OFF → System shut down

**Sequence:**
```cpp
digitalWrite(DONE, HIGH);  // System stays on
// Do work...
digitalWrite(DONE, LOW);   // Kill power to self
```

## Required Libraries

Install through Arduino Library Manager:

- **RTClib** by Adafruit

## Installation

1. **Clone this repository:**
   ```bash
   git clone https://github.com/yourusername/esp32-s3-rtc-power-control.git
   ```

2. **Open in Arduino IDE:**
   - Open `kill_rtc.ino`

3. **Install required library:**
   - **Sketch** → **Include Library** → **Manage Libraries**
   - Search for "RTClib" by Adafruit
   - Click Install

4. **Select your board:**
   - **Tools** → **Board** → **ESP32 Arduino** → **ESP32S3 Dev Module**

5. **Upload the sketch**

6. **Observe behavior:**
   - System powers up
   - Runs for ~30 seconds
   - Powers down
   - Repeats cycle

## Code Explanation

### Setup Sequence

```cpp
1. Initialize Serial (115200 baud)
2. Set DONE pin HIGH (keep power on)
3. Initialize DS3231 via I2C
4. Disable all alarms
5. Clear alarm flags
6. Disable square wave output
7. Check if RTC lost power → Set time if needed
8. Get current time
9. Calculate alarm time (current + 30 seconds)
10. Set Alarm 1
11. Attach interrupt to GPIO 2
12. Set DONE pin LOW (kill power)
```

### Alarm Types

**DS3231 Alarm 1 Modes:**

| Mode | When Alarm Triggers |
|------|---------------------|
| DS3231_A1_PerSecond | Every second |
| DS3231_A1_Second | When seconds match |
| DS3231_A1_Minute | When minutes + seconds match |
| DS3231_A1_Hour | When hours + minutes + seconds match |
| DS3231_A1_Date | When date + time match |
| DS3231_A1_Day | When day + time match |

**Current Example:**
```cpp
rtc.setAlarm1(alarmTime, DS3231_A1_Second);
// Triggers when seconds field matches alarmTime.second()
```

## Serial Monitor Output

### Successful Operation
```
power up

RTC initialized successfully
Alarm 1 set for: 14:32:45
FALLING start 
FALLING end

low kill pin
[System powers down]
```

### After Alarm Triggers (Next Wake)
```
power up

Alarm triggered! INT/SQW pin pulled LOW.
Alarm 1 set for: 14:33:15
FALLING start 
FALLING end

low kill pin
[System powers down]
```

## Customization

### Change Alarm Interval

**30 Seconds (Current):**
```cpp
DateTime alarmTime = now + TimeSpan(0, 0, 0, 30);
```

**1 Minute:**
```cpp
DateTime alarmTime = now + TimeSpan(0, 0, 1, 0);
```

**5 Minutes:**
```cpp
DateTime alarmTime = now + TimeSpan(0, 0, 5, 0);
```

**1 Hour:**
```cpp
DateTime alarmTime = now + TimeSpan(0, 1, 0, 0);
```

**1 Day:**
```cpp
DateTime alarmTime = now + TimeSpan(1, 0, 0, 0);
```

**TimeSpan Format:** `(days, hours, minutes, seconds)`

### Different Alarm Modes

**Alarm every minute at :00 seconds:**
```cpp
rtc.setAlarm1(DateTime(0, 0, 0, 0, 0, 0), DS3231_A1_Minute);
```

**Alarm every hour at :00 minutes:**
```cpp
rtc.setAlarm1(DateTime(0, 0, 0, 0, 0, 0), DS3231_A1_Hour);
```

**Alarm at specific time daily:**
```cpp
DateTime alarmTime(2026, 1, 1, 8, 0, 0); // 8:00 AM
rtc.setAlarm1(alarmTime, DS3231_A1_Hour);
```

### Use Alarm 2 (Less Precise)

```cpp
// Alarm 2 has minute resolution (no seconds)
rtc.setAlarm2(DateTime(0, 0, 0, 14, 35, 0), DS3231_A2_Minute);
```

**Alarm 2 Modes:**
- DS3231_A2_PerMinute
- DS3231_A2_Minute
- DS3231_A2_Hour
- DS3231_A2_Date
- DS3231_A2_Day

### Add Task Execution

```cpp
void setup() {
  // ... initialization ...
  
  // Perform your task here before shutdown
  readSensorData();
  sendDataToCloud();
  logToEEPROM();
  
  // Then set alarm and power down
  rtc.setAlarm1(alarmTime, DS3231_A1_Second);
  digitalWrite(DONE, LOW);
}
```

## EEPROM Usage

### Writing to EEPROM

```cpp
#include <Wire.h>

#define EEPROM_ADDR 0x50

void writeEEPROM(uint16_t address, uint8_t data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5); // EEPROM write cycle time
}
```

### Reading from EEPROM

```cpp
uint8_t readEEPROM(uint16_t address) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(address >> 8));
  Wire.write((int)(address & 0xFF));
  Wire.endTransmission();
  
  Wire.requestFrom(EEPROM_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF; // Error
}
```

### Data Logging Example

```cpp
struct LogEntry {
  uint32_t timestamp;
  float temperature;
  uint16_t battery_mv;
};

void saveLog(uint16_t addr, LogEntry log) {
  uint8_t* ptr = (uint8_t*)&log;
  for (uint16_t i = 0; i < sizeof(LogEntry); i++) {
    writeEEPROM(addr + i, ptr[i]);
  }
}
```

## Advanced Features

### Temperature Reading

DS3231 has built-in temperature sensor:

```cpp
float temperature = rtc.getTemperature();
Serial.print("Temperature: ");
Serial.print(temperature);
Serial.println(" °C");
```

### Check Alarm Status

```cpp
bool alarm1Triggered = rtc.alarmFired(1);
bool alarm2Triggered = rtc.alarmFired(2);

if (alarm1Triggered) {
  Serial.println("Alarm 1 fired!");
  rtc.clearAlarm(1);
}
```

### 32kHz Output (for external circuits)

```cpp
// Enable 32.768 kHz square wave on SQW pin
rtc.writeSqwPinMode(DS3231_SquareWave32kHz);

// Other options:
// DS3231_OFF
// DS3231_SquareWave1kHz
// DS3231_SquareWave4kHz
// DS3231_SquareWave8kHz
```

### Aging Offset Calibration

```cpp
// Fine-tune RTC accuracy
int8_t offset = rtc.readAgingOffset();
Serial.print("Current offset: ");
Serial.println(offset);

// Adjust if needed (range: -128 to +127)
rtc.writeAgingOffset(0); // Reset to factory
```

## Power Consumption Analysis

### Active Mode (ESP32 ON)
- ESP32-S3: ~50-150 mA (from 3.3V VCC_MCU_PB rail)
- DS3231: 200 μA (from MID_VCC)
- EEPROM: 1 mA during write, <1μA standby (from MID_VCC)
- BUCK Converter: ~10-15 mA quiescent (TPS65262-1)
- 4G Module: 100-500 mA when active (from 2.5V and 3.8V rails)
- **Total Active (without 4G)**: ~60-165 mA
- **Total Active (with 4G)**: ~160-665 mA

### Sleep Mode (ESP32 OFF, BUCK rails disabled)
- DS3231: < 2 μA (on battery backup)
- EEPROM: < 1 μA (standby)
- BUCK Converter: < 1 μA (shutdown mode)
- Load Switch: < 1 μA (off state)
- **Total Sleep**: < 5 μA

### BUCK Converter Efficiency
The TPS65262-1 provides excellent efficiency across all three rails:
- Light load (10-50mA): 85-90% efficient
- Medium load (50-200mA): 90-93% efficient
- Heavy load (200-500mA): 88-92% efficient
- Multiple rail operation: Minimal cross-regulation

### Power Rail Startup Timing
```
t=0ms:    BUCK_EN2 asserted → TPS65262-1 starts
t=1ms:    2.5V rail reaches regulation
t=2ms:    3.8V rail reaches regulation (if BUCK_EN1 enabled)
t=3ms:    3.3V rails reach regulation (BST2, BST3)
t=5ms:    MID_VCC stable (load switch enabled by SQWINT)
t=10ms:   All rails stable, ESP32-S3 begins boot sequence
```

### Battery Life Estimation

**CR1220 Battery (40 mAh typical):**
```
Sleep current: 3 μA average (RTC + minimal leakage)
Sleep-only life = 40,000 μAh / 3 μA = 13,333 hours ≈ 1.5 years

With 30-second wake cycles (5 seconds active):
Active time per day: 480 wake cycles × 5s = 2,400s = 40 minutes  
Active duty cycle: 40 minutes / 1440 minutes = 2.8%

Average current: (0.972 × 3μA) + (0.028 × 50,000μA) ≈ 1,400 μA
Battery life = 40,000 μAh / 1,400 μA = 28.5 hours ≈ 1.2 days

Recommendation: Use main power (BUCK_EN2) with battery backup only for RTC
The battery is designed to maintain timekeeping during power outages, not
to power the complete system during normal operation.
```

**BUCK Input Power Requirements:**
```
Input voltage range: 2.3V - 5.5V (VCC_LVIN1)
Recommended input: 3.7V Li-ion or 5V USB
Peak current draw: 500-800mA (all rails active with 4G module)
Input capacitance: 220μF total (C67 + C58 + C21 + C23)
```

## Troubleshooting

### RTC Not Found
- **Check I2C wiring**: Verify SDA and SCL connections
- **Check power**: DS3231 needs 3.3V on VCC
- **Pull-ups**: Ensure 4.7kΩ pull-ups on SDA/SCL
- **I2C scan**: Run I2C scanner to find device at 0x68

### Alarm Not Triggering
- **Check interrupt pin**: GPIO 2 connected to INT/SQW?
- **Pull-up**: INT/SQW needs pull-up (internal or external)
- **Alarm not set**: Verify alarm configuration
- **Alarm cleared**: May have auto-cleared, check status

### System Won't Power Down
- **DONE pin**: Verify GPIO 1 connected to load switch
- **Load switch**: Check BS-08 wiring and enable logic
- **Diode direction**: D1/D2 must be correctly oriented

### Time Keeps Resetting
- **Battery dead**: Replace CR1220 coin cell
- **Battery connection**: Check diodes and connections
- **VBAT pin**: Ensure pin 13 connected to battery circuit
- **Lost power flag**: Check with `rtc.lostPower()`

### EEPROM Not Working
- **Address**: Verify EEPROM at 0x50
- **Write delay**: Must wait 5ms after write
- **Page boundary**: Don't cross 8-byte page boundaries
- **I2C speed**: Some EEPROM need slower clock (100kHz)

### Interrupt Not Firing
- **ISR attribute**: Ensure `IRAM_ATTR` on interrupt function
- **Edge trigger**: Use FALLING edge for INT/SQW
- **Clear alarm**: Must clear alarm flag in ISR
- **Debounce**: Add small delay if seeing multiple triggers

## Real-World Applications

### Data Logger
```cpp
// Wake every hour, log sensor data to EEPROM
DateTime alarmTime = now + TimeSpan(0, 1, 0, 0);
float temp = readTemperature();
saveLogEntry(temp);
rtc.setAlarm1(alarmTime, DS3231_A1_Hour);
```

### Garden Irrigation Controller
```cpp
// Water plants every 12 hours at 6 AM and 6 PM
DateTime morningTime(2026, 1, 1, 6, 0, 0);
DateTime eveningTime(2026, 1, 1, 18, 0, 0);
controlValve(ON);
delay(60000); // Water for 1 minute
controlValve(OFF);
```

### Solar-Powered Weather Station
```cpp
// Take readings every 15 minutes during daylight
if (isDaytime()) {
  readAllSensors();
  transmitData();
  DateTime next = now + TimeSpan(0, 0, 15, 0);
} else {
  DateTime next = now + TimeSpan(0, 1, 0, 0); // Hourly at night
}
```

### Wildlife Camera Trigger
```cpp
// Check motion sensor every 5 minutes
if (motionDetected()) {
  captureImage();
  saveToSD();
}
DateTime next = now + TimeSpan(0, 0, 5, 0);
```

## Safety & Best Practices

⚠️ **IMPORTANT**

- **Backup battery**: Replace CR1220 every 1-2 years
- **Power sequencing**: Always pull DONE LOW after setting alarm
- **Interrupt clearing**: Must clear alarm flag to re-arm
- **I2C pull-ups**: Required for reliable communication (4.7kΩ typical)
- **Load switch rating**: Ensure BS-08 can handle ESP32 current spikes
- **Decoupling**: Add 10μF + 100nF caps near DS3231 VCC
- **ESD protection**: Handle RTC module with anti-static precautions

## Component Specifications

### TPS65262-1 BUCK Converter (U13)
- **Manufacturer**: Texas Instruments
- **Type**: Triple-output synchronous buck converter
- **Input Voltage**: 2.3V - 5.5V (VCC_LVIN1)
- **Output Voltages**: Configurable (2.5V, 3.8V, 3.3V in this design)
- **Output Current**: 600mA per rail (typical)
- **Switching Frequency**: ~600kHz (adjustable)
- **Efficiency**: Up to 95% (load dependent)
- **Quiescent Current**: 75μA per rail (typical)
- **Shutdown Current**: <1μA
- **Package**: QFN or similar (check datasheet for exact type)

**Associated Components for BUCK Converter:**

**Input Capacitors:**
- C57, C67: 100μF (input bulk capacitance)
- C58, C21, C23: 10μF each (input decoupling)
- C45, C20, C22: 100pF, 0.1μF, 10μF (high-frequency filtering)

**Output Inductors:**
- L1: 10μH (for 2.5V rail, labeled on schematic)
- L2: 33μH @ 0.5A (for 3.3V MCU rail)
- L3: 33μH @ 0.5A (for 3.3V SD/Audio rail)

**Output Capacitors per Rail:**

2.5V Rail:
- C24: 0.047μF (high-frequency bypass)
- C25, C26, C33: 22μF each (bulk storage)
- C70, C71: 100μF, 0.1μF (output filtering)

3.3V MCU Rail:
- C43: 0.047μF (high-frequency bypass)
- C40, C38, C34: 10μF, 10μF, 22μF (bulk storage)
- C72, C73: 100μF, 0.1μF (output filtering)

3.3V SD/Audio Rail:
- C44: 0.047μF (high-frequency bypass)
- C46, C47, C49: 10μF, 10μF, 22μF (bulk storage)
- C74, C75: 100μF, 0.1μF (output filtering)

**Feedback Networks:**
- 2.5V Rail: R15, R16, R17 resistor divider
- 3.3V MCU: R22, R19, R21 resistor divider
- 3.3V SD: R23, R35, R26 resistor divider
- (Exact values determine output voltage)

**Compensation Components:**
- C54: 22pF (COMP1 compensation)
- C53: 3300pF (compensation network)
- R36: 330Ω (series resistor)

### DS3231 Real-Time Clock
- **Accuracy**: ±2ppm (±1 min/year) at 25°C
- **Temperature Range**: -40°C to +85°C
- **Current**: 200 μA @ 3.3V (active), <2 μA (backup)
- **Battery Voltage**: 2.3V - 5.5V
- **Crystal**: Integrated 32.768 kHz TCXO
- **Alarms**: 2 programmable alarms
- **Interface**: I2C (100kHz, 400kHz)
- **Address**: 0x68 (fixed)

### CAT24C02WI-G EEPROM
- **Capacity**: 2 Kbit (256 × 8)
- **Organization**: 256 bytes
- **Page Size**: 8 bytes
- **Write Cycle**: 5ms typical
- **Endurance**: 1,000,000 cycles
- **Retention**: 100 years @ 25°C
- **Interface**: I2C (100kHz, 400kHz, 1MHz)
- **Address**: 0x50-0x57 (configurable via A0-A2)
- **Voltage**: 1.7V - 5.5V
- **Current**: 1mA write, <1μA standby

### BS-08-B2AA002 Load Switch
- **Type**: P-channel MOSFET load switch
- **Input Voltage**: Up to 5.5V
- **Output Current**: Up to 500mA continuous
- **On-Resistance**: Low RDS(on) for minimal voltage drop
- **Enable Logic**: Active HIGH or LOW (verify datasheet)
- **Package**: SOT-23-5 or similar

### LL4148-M-18 Diodes (D1, D2)
- **Type**: Fast switching diode
- **Forward Voltage**: ~0.7V @ 10mA
- **Reverse Voltage**: 100V
- **Forward Current**: 300mA continuous
- **Package**: SOD-123 (M-18)
- **Application**: Battery backup isolation

### BC847 Transistor (U18)
- **Type**: NPN general-purpose transistor
- **Collector Current**: 100mA max
- **Voltage**: 45V max
- **Package**: SOT-23
- **Application**: LED indicator driver

### CR1220 Coin Cell
- **Nominal Voltage**: 3V
- **Capacity**: 35-40 mAh
- **Diameter**: 12.5 mm
- **Height**: 2.0 mm
- **Chemistry**: Lithium Manganese Dioxide
- **Typical Life**: 1-2 years for RTC backup

## Schematic References

This project includes two detailed schematic diagrams:

**Complete_Power_Schematic.png** - Full system architecture showing:
- TPS65262-1 triple-output BUCK converter (U13) with all three voltage rails
- 2.5V rail generation with inductor L1 and feedback network (R15, R16, R17)
- 3.8V rail for 4G module high-power operations
- 3.3V rail for ESP32-S3 with inductor L2 and feedback network (R22, R19, R21)
- 3.3V SD card/audio LDO rail with inductor L3 and feedback network (R23, R35, R26)
- Input power section with capacitor bank and ferrite beads
- Enable signal routing (BST1, BST2, BST3, BUCK_EN1, BUCK_EN2, BUCK_EN3)
- RTC section in lower left showing DS3231, EEPROM, and battery backup
- All component values, test points, and solder jumper locations

**RTC_Schematic.png** - Detailed RTC subsystem showing:
- DS3231 real-time clock (U16) with I2C connections
- CAT24C02 EEPROM (U17) with address configuration
- BS-08-B2AA002 load switch (U18) with enable control
- Battery backup circuit with CR1220 and protection diodes (D1, D2)
- BC847 LED indicator circuit (U18) for status monitoring
- SQWINT signal network connecting alarm output to system
- MID_VCC power distribution to RTC and EEPROM
- Pin-by-pin connections and component placement

## License

This project is open source and available under the MIT License.

## Contributing

Pull requests are welcome! Areas for improvement:
- Add deep sleep integration
- Implement EEPROM wear leveling
- Create calendar alarm examples
- Add temperature compensation algorithms

## Author

Your Name

## Resources

- [DS3231 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf)
- [RTClib Documentation](https://adafruit.github.io/RTClib/html/index.html)
- [CAT24C02 Datasheet](https://www.onsemi.com/pdf/datasheet/cat24c02-d.pdf)
- [ESP32-S3 Datasheet](https://www.espressif.com/en/products/socs/esp32-s3)
- [I2C Protocol Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)

## Acknowledgments

- Analog Devices (Maxim) for DS3231 RTC
- Adafruit for RTClib library
- ON Semiconductor for CAT24C02 EEPROM
- Espressif for ESP32-S3 platform
- Arduino community for development tools
