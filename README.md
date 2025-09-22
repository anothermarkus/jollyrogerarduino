# Dead Men Tell No Tales — Talking Skull Servo Sync

This project syncs a **servo-driven jaw** (animatronic skull or puppet) with audio playback from a **DFRobot DF1201S MP3 player module**. It makes the skull "say" the phrase:

> **“Dead men tell no tales.”**

The Arduino sketch smoothly animates the jaw across a calibrated range of **0° to 45°**, timed to match the audio playback for a realistic lip-sync effect.  

---

## ✨ Features
- Smooth servo motion with multiple jaw positions (`0–45°`)  
- Pre-timed sequence for *Dead men tell no tales* (~11 seconds)  
- DF1201S MP3 playback integration (UART)  
- Automatic sync offset (~150 ms) so the jaw starts moving when the sound starts  
- Simple to extend for new audio files or phrases  
- Adding a new feature to sing the Yo Ho Pirate Song

---

## 🛠 Hardware Required
- **Arduino Uno (or compatible)**
- **DFRobot DFPlayer Pro (DF1201S)**
- **Servo motor (mouth/jaw)**
- **HC-SR04 Ultrasonic sensor**
- **LED (with resistor ~220Ω)**
- **Aux output (3.5mm jack for DFPlayer PRO)**
- **Wires, breadboard, or perfboard**

---

## Connections

### Servo (mouth)
- Servo **signal** → Arduino **D9**
- Servo **VCC (red)** → **5V**
- Servo **GND (brown/black)** → **GND**

### HC-SR04 Ultrasonic Sensor
- **VCC** → **5V**
- **GND** → **GND**
- **Trig** → Arduino **D10**
- **Echo** → Arduino **D13**

### LED
- LED **anode (+)** → Arduino **D8** (through **220Ω resistor** to limit current)
- LED **cathode (–)** → **GND**

### DFPlayer PRO (DF1201S)
- **RX** (DFPlayer) → Arduino **D3 (TX of SoftwareSerial)**
- **TX** (DFPlayer) → Arduino **D2 (RX of SoftwareSerial)**
- **VCC** → **5V**
- **GND** → **GND**
- **DACR / DACL** → 3.5 mm AUX jack → external powered speakers or amplifier

---

## Power Notes
- Servo draws significant current → may need **separate 5V supply** (common ground with Arduino).
- DFPlayer PRO also powered from **5V**.
- Keep Arduino USB connected or provide regulated **5V power**.

---

## 📂 File Setup
1. Place your MP3 file on the SD card. Example:   /mp3/0001.mp3
2. The sketch will call `DF1201S.start()` to play the first track.  

  # Wiring Table — Arduino Uno + DFPlayer Pro + Servo + HC-SR04 + LED

| **Arduino Uno Pin** | **Connection**                        | **Target Device Pin**      |
|----------------------|---------------------------------------|-----------------------------|
| D2                  | RX (SoftwareSerial)                   | DFPlayer **TX**            |
| D3                  | TX (SoftwareSerial)                   | DFPlayer **RX**            |
| D9                  | Servo control signal                  | Servo **Signal**           |
| D10                 | Trigger output                        | HC-SR04 **Trig**           |
| D13                 | Echo input                            | HC-SR04 **Echo**           |
| D8                  | Digital output (with 220Ω resistor)   | LED **Anode (+)**          |
| 5V                  | Power supply                          | Servo VCC, HC-SR04 VCC, DFPlayer VCC |
| GND                 | Common ground                         | Servo GND, HC-SR04 GND, DFPlayer GND, LED Cathode |
| —                   | Audio output                          | DFPlayer **DACL/DACR → AUX Jack → Powered Speakers** |


---

## ▶️ Usage
1. Upload the sketch to your Arduino.  
2. Power everything on.  
3. The DF1201S will play the MP3, and the jaw servo will sync with the audio.  
4. The sequence repeats with a short pause.  

---

## ⚙️ Calibration
- Servo range is set for **0° (closed)** to **45° (open)**.  
- If your servo moves the wrong direction:
- Flip the servo horn physically, or  
- Swap `JAW_CLOSED = 45` and `JAW_OPEN = 0` in the code.  

---

## 📜 License
MIT License — feel free to use, modify, and share.  
