import { useState, useEffect, useCallback } from 'react';
import type { Favorite } from '../types';
import { favoriteService } from '../services/favorite.service';
import { useAuth } from './useAuth';

export const useFavorites = () => {
  const { isAuthenticated } = useAuth();
  const [favorites, setFavorites] = useState<Favorite[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  const fetchFavorites = useCallback(async () => {
    if (!isAuthenticated) {
      setFavorites([]);
      return;
    }
    try {
      setLoading(true);
      setError(null);
      const data = await favoriteService.getFavorites();
      setFavorites(data);
    } catch (err: any) {
      setError(err?.response?.data?.message || err?.message || 'Failed to fetch favorites');
    } finally {
      setLoading(false);
    }
  }, [isAuthenticated]);

  useEffect(() => {
    fetchFavorites();
  }, [fetchFavorites]);

  const isFavorite = useCallback(
    (routeId: number) => {
      return favorites.some((f) => f.route_id === routeId);
    },
    [favorites]
  );

  const toggleFavorite = async (routeId: number) => {
    if (!isAuthenticated) return;

    const existing = favorites.find((f) => f.route_id === routeId);

    if (existing) {
      // Optimistic remove
      setFavorites((prev) => prev.filter((f) => f.id !== existing.id));
      try {
        await favoriteService.removeFavorite(existing.id);
      } catch {
        // Rollback
        setFavorites((prev) => [...prev, existing]);
      }
    } else {
      // Optimistic add
      const tempFav: Favorite = {
        id: Date.now(),
        user_id: 0,
        route_id: routeId,
        created_at: new Date().toISOString(),
      };
      setFavorites((prev) => [...prev, tempFav]);
      try {
        const saved = await favoriteService.addFavorite(routeId);
        setFavorites((prev) => prev.map((f) => (f.id === tempFav.id ? saved : f)));
      } catch {
        // Rollback
        setFavorites((prev) => prev.filter((f) => f.id !== tempFav.id));
      }
    }
  };

  return {
    favorites,
    loading,
    error,
    isFavorite,
    toggleFavorite,
    refetch: fetchFavorites,
  };
};
