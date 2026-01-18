import streamlit as st
import time

LOG_FILE = "decoded_output.txt"

st.set_page_config(page_title="Blink Morse Logger", layout="centered")
st.title("👁️ Blink Morse Logger")
st.caption("ESP32 → Decoder → Analyzer → Streamlit")

text_placeholder = st.empty()
stats_placeholder = st.empty()

while True:
    try:
        with open(LOG_FILE, "r") as f:
            text = f.read()
    except FileNotFoundError:
        text = ""

    text_placeholder.markdown(
        f"### Decoded Text\n```\n{text}\n```"
    )

    stats_placeholder.markdown(
        f"""
        **Characters:** {len(text)}  
        **Words:** {len(text.split())}
        """
    )

    time.sleep(1)
