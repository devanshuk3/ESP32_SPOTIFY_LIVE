import time
import serial
import spotipy
from spotipy.oauth2 import SpotifyOAuth

# -------- SERIAL CONFIG --------
ser = serial.Serial("COM8", 115200, timeout=1)
time.sleep(2)  # allow ESP32 reset

# -------- SPOTIFY CONFIG --------
sp = spotipy.Spotify(auth_manager=SpotifyOAuth(
    client_id="b99cc44065594536b61a661b43cb32cb",
    client_secret="b3614ec784d6464abc60529c7a9725dd",
    redirect_uri="http://127.0.0.1:8888/callback",
    scope=(
        "user-read-currently-playing "
        "user-read-playback-state "
        "user-modify-playback-state"
    )
))

# -------- CACHE --------
last_payload = ""
last_track_id = None
cached_energy = 0
cached_bpm = 120   # safe default BPM


# -------- HELPERS --------
def toggle_playback():
    playback = sp.current_playback()
    if playback and playback["is_playing"]:
        sp.pause_playback()
    else:
        sp.start_playback()


def get_audio_features(track_id):
    """
    Fetch energy (0–100) and BPM safely.
    Called ONLY when track changes.
    """
    global cached_energy, cached_bpm

    try:
        features_list = sp.audio_features([track_id])
        if not features_list:
            return cached_energy, cached_bpm

        features = features_list[0]
        if features is None:
            return cached_energy, cached_bpm

        if features.get("energy") is not None:
            cached_energy = int(features["energy"] * 100)

        if features.get("tempo") is not None:
            cached_bpm = int(features["tempo"])

    except Exception as e:
        print("Audio feature error:", e)

    return cached_energy, cached_bpm


# -------- MAIN LOOP --------
while True:
    # ---- READ COMMAND FROM ESP32 ----
    if ser.in_waiting:
        cmd = ser.readline().decode(errors="ignore").strip()
        if cmd == "TOGGLE":
            toggle_playback()
            time.sleep(0.3)

    # ---- GET CURRENT TRACK ----
    track = sp.current_user_playing_track()

    if not track or track.get("item") is None:
        time.sleep(1)
        continue

    item = track["item"]

    song = item["name"]
    artist = item["artists"][0]["name"]
    track_id = item["id"]

    is_playing = track["is_playing"]
    progress = track["progress_ms"]
    duration = item["duration_ms"]

    # ---- FETCH ENERGY + BPM ON TRACK CHANGE ----
    if track_id != last_track_id:
        cached_energy, cached_bpm = get_audio_features(track_id)
        last_track_id = track_id

    payload = (
        f"SONG:{song}\n"
        f"ARTIST:{artist}\n"
        f"STATE:{'PLAY' if is_playing else 'PAUSE'}\n"
        f"PROGRESS:{progress}\n"
        f"DURATION:{duration}\n"
        f"VIS:{cached_energy}\n"
        f"BPM:{cached_bpm}\n"
    )

    if payload != last_payload:
        ser.write(payload.encode())
        last_payload = payload

    time.sleep(1)
