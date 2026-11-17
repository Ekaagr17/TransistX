import React, { useState, useEffect } from 'react';
import { Route as RouteIcon } from 'lucide-react';
import { routeService } from '../services/route.service';
import { stopService } from '../services/stop.service';
import { DEHRADUN_ROUTES, DEHRADUN_STOPS } from '../services/journey.service';
import { RouteCard } from '../components/RouteCard';
import { RoutePanel } from '../components/RoutePanel';
import { MapView } from '../maps/MapView';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import { useFavorites } from '../hooks/useFavorites';
import type { Route, RouteStop, Stop } from '../types';

export const RoutesPage: React.FC = () => {
  const [routes, setRoutes] = useState<Route[]>(DEHRADUN_ROUTES);
  const [stops, setStops] = useState<Stop[]>(DEHRADUN_STOPS);
  const [selectedRouteId, setSelectedRouteId] = useState<number | null>(1);
  const [routeStops, setRouteStops] = useState<RouteStop[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const { isFavorite, toggleFavorite } = useFavorites();

  const fetchRoutes = async () => {
    try {
      setLoading(true);
      setError(null);
      const [rData, sData] = await Promise.all([
        routeService.getRoutes().catch(() => DEHRADUN_ROUTES),
        stopService.getStops().catch(() => DEHRADUN_STOPS),
      ]);
      setRoutes(rData.length > 0 ? rData : DEHRADUN_ROUTES);
      setStops(sData.length > 0 ? sData : DEHRADUN_STOPS);

      if (rData.length > 0) {
        handleSelectRoute(rData[0].id);
      } else {
        handleSelectRoute(DEHRADUN_ROUTES[0].id);
      }
    } catch {
      setRoutes(DEHRADUN_ROUTES);
      setStops(DEHRADUN_STOPS);
      handleSelectRoute(DEHRADUN_ROUTES[0].id);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchRoutes();
  }, []);

  const handleSelectRoute = async (routeId: number) => {
    setSelectedRouteId(routeId);
    try {
      const stopsData = await routeService.getRouteStops(routeId);
      if (stopsData && stopsData.length > 0) {
        setRouteStops(stopsData);
        return;
      }
    } catch {
      // quiet fallback
    }

    // Local fallback
    const local = DEHRADUN_ROUTES.find((r) => r.id === routeId);
    if (local) {
      const stopsArr: RouteStop[] = local.stopIds.map((sid, idx) => {
        const found = DEHRADUN_STOPS.find((s) => s.id === sid);
        return {
          id: sid,
          route_id: routeId,
          stop_id: sid,
          stop_order: idx + 1,
          estimated_stop_duration: 30,
          stop: found,
        };
      });
      setRouteStops(stopsArr);
    }
  };

  const selectedRoute = routes.find((r) => r.id === selectedRouteId) || null;

  return (
    <div style={{ display: 'flex', height: '100%', width: '100%', overflow: 'hidden' }}>
      {/* Sidebar List */}
      <div
        style={{
          width: '380px',
          height: '100%',
          backgroundColor: 'var(--bg-secondary)',
          borderRight: '1px solid var(--border-color)',
          display: 'flex',
          flexDirection: 'column',
          flexShrink: 0,
        }}
      >
        <div style={{ padding: 'var(--space-lg)', borderBottom: '1px solid var(--border-color)' }}>
          <h1 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-bold)' }}>
            Transit Routes
          </h1>
          <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
            {routes.length} network lines configured in Dehradun
          </p>
        </div>

        <div style={{ flex: 1, overflowY: 'auto', padding: 'var(--space-md)', display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
          {loading ? (
            <LoadingSpinner text="Loading transit lines..." />
          ) : error ? (
            <EmptyState
              icon={RouteIcon}
              title="Unable to load routes"
              description={error}
              action={{ label: 'Retry', onClick: fetchRoutes }}
            />
          ) : (
            routes.map((route) => (
              <RouteCard
                key={route.id}
                route={route}
                isSelected={route.id === selectedRouteId}
                isFavorite={isFavorite(route.id)}
                onSelect={() => handleSelectRoute(route.id)}
                onToggleFavorite={() => toggleFavorite(route.id)}
              />
            ))
          )}
        </div>
      </div>

      {/* Map View */}
      <div style={{ flex: 1, position: 'relative', height: '100%' }}>
        <MapView
          stops={stops}
          selectedRouteId={selectedRouteId}
          selectedRouteStops={routeStops}
        />

        {selectedRouteId && (
          <RoutePanel
            route={selectedRoute}
            routeStops={routeStops}
            isFavorite={selectedRoute ? isFavorite(selectedRoute.id) : false}
            onToggleFavorite={() => selectedRoute && toggleFavorite(selectedRoute.id)}
            onClose={() => setSelectedRouteId(null)}
          />
        )}
      </div>
    </div>
  );
};
