import api from './api';

export interface GpsPayload {
  latitude: number;
  longitude: number;
  speed: number | null;
  heading: number | null;
  accuracy: number;
  timestamp: string;
  bus_id: number;
  trip_id: number;
}

export interface GpsResponse {
  success: boolean;
  message?: string;
}

export const gpsService = {
  async sendUpdate(payload: GpsPayload): Promise<GpsResponse> {
    const { data } = await api.post<GpsResponse>('/gps/update', payload);
    return data;
  },
};
