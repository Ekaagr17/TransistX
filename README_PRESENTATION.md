# TransitX — Class Presentation Guide

This guide is designed for anyone presenting TransitX on their laptop with zero setup headaches.

---

## 1. What You Need on Your Laptop (Prerequisites)

Only **ONE** thing is strictly required:
* **Node.js (Version 18 or higher)**:
  - Download the recommended LTS installer from [https://nodejs.org/](https://nodejs.org/)
  - Run the installer, keep clicking **"Next"**, and finish.
  - Restart your terminal after installing.

*(Optional but recommended if you want to run the C backend natively: MinGW GCC compiler or Docker Desktop).*

---

## 2. Quickest Way to Run (Under 60 Seconds)

### On Windows:
1. Download or clone this repository to your laptop:
   ```bash
   git clone https://github.com/Ekaagr17/TransistX.git
   ```
2. Open the downloaded folder and **double-click** `start_demo.bat`.
3. It will automatically check dependencies, install them on the first run, and open `http://localhost:5173` in your browser!

### On macOS / Linux (or Manual Terminal on Windows):
1. Open a terminal in the project folder:
   ```bash
   cd frontend
   npm install
   npm run dev
   ```
2. Open [http://localhost:5173](http://localhost:5173) in Chrome, Edge, or Safari.

*(Optional: If you have GCC installed and want the C backend running simultaneously, open a second terminal: `cd backend_c && gcc -std=c11 -O2 src/*.c -Iinclude -o transitx_c -lm && ./transitx_c`).*

---

## 3. Demo Walkthrough for Class Presentation (5-Minute Script)

Follow this sequence for an impressive presentation:

### Slide / Step 1: The Problem & Live Telemetry Map (1 minute)
* Open `http://localhost:5173`.
* **What to say:** *"TransitX is a real-time public bus tracking and journey recommendation platform. The UI is custom-designed with a dark technical transportation palette (#171B20) using MapLibre GL JS and high-resolution Geoapify dark raster tiles."*
* Point out the live buses moving across Dehradun (16 fleet buses on 5 routes).
* Click on any bus to show the **Live Telemetry Panel** (Speed, heading, next stop, ETA).

### Slide / Step 2: The 29-Stop Journey Planner & "Best Pick" Bus (2 minutes)
* Click the prominent emerald button: **"Plan Journey / Best Bus (29 Stops)"**.
* **What to say:** *"Unlike standard transit apps that only show where buses are, TransitX acts as an intelligent transit assistant."*
* Select a **Starting Point** from the 29 Dehradun stops (e.g., *Clock Tower* or *Prince Chowk*).
* Select a **Destination** (e.g., *ISBT Dehradun* or *Pacific Mall*).
* Show the **Boarding Guidance Banner**:
  - Highlights the boarding stop.
  - Displays the pedestrian walking distance (e.g. *320 meters*) and walking duration (~*4 min walk*) powered by the Geoapify Pedestrian Routing API!
* Show the **`[★ RECOMMENDED BEST PICK]`** bus:
  - Explain the algorithm: It calculates which bus arrives at the boarding stop with the shortest wait time and fastest travel duration.
* Click to expand the **Stop-by-Stop Timetable**:
  - Point out that arrival times are dynamically computed against the **exact live wall-clock time** when opening the app (e.g. Stop 1: `17:48`, Stop 2: `17:53`, Stop 3: `17:59`...).

### Slide / Step 3: Add to Favorites (30 seconds)
* Click the **Star icon** on the best bus to bookmark it.
* Go to the **Favourites** tab in the sidebar and show that the bus is saved with its boarding stop and walking distance.

### Slide / Step 4: System Architecture & Backend (1 minute)
* Mention that the backend is built in **Pure C (C11 standard)** using sockets and `cJSON`, capable of streaming telemetry via WebSockets and processing Haversine geodesic math in sub-millisecond time.
* Mention the relational **PostgreSQL + PostGIS** schema with spatial GiST indexing.

### Slide / Step 5: Mobile Driver Interface (Optional Bonus)
* In a new tab, open `http://localhost:5174` (or `npm run dev` inside `driver-app`).
* Show the mobile-first driver GPS interface that collects `watchPosition()` coordinates, checks accuracy (<100m), and transmits updates to the backend.

---

## 4. Troubleshooting Checklist Before the Presentation

| Issue | Quick Fix |
|---|---|
| `npm: command not found` | Install Node.js from [nodejs.org](https://nodejs.org/) and restart terminal. |
| Port 5173 is already in use | Vite will automatically switch to 5174 or 5175. |
| Map tiles don't load | Check internet connection (Geoapify and CartoDB tiles stream online). |
| Offline mode | The frontend has a built-in telemetry simulation engine so the demo works even if the backend server isn't running. |
