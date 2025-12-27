import time
import serial
import spotipy
from spotipy.oauth2 import SpotifyOAuth

# -------- SERIAL --------
ser = serial.Serial("COM8", 115200, timeout=1)

# -------- SPOTIFY --------
sp = spotipy.Spotify(auth_manager=SpotifyOAuth(
    client_id="YOUR_CLIENT_ID",
    client_secret="YOUR_CLIENT_SECRET",
    redirect_uri="LOCALHOST_CALLBACK_URL",
    scope="user-read-currently-playing user-read-playback-state user-modify-playback-state"
))

last_payload = ""

def toggle_playback(sp):
    playback = sp.current_playback()
    if playback and playback["is_playing"]:
        sp.pause_playback()
    else:
        sp.start_playback()

while True:
    # ---- READ BUTTON COMMAND ----
    if ser.in_waiting:
        line = ser.readline().decode(errors="ignore").strip()

        if line == "TOGGLE":
            toggle_playback(sp)
            time.sleep(0.3)

    # ---- READ CURRENT TRACK ----
    track = sp.current_user_playing_track()

    if track and track["item"]:
        song = track["item"]["name"]
        artist = track["item"]["artists"][0]["name"]
        is_playing = track["is_playing"]
        progress = track["progress_ms"]
        duration = track["item"]["duration_ms"]

        payload = (
            f"SONG:{song}\n"
            f"ARTIST:{artist}\n"
            f"STATE:{'PLAY' if is_playing else 'PAUSE'}\n"
            f"PROGRESS:{progress}\n"
            f"DURATION:{duration}\n"
        )

        if payload != last_payload:
            ser.write(payload.encode())
            last_payload = payload

    time.sleep(1)
