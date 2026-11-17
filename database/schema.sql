-- =============================================================================
-- TransitX — PostgreSQL + PostGIS Schema
-- Real-time bus tracking application
-- =============================================================================

-- Enable PostGIS extension
CREATE EXTENSION IF NOT EXISTS postgis;

-- ---------------------------------------------------------------------------
-- TABLES
-- ---------------------------------------------------------------------------

CREATE TABLE users (
    id          SERIAL PRIMARY KEY,
    name        VARCHAR NOT NULL,
    email       VARCHAR UNIQUE NOT NULL,
    password_hash VARCHAR NOT NULL,
    role        TEXT NOT NULL CHECK (role IN ('passenger', 'driver', 'admin')),
    created_at  TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE buses (
    id                  SERIAL PRIMARY KEY,
    bus_number          VARCHAR UNIQUE NOT NULL,
    registration_number VARCHAR,
    capacity            INT,
    status              TEXT DEFAULT 'inactive',
    created_at          TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE drivers (
    id              SERIAL PRIMARY KEY,
    user_id         INT REFERENCES users(id) UNIQUE NOT NULL,
    license_number  VARCHAR NOT NULL
);

CREATE TABLE routes (
    id          SERIAL PRIMARY KEY,
    route_name  VARCHAR NOT NULL,
    route_code  VARCHAR UNIQUE NOT NULL,
    start_point VARCHAR,
    end_point   VARCHAR,
    status      TEXT DEFAULT 'active'
);

CREATE TABLE stops (
    id          SERIAL PRIMARY KEY,
    stop_name   VARCHAR NOT NULL,
    latitude    DOUBLE PRECISION NOT NULL,
    longitude   DOUBLE PRECISION NOT NULL,
    location    GEOGRAPHY(Point, 4326)
);

CREATE TABLE route_stops (
    id                      SERIAL PRIMARY KEY,
    route_id                INT REFERENCES routes(id) ON DELETE CASCADE,
    stop_id                 INT REFERENCES stops(id) ON DELETE CASCADE,
    stop_order              INT NOT NULL,
    estimated_stop_duration INT DEFAULT 30,
    UNIQUE (route_id, stop_order)
);

CREATE TABLE trips (
    id          SERIAL PRIMARY KEY,
    bus_id      INT REFERENCES buses(id),
    route_id    INT REFERENCES routes(id),
    driver_id   INT REFERENCES drivers(id),
    started_at  TIMESTAMPTZ,
    ended_at    TIMESTAMPTZ,
    status      TEXT DEFAULT 'scheduled'
);

CREATE TABLE gps_updates (
    id          BIGSERIAL PRIMARY KEY,
    bus_id      INT REFERENCES buses(id),
    trip_id     INT REFERENCES trips(id),
    latitude    DOUBLE PRECISION NOT NULL,
    longitude   DOUBLE PRECISION NOT NULL,
    speed       DOUBLE PRECISION,
    heading     DOUBLE PRECISION,
    accuracy    DOUBLE PRECISION,
    recorded_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE bus_latest_locations (
    bus_id      INT PRIMARY KEY REFERENCES buses(id),
    latitude    DOUBLE PRECISION NOT NULL,
    longitude   DOUBLE PRECISION NOT NULL,
    speed       DOUBLE PRECISION,
    heading     DOUBLE PRECISION,
    accuracy    DOUBLE PRECISION,
    updated_at  TIMESTAMPTZ DEFAULT NOW(),
    location    GEOGRAPHY(Point, 4326)
);

CREATE TABLE favorites (
    id          SERIAL PRIMARY KEY,
    user_id     INT REFERENCES users(id) ON DELETE CASCADE,
    route_id    INT REFERENCES routes(id) ON DELETE CASCADE,
    created_at  TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE (user_id, route_id)
);

CREATE TABLE service_alerts (
    id          SERIAL PRIMARY KEY,
    route_id    INT REFERENCES routes(id),
    message     TEXT NOT NULL,
    severity    TEXT DEFAULT 'info',
    created_at  TIMESTAMPTZ DEFAULT NOW(),
    expires_at  TIMESTAMPTZ
);

-- ---------------------------------------------------------------------------
-- INDEXES
-- ---------------------------------------------------------------------------

CREATE INDEX idx_gps_updates_bus_recorded   ON gps_updates (bus_id, recorded_at);
CREATE INDEX idx_gps_updates_trip           ON gps_updates (trip_id);
CREATE INDEX idx_trips_bus                  ON trips (bus_id);
CREATE INDEX idx_trips_driver               ON trips (driver_id);
CREATE INDEX idx_trips_status               ON trips (status);
CREATE INDEX idx_bus_latest_locations_geo   ON bus_latest_locations USING GIST (location);
CREATE INDEX idx_stops_geo                  ON stops USING GIST (location);
CREATE INDEX idx_route_stops_route          ON route_stops (route_id);
CREATE INDEX idx_service_alerts_route_exp   ON service_alerts (route_id, expires_at);

-- ---------------------------------------------------------------------------
-- TRIGGERS — auto-compute GEOGRAPHY location from lat/lng
-- ---------------------------------------------------------------------------

-- Stops: set location from latitude / longitude
CREATE OR REPLACE FUNCTION fn_stops_set_location()
RETURNS TRIGGER AS $$
BEGIN
    NEW.location := ST_SetSRID(ST_MakePoint(NEW.longitude, NEW.latitude), 4326)::geography;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_stops_set_location
    BEFORE INSERT OR UPDATE OF latitude, longitude ON stops
    FOR EACH ROW
    EXECUTE FUNCTION fn_stops_set_location();

-- Bus latest locations: set location from latitude / longitude
CREATE OR REPLACE FUNCTION fn_bus_latest_locations_set_location()
RETURNS TRIGGER AS $$
BEGIN
    NEW.location := ST_SetSRID(ST_MakePoint(NEW.longitude, NEW.latitude), 4326)::geography;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_bus_latest_locations_set_location
    BEFORE INSERT OR UPDATE OF latitude, longitude ON bus_latest_locations
    FOR EACH ROW
    EXECUTE FUNCTION fn_bus_latest_locations_set_location();
