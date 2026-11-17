export interface User {
  id: number;
  name: string;
  email: string;
  role: 'passenger' | 'driver' | 'admin';
  created_at: string;
}

export interface Bus {
  id: number;
  bus_number: string;
  registration_number: string | null;
  capacity: number | null;
  status: 'active' | 'inactive';
  created_at: string;
  route_id?: number;
}

export interface Route {
  id: number;
  route_name: string;
  route_code: string;
  start_point: string | null;
  end_point: string | null;
  status: 'active' | 'inactive';
}

export interface Stop {
  id: number;
  stop_name: string;
  latitude: number;
  longitude: number;
}

export interface RouteStop {
  id: number;
  route_id: number;
  stop_id: number;
  stop_order: number;
  estimated_stop_duration: number;
  stop?: Stop;
}

export interface Trip {
  id: number;
  bus_id: number;
  route_id: number;
  driver_id: number;
  started_at: string | null;
  ended_at: string | null;
  status: 'scheduled' | 'active' | 'completed' | 'cancelled';
  bus?: Bus;
  route?: Route;
  driver?: Driver;
}

export interface Driver {
  id: number;
  user_id: number;
  license_number: string;
  user?: User;
}

export interface GpsUpdate {
  id: number;
  bus_id: number;
  trip_id: number | null;
  latitude: number;
  longitude: number;
  speed: number | null;
  heading: number | null;
  accuracy: number | null;
  recorded_at: string;
}

export interface BusLocation {
  bus_id: number;
  latitude: number;
  longitude: number;
  speed: number | null;
  heading: number | null;
  accuracy: number | null;
  updated_at: string;
  bus?: Bus;
  trip?: Trip;
  route?: Route;
  next_stop?: Stop;
  distance_to_next_stop?: number;
}

export interface Favorite {
  id: number;
  user_id: number;
  route_id: number;
  created_at: string;
  route?: Route;
}

export interface FavoriteBus {
  id: number;
  bus_id: number;
  bus_number: string;
  route_name?: string;
  added_at: string;
}

export interface ServiceAlert {
  id: number;
  route_id: number | null;
  message: string;
  severity: 'info' | 'warning' | 'critical';
  created_at: string;
  expires_at: string | null;
  route?: Route;
}

export interface EtaResult {
  bus_id: number;
  stop_id: number;
  eta_minutes: number;
  distance_km: number;
  bus?: Bus;
}

export interface SearchResult {
  type: 'bus' | 'route' | 'stop';
  id: number;
  label: string;
  sublabel: string;
}

export interface AuthResponse {
  token: string;
  user: User;
}

export interface ApiResponse<T> {
  data: T;
  message?: string;
  error?: string;
}

export interface WebSocketMessage {
  type: 'location_update' | 'bus_status' | 'alert';
  data: BusLocation | Bus | ServiceAlert;
}

// ==========================================
// Journey Planner & Time Engine Types
// ==========================================

export interface StopTiming {
  stop_id: number;
  stop_name: string;
  stop_order: number;
  latitude: number;
  longitude: number;
  scheduled_time: string;
  estimated_time: string;
  eta_minutes_from_now: number;
  is_passed: boolean;
  distance_from_prev_km: number;
}

export interface JourneyCandidate {
  bus_id: number;
  bus_number: string;
  registration_number: string;
  capacity: number;
  status: 'active' | 'inactive';
  route_id: number;
  route_code: string;
  route_name: string;
  start_point: string;
  end_point: string;
  boarding_stop: Stop;
  destination_stop: Stop;
  boarding_eta_minutes: number;
  arrival_time_at_boarding: string;
  destination_eta_minutes: number;
  arrival_time_at_destination: string;
  total_trip_minutes: number;
  walking_distance_meters: number;
  walking_duration_minutes: number;
  is_best_pick: boolean;
  best_pick_reason?: string;
  stop_timings: StopTiming[];
  current_speed_kmh: number;
  current_location: {
    latitude: number;
    longitude: number;
  };
}

export interface JourneyPlanRequest {
  start_stop_id: number;
  end_stop_id: number;
  passenger_lat?: number;
  passenger_lng?: number;
  preferred_bus_id?: number;
}
