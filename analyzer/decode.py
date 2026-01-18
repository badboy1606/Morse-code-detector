import time

LOG_FILE = "decoded_output.txt"

def read_decoded_text():
    try:
        with open(LOG_FILE, "r") as f:
            return f.read()
    except FileNotFoundError:
        return ""

def analyze_text(text):
    """
    You can expand this later:
    - word count
    - blink count
    - timestamps
    """
    return {
        "text": text,
        "length": len(text),
        "words": len(text.split())
    }

if __name__ == "__main__":
    while True:
        text = read_decoded_text()
        stats = analyze_text(text)

        print("Decoded Text:", stats["text"])
        print("Characters:", stats["length"])
        print("Words:", stats["words"])
        print("-" * 30)

        time.sleep(1)
