import serial
import time

# =======================
# Morse Code Dictionary
# =======================
MORSE_MAP = {
    ".-": "A",   "-...": "B", "-.-.": "C", "-..": "D",  ".": "E",
    "..-.": "F", "--.": "G",  "....": "H", "..": "I",   ".---": "J",
    "-.-": "K",  ".-..": "L", "--": "M",   "-.": "N",   "---": "O",
    ".--.": "P", "--.-": "Q", ".-.": "R",  "...": "S",  "-": "T",
    "..-": "U",  "...-": "V", ".--": "W",  "-..-": "X",
    "-.--": "Y", "--..": "Z",

    "-----": "0", ".----": "1", "..---": "2", "...--": "3",
    "....-": "4", ".....": "5", "-....": "6", "--...": "7",
    "---..": "8", "----.": "9"
}

# =======================
# Timing Thresholds (seconds)
# =======================
LETTER_GAP = 0.7   # pause between letters
WORD_GAP   = 1.5   # pause between words

# =======================
# Serial Setup
# =======================
PORT = "COM3"      # change this
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.1)

print("Listening for Morse code...")

# =======================
# Decoder State
# =======================
current_symbol = ""
decoded_text = ""
last_time = time.time()

# =======================
# Main Loop
# =======================
while True:
    data = ser.readline().decode().strip()
    now = time.time()

    # Check gaps
    gap = now - last_time

    if gap > WORD_GAP and current_symbol:
        decoded_text += MORSE_MAP.get(current_symbol, "?") + " "
        current_symbol = ""

    elif gap > LETTER_GAP and current_symbol:
        decoded_text += MORSE_MAP.get(current_symbol, "?")
        current_symbol = ""

    # Read dot / dash
    if data == "." or data == "-":
        current_symbol += data
        last_time = now
        print(f"Symbol: {data} | Current: {current_symbol}")

    with open("decoded_output.txt", "w") as f:
        f.write(decoded_text)


    # Print decoded output continuously
    print("Decoded:", decoded_text, end="\r")
