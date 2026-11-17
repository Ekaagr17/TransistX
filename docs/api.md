# TransitX — REST API & WebSocket Reference

This document provides the definitive technical specification for all REST endpoints and WebSocket channels provided by the TransitX C++ backend service.

---

## Table of Contents

- [Global Standards & Conventions](#global-standards--conventions)
- [Authentication Endpoints](#authentication-endpoints)
  - [POST /api/auth/register](#post-apiauthregister)
  - [POST /api/auth/login](#post-apiauthlogin)
  - [GET /api/auth/me](#get-apiauthme)
- [Buses Endpoints](#buses-endpoints)
  - [GET /api/buses](#get-apibuses)
  - [GET /api/buses/:id](#get-apibusesid)
  - [GET /api/buses/:id/location](#get-apibusesidlocation)
  - [GET /api/buses/:id/eta/:stopId](#get-apibusesid-etastopid)
- [Routes Endpoints](#routes-endpoints)
  - [GET /api/routes](#get-apiroutes)
  - [GET /api/routes/:id](#get-apiroutesid)
  - [GET /api/routes/:id/stops](#get-apiroutesidstops)
- [Stops Endpoints](#stops-endpoints)
  - [GET /api/stops](#get-apistops)
  - [GET /api/stops/:id](#get-apistopsid)
  - [GET /api/stops/nearby](#get-apistopsnearby)
- [GPS Telemetry Endpoints](#gps-telemetry-endpoints)
  - [POST /api/gps/update](#post-apigpsupdate)
  - [GET /api/gps/status](#get-apigpsstatus)
- [Driver Operations](#driver-operations)
  - [GET /api/driver/dashboard](#get-apidriverdashboard)
  - [POST /api/driver/trips/start](#post-apidrivertripsstart)
  - [POST /api/driver/trips/pause](#post-apidrivertripspause)
  - [POST /api/driver/trips/resume](#post-apidrivertripsresume)
  - [POST /api/driver/trips/end](#post-apidrivertripsend)
- [Favorites Endpoints](#favorites-endpoints)
  - [GET /api/favorites](#get-apifavorites)
  - [POST /api/favorites](#post-apifavorites)
  - [DELETE /api/favorites/:routeId](#delete-apifavoritesrouteid)
- [Service Alerts Endpoints](#service-alerts-endpoints)
  - [GET /api/alerts](#get-apialerts)
  - [POST /api/admin/alerts](#post-apiadminalerts)
- [WebSocket Live Telemetry Channel](#websocket-live-telemetry-channel)
  - [Connection Protocol (/ws/live-buses)](#connection-protocol-wslive-buses)

---

## Global Standards & Conventions

### Base URL
- Local Development: `http://localhost:8080`
- Docker Network: `http://backend:8080`

### Headers
- `Content-Type: application/json` is required for all `POST` / `PUT` requests with JSON payloads.
- Authentication utilizes standard Bearer tokens in the `Authorization` header:
  ```http
  Authorization: Bearer <your-jwt-token>
  ```

### Standard Status Codes
- `200 OK`: Request succeeded.
- `201 Created`: Resource successfully created.
- `400 Bad Request`: Validation failure or malformed JSON payload.
- `401 Unauthorized`: Missing, expired, or invalid JWT token.
- `403 Forbidden`: Authenticated user lacks necessary role permissions.
- `404 Not Found`: Target entity or resource does not exist.
- `500 Internal Server Error`: Server or database failure.

---

## Authentication Endpoints

### POST /api/auth/register
Registers a new passenger user account.

- **Authentication**: None (Public)
- **Request Headers**: `Content-Type: application/json`

#### Request Body
```json
{
  "name": "Pooja Joshi",
  "email": "pooja@example.com",
  "password": "SecurePassword123"
}
```

#### Successful Response (`201 Created`)
```json
{
  "success": true,
  "message": "User registered successfully",
  "user": {
    "id": 9,
    "name": "Pooja Joshi",
    "email": "pooja@example.com",
    "role": "passenger"
  }
}
```

#### Error Responses
- `400 Bad Request`: Missing fields or invalid email syntax.
  ```json
  { "error": "Invalid email address format" }
  ```
- `409 Conflict`: Email already in use.
  ```json
  { "error": "User with this email already exists" }
  ```

---

### POST /api/auth/login
Authenticates user credentials and issues a signed JSON Web Token (HS256).

- **Authentication**: None (Public)
- **Request Headers**: `Content-Type: application/json`

#### Request Body
```json
{
  "email": "pass1@demo.com",
  "password": "password123"
}
```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxLCJlbWFpbCI6InBhc3MxQGRlbW8uY29tIiwicm9sZSI6InBhc3NlbmdlciIsImV4cCI6MTc5NzMwMTQwMH0.signature",
  "user": {
    "id": 1,
    "name": "Amit Sharma",
    "email": "pass1@demo.com",
    "role": "passenger"
  }
}
```

#### Error Responses
- `401 Unauthorized`: Invalid credentials.
  ```json
  { "error": "Invalid email or password" }
  ```

---

### GET /api/auth/me
Retrieves current authenticated user's profile based on the JWT bearer token.

- **Authentication**: Required (`passenger`, `driver`, or `admin`)
- **Request Headers**: `Authorization: Bearer <token>`

#### Successful Response (`200 OK`)
```json
{
  "id": 1,
  "name": "Amit Sharma",
  "email": "pass1@demo.com",
  "role": "passenger",
  "created_at": "2026-11-17T10:00:00Z"
}
```

#### Error Responses
- `401 Unauthorized`: Token missing or invalid.
  ```json
  { "error": "Authorization token required" }
  ```

---

## Buses Endpoints

### GET /api/buses
Lists all registered transit fleet buses with operational statuses and active trips.

- **Authentication**: None (Public)

#### Successful Response (`200 OK`)
```json
[
  {
    "id": 1,
    "bus_number": "UK07PA7001",
    "registration_number": "UK-07-PA-7001",
    "capacity": 40,
    "status": "active",
    "active_trip": {
      "trip_id": 12,
      "route_id": 1,
      "route_name": "Demo: Clock Tower - ISBT",
      "route_code": "DR001"
    }
  },
  {
    "id": 6,
    "bus_number": "UK07PA7006",
    "registration_number": "UK-07-PA-7006",
    "capacity": 52,
    "status": "inactive",
    "active_trip": null
  }
]
```

---

### GET /api/buses/:id
Retrieves detailed metadata for an individual bus.

- **Authentication**: None (Public)
- **Path Parameters**: `id` (integer) — Bus ID

#### Successful Response (`200 OK`)
```json
{
  "id": 1,
  "bus_number": "UK07PA7001",
  "registration_number": "UK-07-PA-7001",
  "capacity": 40,
  "status": "active",
  "created_at": "2026-11-17T09:00:00Z"
}
```

#### Error Responses
- `404 Not Found`:
  ```json
  { "error": "Bus with ID 999 not found" }
  ```

---

### GET /api/buses/:id/location
Fetches the latest real-time GPS coordinate telemetry recorded for a specific bus.

- **Authentication**: None (Public)
- **Path Parameters**: `id` (integer) — Bus ID

#### Successful Response (`200 OK`)
```json
{
  "bus_id": 1,
  "latitude": 30.3175,
  "longitude": 78.0334,
  "speed": 28.5,
  "heading": 175.2,
  "accuracy": 3.8,
  "updated_at": "2026-11-17T10:32:04Z",
  "is_stale": false
}
```

#### Error Responses
- `404 Not Found`: No location telemetry found for the bus.
  ```json
  { "error": "No location records available for bus 6" }
  ```

---

### GET /api/buses/:id/eta/:stopId
Computes real-time dynamic arrival estimation for a bus approaching a specific route stop.

- **Authentication**: None (Public)
- **Path Parameters**:
  - `id` (integer) — Bus ID
  - `stopId` (integer) — Target Stop ID

#### Successful Response (`200 OK`)
```json
{
  "bus_id": 1,
  "stop_id": 3,
  "stop_name": "Gandhi Road",
  "estimated_arrival": "2026-11-17T10:39:15Z",
  "estimated_duration_minutes": 7.2,
  "remaining_distance_km": 2.14,
  "current_speed_kmh": 28.5,
  "confidence": 0.95,
  "last_gps_update": "2026-11-17T10:32:04Z"
}
```

#### Error Responses
- `400 Bad Request`: Target stop is not on the active trip's route or bus has already passed the stop.
  ```json
  { "error": "Stop 3 is not an upcoming stop on the vehicle's active route" }
  ```
- `404 Not Found`: Bus or stop not found.

---

## Routes Endpoints

### GET /api/routes
Returns all defined transit routes.

- **Authentication**: None (Public)

#### Successful Response (`200 OK`)
```json
[
  {
    "id": 1,
    "route_name": "Demo: Clock Tower - ISBT",
    "route_code": "DR001",
    "start_point": "Clock Tower",
    "end_point": "ISBT Dehradun",
    "status": "active"
  },
  {
    "id": 2,
    "route_name": "Demo: Rajpur Road - Mussoorie",
    "route_code": "DR002",
    "start_point": "Rajpur Road",
    "end_point": "Mussoorie",
    "status": "active"
  }
]
```

---

### GET /api/routes/:id
Retrieves detailed information for a single route.

- **Authentication**: None (Public)
- **Path Parameters**: `id` (integer) — Route ID

#### Successful Response (`200 OK`)
```json
{
  "id": 1,
  "route_name": "Demo: Clock Tower - ISBT",
  "route_code": "DR001",
  "start_point": "Clock Tower",
  "end_point": "ISBT Dehradun",
  "status": "active",
  "total_stops": 6
}
```

---

### GET /api/routes/:id/stops
Returns the ordered sequence of transit stops along a specified route corridor.

- **Authentication**: None (Public)
- **Path Parameters**: `id` (integer) — Route ID

#### Successful Response (`200 OK`)
```json
[
  {
    "route_id": 1,
    "stop_id": 1,
    "stop_order": 1,
    "stop_name": "Clock Tower",
    "latitude": 30.3165,
    "longitude": 78.0322,
    "estimated_stop_duration": 60
  },
  {
    "route_id": 1,
    "stop_id": 2,
    "stop_order": 2,
    "stop_name": "Paltan Bazaar",
    "latitude": 30.3185,
    "longitude": 78.0370,
    "estimated_stop_duration": 30
  },
  {
    "route_id": 1,
    "stop_id": 3,
    "stop_order": 3,
    "stop_name": "Gandhi Road",
    "latitude": 30.3148,
    "longitude": 78.0400,
    "estimated_stop_duration": 30
  }
]
```

---

## Stops Endpoints

### GET /api/stops
Lists all transit stops registered in the network.

- **Authentication**: None (Public)

#### Successful Response (`200 OK`)
```json
[
  {
    "id": 1,
    "stop_name": "Clock Tower",
    "latitude": 30.3165,
    "longitude": 78.0322
  },
  {
    "id": 6,
    "stop_name": "ISBT Dehradun",
    "latitude": 30.2724,
    "longitude": 78.0462
  }
]
```

---

### GET /api/stops/:id
Retrieves an individual stop's metadata and lists routes that call at this stop.

- **Authentication**: None (Public)
- **Path Parameters**: `id` (integer) — Stop ID

#### Successful Response (`200 OK`)
```json
{
  "id": 1,
  "stop_name": "Clock Tower",
  "latitude": 30.3165,
  "longitude": 78.0322,
  "routes": [
    { "id": 1, "route_code": "DR001", "route_name": "Demo: Clock Tower - ISBT" }
  ]
}
```

---

### GET /api/stops/nearby
Spatial radial proximity search leveraging PostGIS `ST_DWithin`.

- **Authentication**: None (Public)
- **Query Parameters**:
  - `lat` (float, required) — Commuter's current latitude
  - `lng` (float, required) — Commuter's current longitude
  - `radius` (float, optional, default: `1000`) — Search radius in meters

#### Request Example
```http
GET /api/stops/nearby?lat=30.3170&lng=78.0325&radius=800
```

#### Successful Response (`200 OK`)
```json
[
  {
    "id": 1,
    "stop_name": "Clock Tower",
    "latitude": 30.3165,
    "longitude": 78.0322,
    "distance_meters": 62.4
  },
  {
    "id": 2,
    "stop_name": "Paltan Bazaar",
    "latitude": 30.3185,
    "longitude": 78.0370,
    "distance_meters": 465.1
  }
]
```

---

## GPS Telemetry Endpoints

### POST /api/gps/update
Ingests real-time vehicle telemetry from an active driver device.

- **Authentication**: Required (`driver`)
- **Request Headers**:
  - `Authorization: Bearer <driver-jwt>`
  - `Content-Type: application/json`

#### Request Body
```json
{
  "bus_id": 1,
  "trip_id": 12,
  "latitude": 30.317512,
  "longitude": 78.033481,
  "speed": 28.5,
  "heading": 175.2,
  "accuracy": 3.8
}
```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "status": "processed",
  "bus_id": 1,
  "recorded_at": "2026-11-17T10:32:04Z"
}
```

#### Error Responses
- `400 Bad Request`: Coordinate accuracy out of acceptable range ($> 200\text{m}$) or coordinates invalid.
  ```json
  { "error": "Accuracy error exceeds tolerance limit (200m)" }
  ```
- `403 Forbidden`: Authenticated user is not a driver or is not assigned to this trip.

---

### GET /api/gps/status
Health check inspection endpoint for the telemetry ingestion pipeline.

- **Authentication**: None (Public)

#### Successful Response (`200 OK`)
```json
{
  "pipeline_status": "healthy",
  "active_tracking_buses": 5,
  "total_updates_received_today": 14208,
  "average_ingest_latency_ms": 1.4
}
```

---

## Driver Operations

### GET /api/driver/dashboard
Provides all operational parameters required for the driver's active console.

- **Authentication**: Required (`driver`)
- **Request Headers**: `Authorization: Bearer <token>`

#### Successful Response (`200 OK`)
```json
{
  "driver": {
    "id": 1,
    "name": "Suresh Panwar",
    "license_number": "UK-DL-2020-001234"
  },
  "assigned_bus": {
    "id": 1,
    "bus_number": "UK07PA7001",
    "registration_number": "UK-07-PA-7001"
  },
  "active_trip": {
    "id": 12,
    "route_id": 1,
    "route_name": "Demo: Clock Tower - ISBT",
    "route_code": "DR001",
    "status": "in_progress",
    "started_at": "2026-11-17T10:15:00Z"
  }
}
```

---

### POST /api/driver/trips/start
Transitions a scheduled trip to `in_progress` and begins public tracking.

- **Authentication**: Required (`driver`)
- **Request Body**:
  ```json
  {
    "bus_id": 1,
    "route_id": 1
  }
  ```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "trip_id": 15,
  "status": "in_progress",
  "started_at": "2026-11-17T10:35:00Z"
}
```

---

### POST /api/driver/trips/pause
Temporarily pauses active trip tracking (e.g., breakdown, road blockage).

- **Authentication**: Required (`driver`)
- **Request Body**:
  ```json
  {
    "trip_id": 15
  }
  ```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "trip_id": 15,
  "status": "paused"
}
```

---

### POST /api/driver/trips/resume
Resumes a previously paused trip.

- **Authentication**: Required (`driver`)
- **Request Body**:
  ```json
  {
    "trip_id": 15
  }
  ```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "trip_id": 15,
  "status": "in_progress"
}
```

---

### POST /api/driver/trips/end
Concludes the trip, unlinks active vehicle broadcast, and archives trip logs.

- **Authentication**: Required (`driver`)
- **Request Body**:
  ```json
  {
    "trip_id": 15
  }
  ```

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "trip_id": 15,
  "status": "completed",
  "ended_at": "2026-11-17T11:20:00Z"
}
```

---

## Favorites Endpoints

### GET /api/favorites
Lists all transit routes bookmarked by the currently authenticated passenger.

- **Authentication**: Required (`passenger`, `driver`, `admin`)
- **Request Headers**: `Authorization: Bearer <token>`

#### Successful Response (`200 OK`)
```json
[
  {
    "favorite_id": 1,
    "route_id": 1,
    "route_name": "Demo: Clock Tower - ISBT",
    "route_code": "DR001",
    "created_at": "2026-11-17T09:30:00Z"
  }
]
```

---

### POST /api/favorites
Saves a route to the user's bookmarks list.

- **Authentication**: Required (`passenger`, `driver`, `admin`)
- **Request Body**:
  ```json
  {
    "route_id": 2
  }
  ```

#### Successful Response (`201 Created`)
```json
{
  "success": true,
  "message": "Route added to favorites",
  "favorite_id": 4,
  "route_id": 2
}
```

---

### DELETE /api/favorites/:routeId
Removes a route from the user's favorites list.

- **Authentication**: Required (`passenger`, `driver`, `admin`)
- **Path Parameters**: `routeId` (integer) — ID of route to unfavorite

#### Successful Response (`200 OK`)
```json
{
  "success": true,
  "message": "Route removed from favorites"
}
```

---

## Service Alerts Endpoints

### GET /api/alerts
Retrieves all active transit disruption advisories.

- **Authentication**: None (Public)

#### Successful Response (`200 OK`)
```json
[
  {
    "id": 1,
    "route_id": 2,
    "route_code": "DR002",
    "message": "Traffic diversion near Kolhu Khet due to road maintenance.",
    "severity": "warning",
    "created_at": "2026-11-17T08:00:00Z",
    "expires_at": "2026-11-17T18:00:00Z"
  }
]
```

---

### POST /api/admin/alerts
Publishes a new route advisory or network-wide emergency service bulletin.

- **Authentication**: Required (`admin`)
- **Request Headers**: `Authorization: Bearer <admin-jwt>`
- **Request Body**:
  ```json
  {
    "route_id": 1,
    "message": "Heavy monsoon delays along Gandhi Road corridor.",
    "severity": "warning",
    "expires_in_hours": 6
  }
  ```

#### Successful Response (`201 Created`)
```json
{
  "success": true,
  "alert": {
    "id": 2,
    "route_id": 1,
    "message": "Heavy monsoon delays along Gandhi Road corridor.",
    "severity": "warning",
    "created_at": "2026-11-17T10:35:00Z",
    "expires_at": "2026-11-17T16:35:00Z"
  }
}
```

---

## WebSocket Live Telemetry Channel

### Connection Protocol (/ws/live-buses)
- **URL**: `ws://localhost:8080/ws/live-buses`
- **Protocol**: Standard WebSocket (RFC 6455)

#### Client Commands (Optional Subscription Filtering)
Upon connection, clients receive all broadcasted bus movements by default. Commuters can optionally filter streams by route or bus:

```json
{
  "action": "subscribe",
  "route_id": 1
}
```

To clear filters and receive the global feed:
```json
{
  "action": "subscribe_all"
}
```

#### Server Broadcast Message Format
Dispatched instantly whenever a driver transmits fresh telemetry:

```json
{
  "type": "BUS_LOCATION_UPDATE",
  "bus_id": 1,
  "bus_number": "UK07PA7001",
  "route_id": 1,
  "route_code": "DR001",
  "latitude": 30.317512,
  "longitude": 78.033481,
  "speed": 28.5,
  "heading": 175.2,
  "accuracy": 3.8,
  "updated_at": "2026-11-17T10:32:04Z"
}
```
