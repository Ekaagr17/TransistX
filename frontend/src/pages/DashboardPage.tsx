import React, { useState, useEffect } from 'react';
import { Bus as BusIcon, Route as RouteIcon, MapPin, Radio } from 'lucide-react';
import { MapView } from '../maps/MapView';
import { StatCard } from '../components/StatCard';
import { BusInfoPanel } from '../components/BusInfoPanel';
import { RoutePanel } from '../components/RoutePanel';
import { StopPanel } from '../components/StopPanel';
import { useBuses } from '../hooks/useBuses';
import { useFavorites } from '../hooks/useFavorites';
import { stopService } from '../services/stop.service';
import { routeService } from '../services/route.service';
import type { Stop, Route, RouteStop, BusLocation } from '../types';

export const DashboardPage: React.FC = () => {
  const { buses, locations, loading: busesLoading } = useBuses();
  const { isFavorite, toggleFavorite } = useFavorites();
  const [stops, setStops] = useState<Stop[]>([]);
  const [routes, setRoutes] = useState<Route[]>([]);
  
  const [selectedBusId, setSelectedBusId] = useState<number | null>(null);
  const [selectedStopId, setSelectedStopId] = useState<number | null>(null);
  const [selectedRouteId, setSelectedRouteId] = useState<number | null>(null);
  const [selectedRouteStops, setSelectedRouteStops] = useState<RouteStop[]>([]);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [stopsData, routesData] = await Promise.all([
          stopService.getStops(),
          routeService.getRoutes(),
        ]);
        setStops(stopsData);
        setRoutes(routesData);
      } catch {
        // quiet error
      }
    };
    fetchData();
  }, []);

  const handleSelectBus = (busId: number) => {
    setSelectedBusId(busId);
    setSelectedStopId(null);
    setSelectedRouteId(null);
  };

  const handleSelectStop = (stopId: number) => {
    setSelectedStopId(stopId);
    setSelectedBusId(null);
    setSelectedRouteId(null);
  };

  const handleSelectRoute = async (routeId: number) => {
    setSelectedRouteId(routeId);
    setSelectedBusId(null);
    setSelectedStopId(null);
    try {
      const stopsData = await routeService.getRouteStops(routeId);
      setSelectedRouteStops(stopsData);
    } catch {
      setSelectedRouteStops([]);
    }
  };

  // Convert map locations to array
  const busLocationList: BusLocation[] = Array.from(locations.values());
  const activeBusesCount = busLocationList.filter((b) => b.latitude && b.longitude).length || buses.filter(b => b.status === 'active').length;

  const selectedBus = buses.find((b) => b.id === selectedBusId) || null;
  const selectedBusLocation = selectedBusId ? locations.get(selectedBusId) || null : null;
  const selectedStop = stops.find((s) => s.id === selectedStopId) || null;
  const selectedRoute = routes.find((r) => r.id === selectedRouteId) || null;

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100%', width: '100%', position: 'relative' }}>
      {/* Top Floating Stats Bar */}
      <div
        style={{
          position: 'absolute',
          top: 'var(--space-md)',
          left: 'var(--space-md)',
          zIndex: 10,
          display: 'flex',
          gap: 'var(--space-md)',
          pointerEvents: 'none',
        }}
      >
        <div style={{ pointerEvents: 'auto', minWidth: '160px' }}>
          <StatCard
            title="ACTIVE BUSES"
            value={busesLoading ? '--' : activeBusesCount}
            icon={BusIcon}
            subtitle="Broadcasting live GPS"
          />
        </div>
        <div style={{ pointerEvents: 'auto', minWidth: '160px' }}>
          <StatCard
            title="TOTAL ROUTES"
            value={routes.length || 5}
            icon={RouteIcon}
            subtitle="Dehradun Network"
          />
        </div>
        <div style={{ pointerEvents: 'auto', minWidth: '160px' }}>
          <StatCard
            title="TRANSIT STOPS"
            value={stops.length || 29}
            icon={MapPin}
            subtitle="Geo-mapped stops"
          />
        </div>
      </div>

      {/* Main Map Area */}
      <div style={{ flex: 1, position: 'relative', width: '100%', height: '100%' }}>
        <MapView
          busLocations={busLocationList}
          stops={stops}
          selectedBusId={selectedBusId}
          selectedStopId={selectedStopId}
          selectedRouteId={selectedRouteId}
          selectedRouteStops={selectedRouteStops}
          onBusSelect={handleSelectBus}
          onStopSelect={handleSelectStop}
        />

        {/* Right Detail Panels */}
        {selectedBusId && (
          <BusInfoPanel
            bus={selectedBus}
            location={selectedBusLocation}
            onClose={() => setSelectedBusId(null)}
            onViewRoute={handleSelectRoute}
          />
        )}

        {selectedRouteId && (
          <RoutePanel
            route={selectedRoute}
            routeStops={selectedRouteStops}
            isFavorite={selectedRoute ? isFavorite(selectedRoute.id) : false}
            onToggleFavorite={() => selectedRoute && toggleFavorite(selectedRoute.id)}
            onClose={() => setSelectedRouteId(null)}
            onSelectStop={handleSelectStop}
          />
        )}

        {selectedStopId && (
          <StopPanel
            stop={selectedStop}
            onClose={() => setSelectedStopId(null)}
            approachingBuses={busLocationList.slice(0, 3)}
            onSelectBus={handleSelectBus}
          />
        )}
      </div>
    </div>
  );
};
