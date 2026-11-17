import api from './api';

export type TripStatus = 'idle' | 'active' | 'paused' | 'ended';

export interface Trip {
  id: number;
  bus_id: number;
  route_id: number;
  driver_id: number;
  status: string;
  started_at: string;
  ended_at?: string;
}

export interface StartTripPayload {
  bus_id: number;
  route_id: number;
}

export const tripService = {
  async startTrip(payload: StartTripPayload): Promise<Trip> {
    const { data } = await api.post<Trip>('/trips/start', payload);
    return data;
  },

  async pauseTrip(tripId: number): Promise<Trip> {
    const { data } = await api.post<Trip>(`/trips/${tripId}/pause`);
    return data;
  },

  async resumeTrip(tripId: number): Promise<Trip> {
    const { data } = await api.post<Trip>(`/trips/${tripId}/resume`);
    return data;
  },

  async endTrip(tripId: number): Promise<Trip> {
    const { data } = await api.post<Trip>(`/trips/${tripId}/end`);
    return data;
  },

  async getCurrentTrip(): Promise<Trip | null> {
    try {
      const { data } = await api.get<Trip>('/trips/current');
      return data;
    } catch {
      return null;
    }
  },
};
