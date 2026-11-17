import { useState, useEffect } from 'react';
import type { SearchResult } from '../types';
import { busService } from '../services/bus.service';
import { routeService } from '../services/route.service';
import { stopService } from '../services/stop.service';

export const useSearch = () => {
  const [query, setQuery] = useState<string>('');
  const [results, setResults] = useState<SearchResult[]>([]);
  const [isSearching, setIsSearching] = useState<boolean>(false);

  useEffect(() => {
    if (!query.trim() || query.length < 2) {
      setResults([]);
      setIsSearching(false);
      return;
    }

    const timer = setTimeout(async () => {
      setIsSearching(true);
      try {
        const q = query.toLowerCase();
        const [buses, routes, stops] = await Promise.all([
          busService.getBuses().catch(() => []),
          routeService.getRoutes().catch(() => []),
          stopService.getStops().catch(() => []),
        ]);

        const matchedBuses: SearchResult[] = buses
          .filter(
            (b) =>
              b.bus_number.toLowerCase().includes(q) ||
              (b.registration_number && b.registration_number.toLowerCase().includes(q))
          )
          .map((b) => ({
            type: 'bus' as const,
            id: b.id,
            label: `Bus ${b.bus_number}`,
            sublabel: b.registration_number || 'No reg number',
          }));

        const matchedRoutes: SearchResult[] = routes
          .filter(
            (r) =>
              r.route_name.toLowerCase().includes(q) ||
              r.route_code.toLowerCase().includes(q)
          )
          .map((r) => ({
            type: 'route' as const,
            id: r.id,
            label: `${r.route_code}: ${r.route_name}`,
            sublabel: `${r.start_point || ''} → ${r.end_point || ''}`,
          }));

        const matchedStops: SearchResult[] = stops
          .filter((s) => s.stop_name.toLowerCase().includes(q))
          .map((s) => ({
            type: 'stop' as const,
            id: s.id,
            label: s.stop_name,
            sublabel: `Lat: ${s.latitude.toFixed(4)}, Lng: ${s.longitude.toFixed(4)}`,
          }));

        setResults([...matchedBuses, ...matchedRoutes, ...matchedStops]);
      } catch {
        setResults([]);
      } finally {
        setIsSearching(false);
      }
    }, 300);

    return () => clearTimeout(timer);
  }, [query]);

  return {
    query,
    setQuery,
    results,
    isSearching,
    clearSearch: () => {
      setQuery('');
      setResults([]);
    },
  };
};
