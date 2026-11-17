-- =============================================================================
-- TransitX — Seed Data
-- Demonstration data for Dehradun, Uttarakhand, India
-- =============================================================================
-- NOTE: These are demonstration routes and schedules for development/testing.
-- They do NOT represent official Dehradun city bus routes or timetables.
-- =============================================================================

-- ---------------------------------------------------------------------------
-- USERS
-- All passwords: 'password123' (bcrypt hash below)
-- ---------------------------------------------------------------------------

INSERT INTO users (name, email, password_hash, role) VALUES
    ('Amit Sharma',   'pass1@demo.com',      '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'passenger'),
    ('Priya Rawat',   'pass2@demo.com',      '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'passenger'),
    ('Rahul Negi',    'pass3@demo.com',      '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'passenger'),
    ('Suresh Panwar', 'driver1@demo.com',    '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'driver'),
    ('Manoj Bisht',   'driver2@demo.com',    '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'driver'),
    ('Deepak Thapa',  'driver3@demo.com',    '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'driver'),
    ('Rajesh Gusain', 'driver4@demo.com',    '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'driver'),
    ('Admin User',    'admin@transitx.com',  '$2b$12$LJ3m4ys3Lk0TSwHlvTGKAOWZYQz5y0jXbkTvai38A.GlwIRMiMvNi', 'admin');

-- ---------------------------------------------------------------------------
-- BUSES  (UK-07-PA series, Uttarakhand registration)
-- ---------------------------------------------------------------------------

INSERT INTO buses (bus_number, registration_number, capacity, status) VALUES
    ('UK07PA7001', 'UK-07-PA-7001', 40, 'active'),
    ('UK07PA7002', 'UK-07-PA-7002', 42, 'active'),
    ('UK07PA7003', 'UK-07-PA-7003', 44, 'active'),
    ('UK07PA7004', 'UK-07-PA-7004', 48, 'active'),
    ('UK07PA7005', 'UK-07-PA-7005', 50, 'active'),
    ('UK07PA7006', 'UK-07-PA-7006', 52, 'inactive'),
    ('UK07PA7007', 'UK-07-PA-7007', 54, 'inactive'),
    ('UK07PA7008', 'UK-07-PA-7008', 56, 'active');

-- ---------------------------------------------------------------------------
-- DRIVERS  (user_ids 4-7 are the driver-role users)
-- ---------------------------------------------------------------------------

INSERT INTO drivers (user_id, license_number) VALUES
    (4, 'UK-DL-2020-001234'),
    (5, 'UK-DL-2019-005678'),
    (6, 'UK-DL-2021-009012'),
    (7, 'UK-DL-2018-003456');

-- ---------------------------------------------------------------------------
-- ROUTES  (Demo routes in Dehradun)
-- NOTE: These are demonstration routes, not official schedules.
-- ---------------------------------------------------------------------------

INSERT INTO routes (route_name, route_code, start_point, end_point, status) VALUES
    ('Demo: Clock Tower - ISBT',          'DR001', 'Clock Tower',   'ISBT Dehradun',    'active'),
    ('Demo: Rajpur Road - Mussoorie',     'DR002', 'Rajpur Road',   'Mussoorie',        'active'),
    ('Demo: Clement Town - Ballupur',     'DR003', 'Clement Town',  'Ballupur Chowk',   'active'),
    ('Demo: Rispana - Premnagar',         'DR004', 'Rispana Bridge','Premnagar',         'active'),
    ('Demo: Survey Chowk - Pacific Mall', 'DR005', 'Survey Chowk',  'Pacific Mall',     'active');

-- ---------------------------------------------------------------------------
-- STOPS  (Real Dehradun coordinates)
-- The trigger will auto-populate the location GEOGRAPHY column.
-- ---------------------------------------------------------------------------

INSERT INTO stops (stop_name, latitude, longitude) VALUES
    -- Route 1: Clock Tower → ISBT
    ('Clock Tower',              30.3165, 78.0322),   -- 1
    ('Paltan Bazaar',            30.3185, 78.0370),   -- 2
    ('Gandhi Road',              30.3148, 78.0400),   -- 3
    ('Nehru Colony',             30.3080, 78.0410),   -- 4
    ('Dalanwala',                30.3030, 78.0435),   -- 5
    ('ISBT Dehradun',            30.2724, 78.0462),   -- 6

    -- Route 2: Rajpur Road → Mussoorie
    ('Rajpur Road',              30.3380, 78.0540),   -- 7
    ('Jakhan',                   30.3480, 78.0580),   -- 8
    ('Malsi',                    30.3600, 78.0620),   -- 9
    ('Kolhu Khet',               30.3750, 78.0680),   -- 10
    ('Barlowganj',               30.4200, 78.0730),   -- 11
    ('Mussoorie Library Chowk',  30.4560, 78.0750),   -- 12

    -- Route 3: Clement Town → Ballupur
    ('Clement Town',             30.2880, 78.0180),   -- 13
    ('FRI Gate',                 30.3410, 78.0000),   -- 14
    ('Race Course',              30.3250, 78.0260),   -- 15
    ('Astley Hall',              30.3200, 78.0310),   -- 16
    ('Karanpur',                 30.3220, 78.0350),   -- 17
    ('Ballupur Chowk',           30.3310, 78.0480),   -- 18

    -- Route 4: Rispana → Premnagar
    ('Rispana Bridge',           30.3140, 78.0280),   -- 19
    ('Laxmi Road',               30.3100, 78.0220),   -- 20
    ('Chander Nagar',            30.3060, 78.0150),   -- 21
    ('Niranjanpur',              30.2980, 78.0100),   -- 22
    ('Premnagar',                30.2900, 78.0050),   -- 23

    -- Route 5: Survey Chowk → Pacific Mall
    ('Survey Chowk',             30.3195, 78.0290),   -- 24
    ('EC Road',                  30.3230, 78.0360),   -- 25
    ('Saharanpur Chowk',         30.3270, 78.0430),   -- 26
    ('Patel Nagar',              30.3330, 78.0500),   -- 27
    ('Shimla Bypass',            30.3400, 78.0590),   -- 28
    ('Pacific Mall',             30.3570, 78.0670);   -- 29

-- ---------------------------------------------------------------------------
-- ROUTE_STOPS  (linking stops to routes with correct ordering)
-- ---------------------------------------------------------------------------

-- Route 1: Clock Tower → ISBT  (stop ids 1-6)
INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) VALUES
    (1, 1,  1, 60),
    (1, 2,  2, 30),
    (1, 3,  3, 30),
    (1, 4,  4, 30),
    (1, 5,  5, 30),
    (1, 6,  6, 60);

-- Route 2: Rajpur Road → Mussoorie  (stop ids 7-12)
INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) VALUES
    (2, 7,  1, 60),
    (2, 8,  2, 30),
    (2, 9,  3, 30),
    (2, 10, 4, 30),
    (2, 11, 5, 30),
    (2, 12, 6, 60);

-- Route 3: Clement Town → Ballupur  (stop ids 13-18)
INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) VALUES
    (3, 13, 1, 60),
    (3, 14, 2, 30),
    (3, 15, 3, 30),
    (3, 16, 4, 30),
    (3, 17, 5, 30),
    (3, 18, 6, 60);

-- Route 4: Rispana → Premnagar  (stop ids 19-23)
INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) VALUES
    (4, 19, 1, 60),
    (4, 20, 2, 30),
    (4, 21, 3, 30),
    (4, 22, 4, 30),
    (4, 23, 5, 60);

-- Route 5: Survey Chowk → Pacific Mall  (stop ids 24-29)
INSERT INTO route_stops (route_id, stop_id, stop_order, estimated_stop_duration) VALUES
    (5, 24, 1, 60),
    (5, 25, 2, 30),
    (5, 26, 3, 30),
    (5, 27, 4, 30),
    (5, 28, 5, 30),
    (5, 29, 6, 60);

-- ---------------------------------------------------------------------------
-- TRIPS  (sample completed trips)
-- NOTE: Demonstration data only.
-- ---------------------------------------------------------------------------

INSERT INTO trips (bus_id, route_id, driver_id, started_at, ended_at, status) VALUES
    (1, 1, 1, '2026-11-16 07:00:00+05:30', '2026-11-16 07:55:00+05:30', 'completed'),
    (2, 2, 2, '2026-11-16 08:30:00+05:30', '2026-11-16 10:15:00+05:30', 'completed');

-- ---------------------------------------------------------------------------
-- FAVORITES  (sample favorites)
-- ---------------------------------------------------------------------------

INSERT INTO favorites (user_id, route_id) VALUES
    (1, 1),
    (1, 2),
    (2, 3);

-- =============================================================================
-- END OF SEED DATA
-- =============================================================================
