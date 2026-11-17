import React, { useState, useEffect } from 'react';
import { Route as RouteIcon } from 'lucide-react';
import { routeService } from '../services/route.service';
import { stopService } from '../services/stop.service';
import { RouteCard } from '../components/RouteCard';
import { RoutePanel } from '../components/RoutePanel';
import { MapView } from '../maps/MapView';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import { useFavorites } from '../hooks/useFavorites';
import type { Route, RouteStop, Stop } from '../types';

export const RoutesPage: React.FC = () => {
  const [routes, setRoutes] = useState<Route[]>([]);
  const [stops, setStops] = useState<Stop[]>([]);
  const [selectedRouteId, setSelectedRouteId] = useState<number | null>(null);
  const [routeStops, setRouteStops] = useState<RouteStop[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const { isFavorite, toggleFavorite } = useFavorites();

  const fetchRoutes = async () => {
    try {
      setLoading(true);
      setError(null);
      const [rData, sData] = await Promise.all([
        routeService.getRoutes(),
        stopService.getStops(),
      ]);
      setRoutes(rData);
      setStops(sData);

      // Select first route by default if available
      if (rData.length > 0) {
        handleSelectRoute(rData[0].id);
      }
    } catch (err: any) {
      setError(err?.message || 'Failed to load transit routes');
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
      setRouteStops(stopsData);
    } catch {
      setRouteStops([]);
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
            {routes.length} network lines configured
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
          ) : routes.length === 0 ? (
            <EmptyState
              icon={RouteIcon}
              title="No routes found"
              description="No public transit routes available."
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
