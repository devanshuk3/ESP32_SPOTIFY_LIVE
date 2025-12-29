# ESP32_SPOTIFY_LIVE

ESP32_SPOTIFY_LIVE is an **ESP32-based project that connects to the Spotify Web API** and displays live playback information (such as current track, artist, album art, and playback state). It can also optionally control playback on active Spotify Connect devices. This project is ideal for DIY music displays, wall clocks with “Now Playing” info, or custom Spotify controllers.

> ⚠️ **Note:** This project uses the Spotify Web API which requires authentication via OAuth2. To use it, you **must register a Spotify app** to get a Client ID and configure redirect URLs. :contentReference[oaicite:1]{index=1}

---

## 🧠 Features

- Connects to Spotify using Wi-Fi and the Spotify Web API
- Displays **currently playing track metadata** (title, artist, album, progress)
- Handles **OAuth2 authentication flow** (access + refresh tokens)
- Optional playback controls (play/pause, skip)
- Easy to customize for displays, controllers, or IoT dashboards

---

## 📦 Prerequisites

Before you begin:

1. **Spotify Developer App**
   - Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/)
   - Create a new application
   - Copy your **Client ID**
   - Set the **Redirect URI** to the address your ESP32 will use for OAuth callbacks

2. **Hardware**
   - ESP32 board (DevKit, WROVER, etc.)
   - Optional: OLED/TFT display for showing information

3. **Software**
   - Arduino IDE or PlatformIO
   - ESP32 board support installed
   - Necessary libraries (WiFi, HTTP client, JSON parser)

---

## 🛠️ Installation & Setup

1. **Clone the repository**

   ```bash
   git clone https://github.com/devanshuk3/ESP32_SPOTIFY_LIVE.git
   cd ESP32_SPOTIFY_LIVE
