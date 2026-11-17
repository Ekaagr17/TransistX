import { useState, useEffect, useCallback } from 'react';
import type { Bus, BusLocation } from '../types';
import { busService } from '../services/bus.service';
import { wsService } from '../services/websocket.service';

export const useBuses = () => {
  const [buses, setBuses] = useState<Bus[]>([]);
  const [locations, setLocations] = useState<Map<number, BusLocation>>(new Map());
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);

  const fetchBuses = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);
      const data = await busService.getBuses();
      setBuses(data);

      // fetch current locations for active buses
      const initialLocations = new Map<number, BusLocation>();
      await Promise.all(
        data.map(async (bus) => {
          try {
            const loc = await busService.getBusLocation(bus.id);
            if (loc && loc.latitude && loc.longitude) {
              initialLocations.set(bus.id, loc);
            }
          } catch {
            // Bus might not have an active location
          }
        })
      );
      setLocations(initialLocations);
    } catch (err: any) {
      setError(err?.response?.data?.message || err?.message || 'Failed to fetch buses');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchBuses();

    const unsubscribe = wsService.on('location_update', (newLoc: BusLocation) => {
      setLocations((prev) => {
        const next = new Map(prev);
        next.set(newLoc.bus_id, newLoc);
        return next;
      });
    });

    return () => {
      unsubscribe();
    };
  }, [fetchBuses]);

  return {
    buses,
    locations,
    loading,
    error,
    refetch: fetchBuses,
  };
};
