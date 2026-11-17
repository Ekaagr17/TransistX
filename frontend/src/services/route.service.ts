import api from './api';
import type { Route, RouteStop } from '../types';

export const routeService = {
  async getRoutes(): Promise<Route[]> {
    const res = await api.get<Route[]>('/routes');
    return res.data;
  },

  async getRoute(id: number): Promise<Route> {
    const res = await api.get<Route>(`/routes/${id}`);
    return res.data;
  },

  async getRouteStops(routeId: number): Promise<RouteStop[]> {
    const res = await api.get<RouteStop[]>(`/routes/${routeId}/stops`);
    return res.data;
  },

  async createRoute(data: Partial<Route>): Promise<Route> {
    const res = await api.post<Route>('/routes', data);
    return res.data;
  },

  async updateRoute(id: number, data: Partial<Route>): Promise<Route> {
    const res = await api.put<Route>(`/routes/${id}`, data);
    return res.data;
  },

  async deleteRoute(id: number): Promise<void> {
    await api.delete(`/routes/${id}`);
  },

  async addRouteStop(routeId: number, data: { stop_id: number; stop_order: number }): Promise<RouteStop> {
    const res = await api.post<RouteStop>(`/routes/${routeId}/stops`, data);
    return res.data;
  },

  async removeRouteStop(routeId: number, stopId: number): Promise<void> {
    await api.delete(`/routes/${routeId}/stops/${stopId}`);
  },
};
