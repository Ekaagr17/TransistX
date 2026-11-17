# TransitX

> **Know Where Your Bus Is. Know When It Arrives.**

TransitX is an enterprise-grade, high-performance real-time public transit tracking system engineered for modern urban transit networks. By integrating low-latency C++17 microservices, spatial querying via PostgreSQL and PostGIS, and modern interactive React web interfaces powered by MapLibre GL JS and WebSockets, TransitX bridges the communication gap between transit authorities, bus operators, and daily commuters.

> [!NOTE]
> **Demonstration Data Notice**: The seed data configured out-of-the-box reflects real geographic coordinates and landmarks located in **Dehradun, Uttarakhand, India** (e.g., Clock Tower, ISBT Dehradun, Rajpur Road, Mussoorie, Clement Town). These routes (DR001–DR005) and timetables are curated strictly for software demonstration and testing purposes and do not represent official city transit schedules.

---

## Table of Contents

- [Features](#features)
- [System Architecture](#system-architecture)
- [Technology Stack](#technology-stack)
- [Prerequisites](#prerequisites)
- [Quick Start with Docker Compose](#quick-start-with-docker-compose)
- [Manual / Local Setup](#manual--local-setup)
  - [1. Database Setup (PostgreSQL + PostGIS)](#1-database-setup-postgresql--postgis)
  - [2. Backend Build & Execution (C++17)](#2-backend-build--execution-c17)
  - [3. Passenger Web App (React + Vite)](#3-passenger-web-app-react--vite)
  - [4. Driver Mobile Web App (React + Vite)](#4-driver-mobile-web-app-react--vite)
- [Project Structure](#project-structure)
- [API Overview](#api-overview)
- [Environment Variables](#environment-variables)
- [Demo Credentials](#demo-credentials)
- [Screenshots & UI Showcase](#screenshots--ui-showcase)
- [Documentation Directory](#documentation-directory)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- 🛰️ **Real-Time GPS Tracking**: Driver devices capture high-frequency geolocation telemetry via HTML5 Geolocation API with accuracy filtering and transmit data directly to the ingest engine.
- ⚡ **Sub-Second Live Updates (WebSockets)**: WebSocket pub/sub channels (`/ws/live-buses`) broadcast live bus movements, speeds, and bearings across active clients with minimal network overhead.
- 🗺️ **Vector Map Visualization**: Smooth, hardware-accelerated mapping powered by MapLibre GL JS, featuring custom transit markers, directional vehicle arrows, live stop clustering, and dynamic polyline route rendering.
- ⏱️ **Intelligent ETA Prediction**: Dynamic ETA pipeline incorporating spatial route projection, Haversine segmented distances, historical average speeds, scheduled dwelling times, and GPS freshness decay.
- 📱 **Dedicated Mobile Driver App**: Touch-optimized interface for transit drivers providing single-tap trip control (Start, Pause, Resume, End), real-time broadcast status, accuracy telemetry, and GPS quality alerts.
- 🛡️ **Role-Based Access Control & Admin Suite**: Secure JWT authentication guarding administrative routes, emergency service alert broadcasting, fleet status monitoring, and driver assignments.
- ⭐ **Commuter Favorites & Alerts**: Commuters can bookmark frequent routes and receive instant notifications regarding traffic delays, route diversions, or maintenance alerts.

---

## System Architecture

TransitX employs an event-driven decoupled architecture optimized for real-time spatial workloads:

```
                  ┌──────────────────────────────────────────────┐
                  │           React Driver Mobile Web App        │
                  │   (navigator.geolocation.watchPosition)      │
                  └──────────────────────┬───────────────────────┘
                                         │ HTTPS / POST telemetry
                                         ▼
┌────────────────────────────────────────────────────────────────────────┐
│                   TransitX C++17 Backend Engine                         │
│  ┌───────────────────────┐                  ┌───────────────────────┐  │
│  │   Crow HTTP Server    │                  │  WebSocket Hub /ws    │  │
│  │ (REST Controllers)    │                  │ (Live Bus Broadcast)  │  │
│  └───────────┬───────────┘                  └───────────▲───────────┘  │
│              │                                          │              │
│              ▼                                          │              │
│  ┌──────────────────────────────────────────────────────┴───────────┐  │
│  │     Business Services & Algorithms                               │  │
│  │     • Route Matcher (Orthogonal Snapping)                        │  │
│  │     • ETA Engine (Haversine + Dwell + Traffic Velocity)          │  │
│  │     • Auth & JWT Verification Middleware                         │  │
│  └──────────────────────────────────┬───────────────────────────────┘  │
└─────────────────────────────────────┼──────────────────────────────────┘
                                      │ libpqxx (Connection Pool)
                                      ▼
             ┌─────────────────────────────────────────────────┐
             │            PostgreSQL 16 + PostGIS              │
             │   • Spatial Indexing (GiST on GEOGRAPHY)        │
             │   • Auto-computing triggers (Lat/Lng -> Point)  │
             │   • Historical telemetry logs & fleet state     │
             └─────────────────────────────────────────────────┘
                                      ▲
                                      │ WebSocket / REST API
                                      │
                  ┌───────────────────┴──────────────────────────┐
                  │         React Passenger Web App               │
                  │  (MapLibre GL JS, Route Explorer, Live ETA)  │
                  └──────────────────────────────────────────────┘
```

For complete technical specifications, see [docs/architecture.md](file:///d:/TransitX/docs/architecture.md).

---

## Technology Stack

| Layer | Technologies | Description |
| :--- | :--- | :--- |
| **Backend Core** | C++17, Crow Framework, nlohmann/json | High-throughput, low-latency microservice architecture |
| **Database & GIS** | PostgreSQL 16, PostGIS 3.4, libpqxx | Relational data persistence and spatial indexing (SRID 4326) |
| **Authentication** | JSON Web Tokens (HS256), bcrypt | Secure stateless authorization with role-based claims |
| **Real-time Comms** | WebSockets (RFC 6455) | Full-duplex telemetry streaming to passenger dashboards |
| **Passenger App** | React 18, TypeScript, Vite, MapLibre GL JS | Modern single-page web app with responsive responsive layout |
| **Driver App** | React 18, TypeScript, Vite, Geolocation API | Touch-friendly PWA/mobile web console with offline detection |
| **DevOps & Containers**| Docker, Docker Compose, Alpine Linux | Containerized multi-service deployment and volume orchestration |

---

## Prerequisites

Before setting up TransitX locally, ensure the following utilities are installed:

### Containerized Environment (Recommended)
- **Docker Engine**: Version 24.0 or higher
- **Docker Compose**: Version 2.20 or higher

### Native / Development Environment
- **Operating System**: Linux (Ubuntu 22.04+ recommended), macOS, or Windows 10/11 with MinGW-w64 or WSL2
- **C++ Compiler**: GCC 9+ or Clang 11+ with C++17 support
- **Build Tools**: CMake 3.16+ and GNU Make
- **Database**: PostgreSQL 14+ with PostGIS 3.0+ extension
- **Libraries**: `libpqxx-dev`, `libpq-dev`, `libssl-dev`, `libcrypto`
- **Node.js**: Version 18.x or 20.x LTS with `npm` (v9+)

---

## Quick Start with Docker Compose

The fastest way to deploy the entire TransitX stack (database, C++ backend, passenger frontend, and driver app) is using Docker Compose:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-org/transitx.git
   cd transitx
   ```

2. **Initialize configuration**:
   ```bash
   cp .env.example .env
   ```

3. **Start all services**:
   ```bash
   docker compose up --build
   ```

4. **Access the applications**:
   - **Passenger Web App**: [http://localhost:5173](http://localhost:5173)
   - **Driver Mobile App**: [http://localhost:5174](http://localhost:5174)
   - **Backend API & WebSocket**: [http://localhost:8080](http://localhost:8080)
   - **PostgreSQL Database**: `localhost:5432`

Database migrations and initial Dehradun seed data are automatically applied on the first database container initialization via `/docker-entrypoint-initdb.d`.

---

## Manual / Local Setup

If you prefer building and running services individually on your host workstation:

### 1. Database Setup (PostgreSQL + PostGIS)

Ensure PostgreSQL is running and PostGIS is installed.

```bash
# Connect to PostgreSQL as superuser
psql -U postgres -h localhost

# In the PostgreSQL console:
CREATE DATABASE transitx_db;
CREATE USER transitx WITH ENCRYPTED PASSWORD 'change_me_in_production';
GRANT ALL PRIVILEGES ON DATABASE transitx_db TO transitx;
\c transitx_db
CREATE EXTENSION IF NOT EXISTS postgis;
GRANT ALL ON SCHEMA public TO transitx;
\q

# Apply schema migrations and seeds
psql -U transitx -d transitx_db -h localhost -f database/schema.sql
psql -U transitx -d transitx_db -h localhost -f database/seeds/seed_data.sql
```

### 2. Backend Build & Execution (C++17)

```bash
cd backend

# Option A: CMake (Standard across Linux / macOS / Windows)
mkdir build && cd build
cmake ..
cmake --build . --config Release
./transitx

# Option B: Make (MinGW / GCC)
make
./transitx
```

The backend server listens on `http://0.0.0.0:8080`.

### 3. Passenger Web App (React + Vite)

```bash
cd frontend

# Install dependencies
npm install

# Run the development server
npm run dev
```

The passenger portal is accessible at `http://localhost:5173`.

### 4. Driver Mobile Web App (React + Vite)

```bash
cd driver-app

# Install dependencies
npm install

# Run the driver development server
npm run dev
```

The driver portal runs at `http://localhost:5174`. Open this URL on your mobile browser or use developer responsive device emulation.

---

## Project Structure

```
TransitX/
├── .env.example                # Sample environment configuration template
├── .gitignore                  # Git ignore rules for builds, nodes, and keys
├── docker-compose.yml          # Multi-container orchestration specification
├── README.md                   # Project overview and quickstart documentation
│
├── database/                   # Database schemas, migrations, and seed datasets
│   ├── schema.sql              # Master DDL schema with PostGIS triggers & indexes
│   ├── migrations/             # Timestamped/versioned migration scripts
│   │   └── 001_initial_schema.sql
│   └── seeds/                  # Seed dataset for Dehradun demo routes
│       └── seed_data.sql
│
├── backend/                    # C++17 Crow REST & WebSocket backend service
│   ├── CMakeLists.txt          # Modern CMake build manifest
│   ├── Makefile                # GNU Makefile for alternate toolchains
│   ├── third_party/            # Embedded header-only libraries (json, jwt, crow)
│   ├── include/transitx/       # Header definitions
│   │   ├── algorithms/         # Spatial math: Haversine, ETA, RouteMatcher
│   │   ├── controllers/        # REST route endpoint handlers
│   │   ├── database/           # libpqxx connection manager and pool
│   │   ├── middleware/         # JWT verification, CORS, Role-based auth
│   │   ├── models/             # Domain entities and JSON serialization
│   │   ├── repositories/       # Parameterized SQL data access layer
│   │   ├── services/           # Core transit business domain logic
│   │   ├── utils/              # Configuration loaders and logging
│   │   └── websocket/          # Live bus WebSocket session manager
│   ├── src/                    # Source implementations (.cpp)
│   └── tests/                  # Unit and integration test suites
│
├── frontend/                   # Commuter portal SPA (React 18 + TS + Vite)
│   ├── index.html              # HTML shell
│   ├── package.json            # Node dependencies and scripts
│   ├── vite.config.ts          # Vite build and proxy configuration
│   └── src/
│       ├── components/         # Reusable UI widgets, badges, loaders
│       ├── features/           # Domain feature modules (search, filters)
│       ├── hooks/              # Custom React hooks (useWebSocket, useBuses)
│       ├── layouts/            # Shell layouts, navbar, sidebar
│       ├── maps/               # MapLibre GL JS integration and layer renderers
│       ├── pages/              # Views (Dashboard, LiveMap, Routes, Admin)
│       ├── services/           # Axios API client integrations
│       └── types/              # TypeScript transit domain definitions
│
├── driver-app/                 # Driver telemetry web app (React 18 + TS + Vite)
│   ├── index.html              # Mobile-first viewport wrapper
│   ├── package.json            # Node dependencies
│   ├── vite.config.ts          # Vite dev configuration
│   └── src/
│       ├── components/         # Touch controls, status chips, accuracy radar
│       ├── hooks/              # useGeolocation (navigator.geolocation wrapper)
│       ├── pages/              # Driver login, dashboard, active trip controls
│       └── services/           # Telemetry ingest API service
│
└── docs/                       # Comprehensive architectural & operational manuals
    ├── architecture.md         # Deep-dive system architecture & data flows
    ├── database.md             # Entity-Relationship diagram, schemas, & spatial SQL
    ├── api.md                  # Complete REST API & WebSocket documentation
    └── setup.md                # Comprehensive installation and troubleshooting guide
```

---

## API Overview

The TransitX C++ backend serves REST endpoints under `/api` and full-duplex WebSocket connections under `/ws`:

| Category | Method | Endpoint | Description | Auth Required |
| :--- | :---: | :--- | :--- | :---: |
| **Authentication** | `POST` | `/api/auth/register` | Register a new user account | No |
| | `POST` | `/api/auth/login` | Authenticate credentials and retrieve JWT | No |
| | `GET` | `/api/auth/me` | Retrieve profile of the currently logged-in user | Yes (JWT) |
| **Buses** | `GET` | `/api/buses` | List all buses with current status & assignment | No |
| | `GET` | `/api/buses/:id` | Get metadata for a specific bus | No |
| | `GET` | `/api/buses/:id/location` | Get the latest GPS telemetry for a bus | No |
| | `GET` | `/api/buses/:id/eta/:stopId`| Compute real-time ETA for a bus arriving at a stop | No |
| **Routes** | `GET` | `/api/routes` | List all transit routes | No |
| | `GET` | `/api/routes/:id` | Get details and geometry for a route | No |
| | `GET` | `/api/routes/:id/stops` | Get ordered sequence of stops along a route | No |
| **Stops** | `GET` | `/api/stops` | Retrieve list of all transit stops | No |
| | `GET` | `/api/stops/:id` | Get stop details and passing routes | No |
| | `GET` | `/api/stops/nearby` | Spatial query for stops within radius `r` of `(lat,lng)`| No |
| **GPS Telemetry** | `POST` | `/api/gps/update` | Ingest real-time GPS coordinate from driver | Yes (Driver) |
| | `GET` | `/api/gps/status` | Ingestion pipeline health and packet metrics | No |
| **Driver Operations**| `GET` | `/api/driver/dashboard` | Active trip status, assigned route, and bus stats | Yes (Driver) |
| | `POST` | `/api/driver/trips/start` | Start scheduled trip and begin telemetry broadcast | Yes (Driver) |
| | `POST` | `/api/driver/trips/pause` | Temporarily pause tracking (e.g. traffic hold) | Yes (Driver) |
| | `POST` | `/api/driver/trips/resume`| Resume telemetry broadcast | Yes (Driver) |
| | `POST` | `/api/driver/trips/end` | Terminate trip, finalize distance, reset bus state | Yes (Driver) |
| **Favorites** | `GET` | `/api/favorites` | List user's bookmarked routes | Yes (User) |
| | `POST` | `/api/favorites` | Bookmark a route for quick access | Yes (User) |
| | `DELETE`| `/api/favorites/:routeId`| Remove a route from bookmarks | Yes (User) |
| **Alerts** | `GET` | `/api/alerts` | Get active transit disruption alerts | No |
| | `POST` | `/api/admin/alerts` | Publish urgent route/service disruption alert | Yes (Admin) |
| **WebSockets** | `WS` | `/ws/live-buses` | Stream live bus location updates and ETA deltas | No |

For full request/response payloads, headers, query parameters, and error status codes, consult [docs/api.md](file:///d:/TransitX/docs/api.md).

---

## Environment Variables

| Variable | Description | Default (Docker) | Production Example |
| :--- | :--- | :--- | :--- |
| `DB_HOST` | Hostname or IP of PostgreSQL database | `postgres` | `db.prod.transitx.internal` |
| `DB_PORT` | PostgreSQL listening port | `5432` | `5432` |
| `DB_NAME` | Database catalog name | `transitx_db` | `transitx_production` |
| `DB_USER` | PostgreSQL username | `transitx` | `transitx_svc` |
| `DB_PASSWORD` | PostgreSQL database user password | `change_me_in_production` | `s3cur3-P@ssw0rd-G3n!#` |
| `JWT_SECRET` | Secret key used for signing HS256 tokens | `change_me_to_a_random_secret` | `k8f2a1b9c7d4e5f6...` |
| `BACKEND_PORT` | Port for C++ Crow HTTP & WS server | `8080` | `8080` |
| `CORS_ORIGIN` | Allowed Origin for Cross-Origin requests | `http://localhost:5173` | `https://transitx.example.com` |
| `FRONTEND_URL` | Base URL of Passenger Frontend | `http://localhost:5173` | `https://transitx.example.com` |
| `DRIVER_APP_URL`| Base URL of Driver Mobile Application | `http://localhost:5174` | `https://driver.transitx.example.com` |

---

## Demo Credentials

The database seeds include pre-populated user accounts across all system roles. All accounts are initialized with the default password: `password123`.

| Role | Email Address | Password | Intended Portal / Use Case |
| :--- | :--- | :--- | :--- |
| **System Administrator** | `admin@transitx.com` | `password123` | Passenger App (`/admin`) — Publish alerts, audit fleet |
| **Bus Driver 1** | `driver1@demo.com` | `password123` | Driver App (`/driver/login`) — Assigned to Bus `UK07PA7001` |
| **Bus Driver 2** | `driver2@demo.com` | `password123` | Driver App (`/driver/login`) — Assigned to Bus `UK07PA7002` |
| **Passenger 1** | `pass1@demo.com` | `password123` | Passenger App (`/login`) — Commuter with saved favorites |
| **Passenger 2** | `pass2@demo.com` | `password123` | Passenger App (`/login`) — Commuter with saved favorites |

---

## Screenshots & UI Showcase

*Illustrative mockups of TransitX interfaces:*

- **Passenger Live Radar Map**:
  ```
  +----------------------------------------------------------------------------+
  | TransitX  [Search routes or stops...]              [Favorites] [Alerts (2)]|
  +----------------------------------------------------------------------------+
  |  [Active Route: DR001 - Clock Tower to ISBT]                               |
  |                                                                            |
  |     ( Stop 1: Clock Tower )                                                |
  |             │                                                              |
  |             │   🚍 UK07PA7001 [28 km/h • Heading South]                   |
  |             ▼                                                              |
  |     ( Stop 2: Paltan Bazaar ) ── ETA: 3 mins                               |
  |             │                                                              |
  |             ▼                                                              |
  |     ( Stop 3: Gandhi Road )   ── ETA: 9 mins                               |
  +----------------------------------------------------------------------------+
  | [Live Stops Nearby] [Route Schedules] [Service Advisories]                 |
  +----------------------------------------------------------------------------+
  ```

- **Driver Mobile Console**:
  ```
  +-----------------------------------+
  | 🚌 TransitX Driver Console   [● LIVE]|
  +-----------------------------------+
  | Driver: Suresh Panwar             |
  | Bus: UK07PA7001 (Route: DR001)    |
  +-----------------------------------+
  | GPS Signal: HIGH PRECISION (±4m)  |
  | Speed: 32 km/h  | Heading: 182° S  |
  +-----------------------------------+
  | Next Stop: Gandhi Road            |
  | Approaching in: 4 mins (1.2 km)   |
  +-----------------------------------+
  |  [ PAUSE TRIP ]    [ END TRIP ]   |
  +-----------------------------------+
  ```

---

## Documentation Directory

Explore the dedicated guides in the [`/docs`](file:///d:/TransitX/docs) directory for exhaustive details:

- 🏗️ **[System Architecture](file:///d:/TransitX/docs/architecture.md)**: Deep dive into the event-driven subsystem, Crow C++ concurrency, spatial matching algorithms, and ETA calculation logic.
- 🗄️ **[Database Architecture](file:///d:/TransitX/docs/database.md)**: Full PostGIS schema definitions, entity-relationship diagrams, GIS spatial functions, index strategies, and seed data.
- 📡 **[API Reference](file:///d:/TransitX/docs/api.md)**: Complete REST API documentation with sample request payloads, query arguments, status codes, and WebSocket subscription protocol.
- 🛠️ **[Installation & Setup Guide](file:///d:/TransitX/docs/setup.md)**: Step-by-step setup walkthrough for Docker, Ubuntu, macOS, and Windows environments with troubleshooting advice.

---

## Contributing

We welcome contributions from transit enthusiasts and software engineers alike:

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/dynamic-dwell-time`).
3. Commit your changes (`git commit -m 'feat: introduce dynamic traffic dwell calculations'`).
4. Ensure all C++ builds compile cleanly and frontends pass type checks (`npm run build`).
5. Push to the branch (`git push origin feature/dynamic-dwell-time`).
6. Open a Pull Request detailing your changes, test results, and rationale.

---

## License

TransitX is licensed under the terms of the [MIT License](LICENSE).  
Copyright © 2026 TransitX Contributors.
