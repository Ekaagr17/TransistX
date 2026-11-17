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
