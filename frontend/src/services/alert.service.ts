import api from './api';
import type { ServiceAlert } from '../types';

export const alertService = {
  async getAlerts(): Promise<ServiceAlert[]> {
    const res = await api.get<ServiceAlert[]>('/alerts');
    return res.data;
  },

  async createAlert(data: Partial<ServiceAlert>): Promise<ServiceAlert> {
    const res = await api.post<ServiceAlert>('/alerts', data);
    return res.data;
  },

  async deleteAlert(id: number): Promise<void> {
    await api.delete(`/alerts/${id}`);
  },
};
