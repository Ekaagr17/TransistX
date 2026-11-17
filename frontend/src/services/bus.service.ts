import api from './api';
import type { Bus, BusLocation, EtaResult } from '../types';

export const busService = {
  async getBuses(): Promise<Bus[]> {
    const res = await api.get<Bus[]>('/buses');
    return res.data;
  },

  async getBus(id: number): Promise<Bus> {
    const res = await api.get<Bus>(`/buses/${id}`);
    return res.data;
  },

  async getBusLocation(busId: number): Promise<BusLocation> {
    const res = await api.get<BusLocation>(`/buses/${busId}/location`);
    return res.data;
  },

  async getBusEta(busId: number, stopId: number): Promise<EtaResult> {
    const res = await api.get<EtaResult>(`/buses/${busId}/eta/${stopId}`);
    return res.data;
  },

  async createBus(data: Partial<Bus>): Promise<Bus> {
    const res = await api.post<Bus>('/buses', data);
    return res.data;
  },

  async updateBus(id: number, data: Partial<Bus>): Promise<Bus> {
    const res = await api.put<Bus>(`/buses/${id}`, data);
    return res.data;
  },

  async deleteBus(id: number): Promise<void> {
    await api.delete(`/buses/${id}`);
  },
};
