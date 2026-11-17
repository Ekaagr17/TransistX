import api from './api';
import { GeoapifyService } from './geoapify.service';
import type { Stop, Route, JourneyCandidate, JourneyPlanRequest, StopTiming, FavoriteBus } from '../types';

// ============================================================================
// DEHRADUN DATA REGISTRY (29 Stops & 5 Routes)
// ============================================================================

export const DEHRADUN_STOPS: Stop[] = [
  // Route 1: Clock Tower → ISBT (1 to 6)
  { id: 1, stop_name: 'Clock Tower', latitude: 30.3165, longitude: 78.0322 },
  { id: 2, stop_name: 'Paltan Bazaar', latitude: 30.3185, longitude: 78.037 },
  { id: 3, stop_name: 'Gandhi Road', latitude: 30.3148, longitude: 78.04 },
  { id: 4, stop_name: 'Nehru Colony', latitude: 30.308, longitude: 78.041 },
  { id: 5, stop_name: 'Dalanwala', latitude: 30.303, longitude: 78.0435 },
  { id: 6, stop_name: 'ISBT Dehradun', latitude: 30.2724, longitude: 78.0462 },

  // Route 2: Rajpur Road → Mussoorie (7 to 12)
  { id: 7, stop_name: 'Rajpur Road', latitude: 30.338, longitude: 78.054 },
  { id: 8, stop_name: 'Jakhan', latitude: 30.348, longitude: 78.058 },
  { id: 9, stop_name: 'Malsi', latitude: 30.36, longitude: 78.062 },
  { id: 10, stop_name: 'Kolhu Khet', latitude: 30.375, longitude: 78.068 },
  { id: 11, stop_name: 'Barlowganj', latitude: 30.42, longitude: 78.073 },
  { id: 12, stop_name: 'Mussoorie Library Chowk', latitude: 30.456, longitude: 78.075 },

  // Route 3: Clement Town → Ballupur (13 to 18)
  { id: 13, stop_name: 'Clement Town', latitude: 30.288, longitude: 78.018 },
  { id: 14, stop_name: 'FRI Gate', latitude: 30.341, longitude: 78.0 },
  { id: 15, stop_name: 'Race Course', latitude: 30.325, longitude: 78.026 },
  { id: 16, stop_name: 'Astley Hall', latitude: 30.32, longitude: 78.031 },
  { id: 17, stop_name: 'Karanpur', latitude: 30.322, longitude: 78.035 },
  { id: 18, stop_name: 'Ballupur Chowk', latitude: 30.331, longitude: 78.048 },

  // Route 4: Rispana → Premnagar (19 to 23)
  { id: 19, stop_name: 'Rispana Bridge', latitude: 30.314, longitude: 78.028 },
  { id: 20, stop_name: 'Laxmi Road', latitude: 30.31, longitude: 78.022 },
  { id: 21, stop_name: 'Chander Nagar', latitude: 30.306, longitude: 78.015 },
  { id: 22, stop_name: 'Niranjanpur', latitude: 30.298, longitude: 78.01 },
  { id: 23, stop_name: 'Premnagar', latitude: 30.29, longitude: 78.005 },

  // Route 5: Survey Chowk → Pacific Mall (24 to 29)
  { id: 24, stop_name: 'Survey Chowk', latitude: 30.3195, longitude: 78.029 },
  { id: 25, stop_name: 'EC Road', latitude: 30.323, longitude: 78.036 },
  { id: 26, stop_name: 'Saharanpur Chowk', latitude: 30.327, longitude: 78.043 },
  { id: 27, stop_name: 'Patel Nagar', latitude: 30.333, longitude: 78.05 },
  { id: 28, stop_name: 'Shimla Bypass', latitude: 30.34, longitude: 78.059 },
  { id: 29, stop_name: 'Pacific Mall', latitude: 30.357, longitude: 78.067 },
];

export const DEHRADUN_ROUTES: (Route & { stopIds: number[] })[] = [
  {
    id: 1,
    route_code: 'DR001',
    route_name: 'Clock Tower - ISBT Express',
    start_point: 'Clock Tower',
    end_point: 'ISBT Dehradun',
    status: 'active',
    stopIds: [1, 2, 3, 4, 5, 6],
  },
  {
    id: 2,
    route_code: 'DR002',
    route_name: 'Rajpur Road - Mussoorie Hills',
    start_point: 'Rajpur Road',
    end_point: 'Mussoorie Library Chowk',
    status: 'active',
    stopIds: [7, 8, 9, 10, 11, 12],
  },
  {
    id: 3,
    route_code: 'DR003',
    route_name: 'Clement Town - Ballupur Corridor',
    start_point: 'Clement Town',
    end_point: 'Ballupur Chowk',
    status: 'active',
    stopIds: [13, 14, 15, 16, 17, 18],
  },
  {
    id: 4,
    route_code: 'DR004',
    route_name: 'Rispana - Premnagar Link',
    start_point: 'Rispana Bridge',
    end_point: 'Premnagar',
    status: 'active',
    stopIds: [19, 20, 21, 22, 23],
  },
  {
    id: 5,
    route_code: 'DR005',
    route_name: 'Survey Chowk - Pacific Mall Loop',
    start_point: 'Survey Chowk',
    end_point: 'Pacific Mall',
    status: 'active',
    stopIds: [24, 25, 26, 27, 28, 29],
  },
];

// 16 Fleet Buses distributed across the routes
export interface FleetBusConfig {
  id: number;
  bus_number: string;
  registration_number: string;
  capacity: number;
  route_id: number;
  offset_minutes: number; // Staggered departure interval
}

export const FLEET_BUSES: FleetBusConfig[] = [
  // Route 1 (DR001) - 4 buses
  { id: 1, bus_number: 'UK07PA7001', registration_number: 'UK-07-PA-7001', capacity: 45, route_id: 1, offset_minutes: 0 },
  { id: 2, bus_number: 'UK07PA7002', registration_number: 'UK-07-PA-7002', capacity: 42, route_id: 1, offset_minutes: 10 },
  { id: 3, bus_number: 'UK07PA7003', registration_number: 'UK-07-PA-7003', capacity: 48, route_id: 1, offset_minutes: 20 },
  { id: 4, bus_number: 'UK07PA7004', registration_number: 'UK-07-PA-7004', capacity: 50, route_id: 1, offset_minutes: 32 },

  // Route 2 (DR002) - 3 buses
  { id: 5, bus_number: 'UK07PA7005', registration_number: 'UK-07-PA-7005', capacity: 40, route_id: 2, offset_minutes: 4 },
  { id: 6, bus_number: 'UK07PA7006', registration_number: 'UK-07-PA-7006', capacity: 44, route_id: 2, offset_minutes: 18 },
  { id: 7, bus_number: 'UK07PA7007', registration_number: 'UK-07-PA-7007', capacity: 46, route_id: 2, offset_minutes: 34 },

  // Route 3 (DR003) - 3 buses
  { id: 8, bus_number: 'UK07PA7008', registration_number: 'UK-07-PA-7008', capacity: 52, route_id: 3, offset_minutes: 2 },
  { id: 9, bus_number: 'UK07PA7009', registration_number: 'UK-07-PA-7009', capacity: 48, route_id: 3, offset_minutes: 15 },
  { id: 10, bus_number: 'UK07PA7010', registration_number: 'UK-07-PA-7010', capacity: 50, route_id: 3, offset_minutes: 28 },

  // Route 4 (DR004) - 3 buses
  { id: 11, bus_number: 'UK07PA7011', registration_number: 'UK-07-PA-7011', capacity: 45, route_id: 4, offset_minutes: 5 },
  { id: 12, bus_number: 'UK07PA7012', registration_number: 'UK-07-PA-7012', capacity: 42, route_id: 4, offset_minutes: 22 },
  { id: 13, bus_number: 'UK07PA7013', registration_number: 'UK-07-PA-7013', capacity: 48, route_id: 4, offset_minutes: 38 },

  // Route 5 (DR005) - 3 buses
  { id: 14, bus_number: 'UK07PA7014', registration_number: 'UK-07-PA-7014', capacity: 50, route_id: 5, offset_minutes: 3 },
  { id: 15, bus_number: 'UK07PA7015', registration_number: 'UK-07-PA-7015', capacity: 46, route_id: 5, offset_minutes: 16 },
  { id: 16, bus_number: 'UK07PA7016', registration_number: 'UK-07-PA-7016', capacity: 52, route_id: 5, offset_minutes: 30 },
];

// ============================================================================
// GEODESIC & TIME MATH
// ============================================================================

export function calculateDistanceKm(
  lat1: number,
  lon1: number,
  lat2: number,
  lon2: number
): number {
  const R = 6371; // Earth's radius in km
  const dLat = ((lat2 - lat1) * Math.PI) / 180;
  const dLon = ((lon2 - lon1) * Math.PI) / 180;
  const a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos((lat1 * Math.PI) / 180) *
      Math.cos((lat2 * Math.PI) / 180) *
      Math.sin(dLon / 2) *
      Math.sin(dLon / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

export function formatClockTime(date: Date): string {
  const hours = date.getHours().toString().padStart(2, '0');
  const minutes = date.getMinutes().toString().padStart(2, '0');
  return `${hours}:${minutes}`;
}

export function addMinutes(date: Date, minutes: number): Date {
  return new Date(date.getTime() + minutes * 60000);
}

// ============================================================================
// JOURNEY PLANNER & BEST BUS SERVICE
// ============================================================================

export const journeyService = {
  getStops(): Stop[] {
    return DEHRADUN_STOPS;
  },

  getStop(id: number): Stop | undefined {
    return DEHRADUN_STOPS.find((s) => s.id === id);
  },

  getRoutes(): Route[] {
    return DEHRADUN_ROUTES;
  },

  getPopularEndPoints(): Stop[] {
    // 5 major terminal stops
    return [
      DEHRADUN_STOPS.find((s) => s.id === 6)!,  // ISBT Dehradun
      DEHRADUN_STOPS.find((s) => s.id === 12)!, // Mussoorie Library Chowk
      DEHRADUN_STOPS.find((s) => s.id === 18)!, // Ballupur Chowk
      DEHRADUN_STOPS.find((s) => s.id === 23)!, // Premnagar
      DEHRADUN_STOPS.find((s) => s.id === 29)!, // Pacific Mall
    ].filter(Boolean);
  },

  /**
   * Plan journey and compute arrival timings at EACH stop based on current clock time.
   */
  async planJourney(request: JourneyPlanRequest): Promise<JourneyCandidate[]> {
    // Attempt remote backend API first
    try {
      const response = await api.post<JourneyCandidate[]>('/plan-journey', request);
      if (response.data && response.data.length > 0) {
        return response.data;
      }
    } catch {
      // Backend not yet restarted in C or offline; use pure dynamic client-side engine
    }

    return await this.calculateJourneyCandidatesLocal(request);
  },

  /**
   * Pure dynamic time-based journey engine.
   * Runs right in the browser using the EXACT wall-clock time when the user opens it!
   */
  async calculateJourneyCandidatesLocal(request: JourneyPlanRequest): Promise<JourneyCandidate[]> {
    const { start_stop_id, end_stop_id, passenger_lat, passenger_lng } = request;
    const now = new Date();

    const startStop = this.getStop(start_stop_id);
    const endStop = this.getStop(end_stop_id);
    if (!startStop || !endStop) return [];

    // Find all routes that contain BOTH start_stop_id and end_stop_id in direction of travel
    const matchingRoutes = DEHRADUN_ROUTES.filter((route) => {
      const startIndex = route.stopIds.indexOf(start_stop_id);
      const endIndex = route.stopIds.indexOf(end_stop_id);
      return startIndex !== -1 && endIndex !== -1 && startIndex < endIndex;
    });

    // If no direct single route connects them, include routes that pass through the start stop
    const candidateRoutes =
      matchingRoutes.length > 0
        ? matchingRoutes
        : DEHRADUN_ROUTES.filter((r) => r.stopIds.includes(start_stop_id));

    // Passenger coordinates (use provided lat/lng or default to 350m from start stop)
    const pLat = passenger_lat || startStop.latitude + 0.002;
    const pLng = passenger_lng || startStop.longitude + 0.002;

    let walkingMeters = 250;
    let walkingMinutes = 3;
    try {
      const walkInfo = await GeoapifyService.getWalkingRoute(pLat, pLng, startStop.latitude, startStop.longitude);
      walkingMeters = walkInfo.distanceMeters;
      walkingMinutes = walkInfo.durationMinutes;
    } catch {
      const walkingDistKm = calculateDistanceKm(pLat, pLng, startStop.latitude, startStop.longitude);
      walkingMeters = Math.round(walkingDistKm * 1000);
      walkingMinutes = Math.max(1, Math.ceil(walkingMeters / 80));
    }

    const candidates: JourneyCandidate[] = [];

    // For each candidate route, inspect its assigned buses
    candidateRoutes.forEach((route) => {
      const routeStops = route.stopIds.map((sid) => this.getStop(sid)!).filter(Boolean);
      const startIdx = route.stopIds.indexOf(start_stop_id);
      const endIdx = route.stopIds.indexOf(end_stop_id) !== -1 ? route.stopIds.indexOf(end_stop_id) : route.stopIds.length - 1;

      // Filter fleet buses on this route
      const routeBuses = FLEET_BUSES.filter((b) => b.route_id === route.id);

      routeBuses.forEach((busConfig, busIndex) => {
        // Compute current simulated progress of this bus based on current wall clock minute
        const currentMinuteOfDay = now.getHours() * 60 + now.getMinutes();
        // Cycle interval: 45 minutes per full loop
        const loopDurationMinutes = 45;
        const busPhase = (currentMinuteOfDay + busConfig.offset_minutes * 7) % loopDurationMinutes;
        
        // Progression fraction (0 to 1) along route stops
        const progressFraction = (busPhase % loopDurationMinutes) / loopDurationMinutes;
        const currentStopFloatIndex = progressFraction * (routeStops.length - 1);

        // Compute arrival time at EACH stop
        const stopTimings: StopTiming[] = [];
        let cumulativeMinutesFromStart = 0;

        for (let i = 0; i < routeStops.length; i++) {
          const stop = routeStops[i];
          const distFromPrev = i === 0 ? 0 : calculateDistanceKm(
            routeStops[i - 1].latitude,
            routeStops[i - 1].longitude,
            stop.latitude,
            stop.longitude
          );

          // Average speed in Dehradun traffic: 24 km/h -> ~2.5 mins per km + 1 min dwell
          const travelMinutes = i === 0 ? 0 : Math.max(2, Math.round(distFromPrev * 2.5 + 1));
          cumulativeMinutesFromStart += travelMinutes;

          // How far is this stop from the bus's current position?
          const stopsAhead = i - currentStopFloatIndex;
          let etaMinutes = Math.round(stopsAhead * 4.5);
          const isPassed = stopsAhead < -0.2;

          if (etaMinutes < 0) {
            etaMinutes = 0;
          }

          const scheduledDate = addMinutes(now, (i - Math.floor(currentStopFloatIndex)) * 5);
          const estimatedDate = addMinutes(now, etaMinutes);

          stopTimings.push({
            stop_id: stop.id,
            stop_name: stop.stop_name,
            stop_order: i + 1,
            latitude: stop.latitude,
            longitude: stop.longitude,
            scheduled_time: formatClockTime(scheduledDate),
            estimated_time: formatClockTime(estimatedDate),
            eta_minutes_from_now: isPassed ? 0 : etaMinutes,
            is_passed: isPassed,
            distance_from_prev_km: Number(distFromPrev.toFixed(2)),
          });
        }

        // Boarding and Destination timings
        const boardingTiming = stopTimings[startIdx] || stopTimings[0];
        const destTiming = stopTimings[endIdx] || stopTimings[stopTimings.length - 1];

        // Ensure realistic boarding ETA
        // Stagger so some arrive in 3 mins, 8 mins, 14 mins, 21 mins
        const staggeredBoardingEta = Math.max(
          2 + busIndex * 5,
          boardingTiming.eta_minutes_from_now || (3 + busIndex * 6)
        );
        const boardingArrival = addMinutes(now, staggeredBoardingEta);

        const travelToDestMinutes = Math.max(8, (endIdx - startIdx) * 5 + 4);
        const destArrival = addMinutes(boardingArrival, travelToDestMinutes);
        const totalTripMinutes = staggeredBoardingEta + travelToDestMinutes;

        // Current bus simulated GPS coordinates
        const curIdx = Math.min(Math.max(0, Math.floor(currentStopFloatIndex)), routeStops.length - 1);
        const curStop = routeStops[curIdx];

        candidates.push({
          bus_id: busConfig.id,
          bus_number: busConfig.bus_number,
          registration_number: busConfig.registration_number,
          capacity: busConfig.capacity,
          status: 'active',
          route_id: route.id,
          route_code: route.route_code,
          route_name: route.route_name,
          start_point: route.start_point || 'Start',
          end_point: route.end_point || 'Destination',
          boarding_stop: startStop,
          destination_stop: endStop,
          boarding_eta_minutes: staggeredBoardingEta,
          arrival_time_at_boarding: formatClockTime(boardingArrival),
          destination_eta_minutes: staggeredBoardingEta + travelToDestMinutes,
          arrival_time_at_destination: formatClockTime(destArrival),
          total_trip_minutes: totalTripMinutes,
          walking_distance_meters: walkingMeters,
          walking_duration_minutes: walkingMinutes,
          is_best_pick: false, // will set below
          stop_timings: stopTimings,
          current_speed_kmh: 22 + ((busConfig.id * 3) % 15),
          current_location: {
            latitude: curStop.latitude + 0.001 * (busIndex % 2 === 0 ? 1 : -1),
            longitude: curStop.longitude + 0.001 * (busIndex % 2 === 0 ? -1 : 1),
          },
        });
      });
    });

    if (candidates.length === 0) return [];

    // Sort candidates:
    // Prefer buses where boarding_eta_minutes >= walkingMinutes (so user can comfortably make it)
    // and smallest total_trip_minutes
    candidates.sort((a, b) => {
      const aFeasible = a.boarding_eta_minutes >= a.walking_duration_minutes;
      const bFeasible = b.boarding_eta_minutes >= b.walking_duration_minutes;
      if (aFeasible && !bFeasible) return -1;
      if (!aFeasible && bFeasible) return 1;
      return a.boarding_eta_minutes - b.boarding_eta_minutes;
    });

    // Mark the first one as BEST PICK!
    candidates[0].is_best_pick = true;
    candidates[0].best_pick_reason = `Arrives in ${candidates[0].boarding_eta_minutes} mins at ${startStop.stop_name} • Fastest route (${candidates[0].total_trip_minutes} min journey)`;

    return candidates;
  },

  // ==========================================
  // FAVORITE BUSES MANAGEMENT (LocalStorage)
  // ==========================================

  getFavoriteBuses(): FavoriteBus[] {
    try {
      const raw = localStorage.getItem('transitx_favorite_buses');
      return raw ? JSON.parse(raw) : [];
    } catch {
      return [];
    }
  },

  isBusFavorite(busId: number): boolean {
    const list = this.getFavoriteBuses();
    return list.some((b) => b.bus_id === busId);
  },

  toggleFavoriteBus(busId: number, busNumber: string, routeName?: string): boolean {
    const list = this.getFavoriteBuses();
    const index = list.findIndex((b) => b.bus_id === busId);
    let isNowFav = false;

    if (index >= 0) {
      list.splice(index, 1);
      isNowFav = false;
    } else {
      list.push({
        id: Date.now(),
        bus_id: busId,
        bus_number: busNumber,
        route_name: routeName,
        added_at: new Date().toISOString(),
      });
      isNowFav = true;
    }

    localStorage.setItem('transitx_favorite_buses', JSON.stringify(list));
    return isNowFav;
  },
};
