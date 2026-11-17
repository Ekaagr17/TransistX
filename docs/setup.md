# TransitX — Installation & Operations Setup Guide

This guide provides step-by-step instructions for installing, configuring, compiling, and running TransitX across development workstations and production environments.

---

## Table of Contents

- [1. System Prerequisites](#1-system-prerequisites)
  - [1.1 Package Requirements by Operating System](#11-package-requirements-by-operating-system)
- [2. Quick Setup via Docker Compose (Recommended)](#2-quick-setup-via-docker-compose-recommended)
  - [2.1 Configuration Steps](#21-configuration-steps)
  - [2.2 Verifying Container Health](#22-verifying-container-health)
- [3. Manual Native Setup](#3-manual-native-setup)
  - [3.1 Database Setup (PostgreSQL 16 + PostGIS)](#31-database-setup-postgresql-16--postgis)
  - [3.2 Schema Migrations & Seed Data](#32-schema-migrations--seed-data)
  - [3.3 Backend Compilation & Execution (C++17)](#33-backend-compilation--execution-c17)
  - [3.4 Passenger Frontend Setup (React + Vite)](#34-passenger-frontend-setup-react--vite)
  - [3.5 Driver Mobile App Setup (React + Vite)](#35-driver-mobile-app-setup-react--vite)
- [4. Environment Configuration Reference](#4-environment-configuration-reference)
- [5. Troubleshooting Common Issues](#5-troubleshooting-common-issues)
  - [5.1 PostGIS Extension Fails to Install](#51-postgis-extension-fails-to-install)
  - [5.2 C++ Compilation / Linker Errors (libpqxx, OpenSSL)](#52-c-compilation--linker-errors-libpqxx-openssl)
  - [5.3 CORS Policy Errors](#53-cors-policy-errors)
  - [5.4 Geolocation Permission Denied / Mobile Insecure Origin](#54-geolocation-permission-denied--mobile-insecure-origin)
  - [5.5 WebSocket Connection Failures](#55-websocket-connection-failures)

---

## 1. System Prerequisites

TransitX can be executed either via containerized services using **Docker & Docker Compose** or as standalone native processes on your host system.

### 1.1 Package Requirements by Operating System

#### Linux (Ubuntu 22.04 LTS / Debian 12)
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libpq-dev \
    libpqxx-dev \
    libssl-dev \
    postgresql \
    postgresql-contrib \
    postgis \
    postgresql-16-postgis-3 \
    nodejs \
    npm
```

#### macOS (Homebrew)
```bash
brew update
brew install \
    cmake \
    libpqxx \
    openssl@3 \
    postgresql@16 \
    postgis \
    node@20
```

#### Windows 10/11
- **Option 1 (Recommended)**: Use **Docker Desktop** with WSL2 backend.
- **Option 2 (Native MinGW-w64)**:
  - Install **Git for Windows** and **MinGW-w64** (GCC 9+ with C++17 support).
  - Install **PostgreSQL 16** with the **PostGIS** bundle using the EnterpriseDB Windows installer.
  - Install **Node.js LTS (18+ or 20+)** from [nodejs.org](https://nodejs.org).

---

## 2. Quick Setup via Docker Compose (Recommended)

Docker Compose orchestrates the four micro-components in isolated networks:
1. `postgres` (PostGIS 16-3.4 image with pre-mounted migrations)
2. `backend` (C++17 Crow multi-threaded server)
3. `frontend` (Passenger portal)
4. `driver-app` (Operator telemetry console)

### 2.1 Configuration Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-org/transitx.git
   cd transitx
   ```

2. **Establish your environment file**:
   ```bash
   cp .env.example .env
   ```

   *Verify that `.env` contains valid credentials. Default local dev values work immediately.*

3. **Build and launch the stack**:
   ```bash
   docker compose up --build -d
   ```

4. **Verify container runtime statuses**:
   ```bash
   docker compose ps
   ```

   All services should display `Up` (with `postgres` showing `healthy`).

### 2.2 Verifying Container Health

Inspect service logs to ensure clean initialization:

```bash
# Check database initialization & seed execution
docker compose logs postgres

# Monitor C++ backend Crow startup & PostgreSQL connection
docker compose logs backend

# Inspect Vite web servers
docker compose logs frontend
docker compose logs driver-app
```

The stack is ready:
- Passenger App: `http://localhost:5173`
- Driver App: `http://localhost:5174`
- Backend API: `http://localhost:8080`

To tear down the containers:
```bash
docker compose down
# Or to wipe database volumes as well:
docker compose down -v
```

---

## 3. Manual Native Setup

For local debugging or native platform compilation without Docker containers:

### 3.1 Database Setup (PostgreSQL 16 + PostGIS)

1. Start your local PostgreSQL service:
   ```bash
   # Linux
   sudo systemctl start postgresql

   # macOS
   brew services start postgresql@16
   ```

2. Provision the database and user:
   ```bash
   # Connect as postgres superuser
   psql -U postgres -h localhost
   ```

   In the `psql` console, execute:
   ```sql
   CREATE DATABASE transitx_db;
   CREATE USER transitx WITH ENCRYPTED PASSWORD 'change_me_in_production';
   GRANT ALL PRIVILEGES ON DATABASE transitx_db TO transitx;
   \c transitx_db
   CREATE EXTENSION IF NOT EXISTS postgis;
   GRANT ALL ON SCHEMA public TO transitx;
   \q
   ```

### 3.2 Schema Migrations & Seed Data

Apply the schema DDL, automated triggers, spatial indexes, and Dehradun demo routes:

```bash
# 1. Apply Schema
psql -U transitx -d transitx_db -h localhost -f database/schema.sql

# 2. Apply Demo Seeds
psql -U transitx -d transitx_db -h localhost -f database/seeds/seed_data.sql
```

To confirm successful seeding, verify the stop count:
```bash
psql -U transitx -d transitx_db -h localhost -c "SELECT COUNT(*) FROM stops;"
# Output should return 29 stops.
```

### 3.3 Backend Compilation & Execution (C++17)

The C++ backend can be built with **CMake** or with the provided **Makefile**.

#### Option A: Building with CMake
```bash
cd backend
mkdir -p build && cd build

# Configure build with C++17
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compile executable
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Run backend executable
./transitx
```

#### Option B: Building with Make (GCC / MinGW)
```bash
cd backend
make clean
make -j4
./transitx
```

When started, the backend outputs:
```
[TransitX] Loaded configuration from environment
[TransitX] PostgreSQL Connection Pool established (host: localhost, db: transitx_db)
[TransitX] Crow HTTP & WebSocket server listening on port 8080
```

### 3.4 Passenger Frontend Setup (React + Vite)

1. Navigate to the frontend directory:
   ```bash
   cd frontend
   ```

2. Install Node packages:
   ```bash
   npm install
   ```

3. Launch the development server:
   ```bash
   npm run dev
   ```

The application will be live at `http://localhost:5173`.

### 3.5 Driver Mobile App Setup (React + Vite)

1. Navigate to the driver application directory:
   ```bash
   cd driver-app
   ```

2. Install Node packages:
   ```bash
   npm install
   ```

3. Run the development server with host binding enabled so mobile devices on the same Wi-Fi network can connect:
   ```bash
   npm run dev -- --host
   ```

Vite will display network addresses (e.g., `http://192.168.1.50:5174`). Open this URL on your mobile phone to test live geolocation streaming.

---

## 4. Environment Configuration Reference

TransitX reads configuration from environment variables defined in `.env`:

| Key | Default | Description |
| :--- | :--- | :--- |
| `DB_HOST` | `postgres` (or `localhost`) | PostgreSQL hostname or IP address |
| `DB_PORT` | `5432` | PostgreSQL port |
| `DB_NAME` | `transitx_db` | PostgreSQL database catalog |
| `DB_USER` | `transitx` | Database user account |
| `DB_PASSWORD` | `change_me_in_production` | Database password |
| `JWT_SECRET` | `change_me_to_a_random_secret`| 256-bit secret key used to sign HS256 tokens |
| `BACKEND_PORT` | `8080` | Port for C++ HTTP & WebSocket server |
| `CORS_ORIGIN` | `http://localhost:5173` | Allowed CORS origin for browser requests |
| `FRONTEND_URL` | `http://localhost:5173` | Public URL for passenger web application |
| `DRIVER_APP_URL`| `http://localhost:5174` | Public URL for driver telemetry application |

---

## 5. Troubleshooting Common Issues

### 5.1 PostGIS Extension Fails to Install

**Symptom**: `ERROR: could not open extension control file ".../postgis.control": No such file or directory`  
**Cause**: The core PostgreSQL server is installed, but the PostGIS spatial binary packages are missing.  
**Resolution**:
- On Ubuntu/Debian:
  ```bash
  sudo apt-get install -y postgresql-16-postgis-3 postgis
  sudo systemctl restart postgresql
  ```
- On macOS:
  ```bash
  brew install postgis
  ```

### 5.2 C++ Compilation / Linker Errors (libpqxx, OpenSSL)

**Symptom**: `fatal error: pqxx/pqxx: No such file or directory` or `undefined reference to pqxx::connection::...`  
**Cause**: Missing headers or libpqxx link library.  
**Resolution**:
- Ensure `pkg-config` can locate `libpqxx`:
  ```bash
  pkg-config --cflags --libs libpqxx
  ```
- If building with CMake, specify the path to `libpqxx` manually if installed in non-standard locations:
  ```bash
  cmake -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql ..
  ```

### 5.3 CORS Policy Errors

**Symptom**: Browser console indicates `Access to fetch at 'http://localhost:8080/api/...' from origin 'http://localhost:5173' has been blocked by CORS policy`.  
**Cause**: Mismatch between the frontend host URL and the backend `CORS_ORIGIN` environment variable.  
**Resolution**:
- Update `CORS_ORIGIN` in your `.env` to include the client origin:
  ```env
  CORS_ORIGIN=http://localhost:5173
  ```
- If testing from multiple devices or a mobile IP:
  ```env
  CORS_ORIGIN=*
  ```

### 5.4 Geolocation Permission Denied / Mobile Insecure Origin

**Symptom**: Driver App displays `Geolocation error: Only secure origins are allowed (HTTPS or localhost)`.  
**Cause**: Modern mobile browsers (iOS Safari, Android Chrome) disable `navigator.geolocation` on non-secure HTTP origins (e.g. `http://192.168.1.x:5174`).  
**Resolution**:
- **Option 1 (Local Testing via Port Forwarding)**: Connect your mobile device via USB and use Chrome DevTools Remote Debugging (`chrome://inspect`) with port forwarding for `localhost:5174`.
- **Option 2 (Vite HTTPS Plugin)**: Install `@vitejs/plugin-basic-ssl` in `driver-app/vite.config.ts` to generate a local self-signed certificate for testing over `https://`.
- **Option 3 (Browser Geolocation Emulation)**: On desktop Chrome/Firefox, open Developer Tools → More Tools → Sensors → Geolocation, and select "Custom location" to simulate coordinates.

### 5.5 WebSocket Connection Failures

**Symptom**: Live map displays `WebSocket disconnected. Reconnecting in 3s...`.  
**Cause**: Backend Crow WebSocket port is unreachable, blocked by a firewall, or the proxy failed to forward the `Upgrade: websocket` HTTP header.  
**Resolution**:
- If using Nginx or an API reverse proxy, ensure WebSocket upgrade headers are passed:
  ```nginx
  proxy_http_version 1.1;
  proxy_set_header Upgrade $http_upgrade;
  proxy_set_header Connection "upgrade";
  ```
- Test direct connection with `wscat`:
  ```bash
  npx wscat -c ws://localhost:8080/ws/live-buses
  ```
