import api from './api';
import type { Stop } from '../types';

export const stopService = {
  async getStops(): Promise<Stop[]> {
    const res = await api.get<Stop[]>('/stops');
    return res.data;
  },

  async getStop(id: number): Promise<Stop> {
    const res = await api.get<Stop>(`/stops/${id}`);
    return res.data;
  },

  async getNearbyStops(lat: number, lng: number, radiusKm: number = 2): Promise<Stop[]> {
    const res = await api.get<Stop[]>('/stops/nearby', {
      params: { lat, lng, radius: radiusKm },
    });
    return res.data;
  },

  async createStop(data: Partial<Stop>): Promise<Stop> {
    const res = await api.post<Stop>('/stops', data);
    return res.data;
  },

  async updateStop(id: number, data: Partial<Stop>): Promise<Stop> {
    const res = await api.put<Stop>(`/stops/${id}`, data);
    return res.data;
  },

  async deleteStop(id: number): Promise<void> {
    await api.delete(`/stops/${id}`);
  },
};
