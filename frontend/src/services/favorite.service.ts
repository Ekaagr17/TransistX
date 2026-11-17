import api from './api';
import type { Favorite } from '../types';

export const favoriteService = {
  async getFavorites(): Promise<Favorite[]> {
    const res = await api.get<Favorite[]>('/favorites');
    return res.data;
  },

  async addFavorite(routeId: number): Promise<Favorite> {
    const res = await api.post<Favorite>('/favorites', { route_id: routeId });
    return res.data;
  },

  async removeFavorite(id: number): Promise<void> {
    await api.delete(`/favorites/${id}`);
  },
};
