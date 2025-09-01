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

---

## 🛠 Hardware Required
- **Arduino UNO / Nano / Leonardo** (or compatible board)  
- **DFRobot DF1201S** MP3 player module  
- **Micro SD card** with `DeadMenTellNoTales.mp3` (or your own file)  
- **Servo motor** (standard SG90 or MG90S works, range calibrated 0–45°)  
- **External 5–6V power supply** for servo (recommended, not from Arduino 5V pin)  
- Jumper wires and breadboard  

---

## 🔌 Wiring

### DF1201S → Arduino
| DF1201S Pin | Arduino UNO Pin | Notes |
|-------------|-----------------|-------|
| VCC         | 5V              | Module power |
| GND         | GND             | Common ground |
| RX          | D3 (SoftwareSerial TX) | Arduino → DF1201S |
| TX          | D2 (SoftwareSerial RX) | DF1201S → Arduino |

### Servo → Arduino
| Servo Pin   | Arduino Pin | Notes |
|-------------|-------------|-------|
| Signal      | D9          | PWM signal pin |
| VCC         | 5–6V ext.   | Separate supply for servo recommended |
| GND         | GND         | Must connect to Arduino GND |

---

## 📂 File Setup
1. Place your MP3 file on the SD card. Example:   /mp3/0001.mp3
2. The sketch will call `DF1201S.start()` to play the first track.  

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
