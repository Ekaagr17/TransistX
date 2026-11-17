import React, { useState, useEffect } from 'react';
import { Bus as BusIcon, Route as RouteIcon, MapPin, Compass, Sparkles } from 'lucide-react';
import { MapView } from '../maps/MapView';
import { StatCard } from '../components/StatCard';
import { BusInfoPanel } from '../components/BusInfoPanel';
import { RoutePanel } from '../components/RoutePanel';
import { StopPanel } from '../components/StopPanel';
import { JourneyPlannerModal } from '../components/JourneyPlannerModal';
import { useBuses } from '../hooks/useBuses';
import { useFavorites } from '../hooks/useFavorites';
import { stopService } from '../services/stop.service';
import { routeService } from '../services/route.service';
import { DEHRADUN_STOPS, DEHRADUN_ROUTES, FLEET_BUSES } from '../services/journey.service';
import type { Stop, Route, RouteStop, BusLocation, JourneyCandidate } from '../types';

export const DashboardPage: React.FC = () => {
  const { buses: apiBuses, locations, loading: busesLoading } = useBuses();
  const { isFavorite, toggleFavorite } = useFavorites();
  const [stops, setStops] = useState<Stop[]>(DEHRADUN_STOPS);
  const [routes, setRoutes] = useState<Route[]>(DEHRADUN_ROUTES);

  const [selectedBusId, setSelectedBusId] = useState<number | null>(null);
  const [selectedStopId, setSelectedStopId] = useState<number | null>(null);
  const [selectedRouteId, setSelectedRouteId] = useState<number | null>(null);
  const [selectedRouteStops, setSelectedRouteStops] = useState<RouteStop[]>([]);

  // Journey planner modal state
  const [isJourneyPlannerOpen, setIsJourneyPlannerOpen] = useState(false);
  const [journeyPlannerPreselectedBusId, setJourneyPlannerPreselectedBusId] = useState<number | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [stopsData, routesData] = await Promise.all([
          stopService.getStops().catch(() => DEHRADUN_STOPS),
          routeService.getRoutes().catch(() => DEHRADUN_ROUTES),
        ]);
        if (stopsData && stopsData.length > 0) setStops(stopsData);
        if (routesData && routesData.length > 0) setRoutes(routesData);
      } catch {
        // Fallback to local 29 stops and 5 routes
        setStops(DEHRADUN_STOPS);
        setRoutes(DEHRADUN_ROUTES);
      }
    };
    fetchData();
  }, []);

  const handleSelectBus = (busId: number) => {
    setSelectedBusId(busId);
    setSelectedStopId(null);

    // Auto-select corresponding route for this bus if known
    const busConfig = FLEET_BUSES.find((b) => b.id === busId);
    if (busConfig) {
      handleSelectRoute(busConfig.route_id);
    }
  };

  const handleSelectStop = (stopId: number) => {
    setSelectedStopId(stopId);
    setSelectedBusId(null);
    setSelectedRouteId(null);
  };

  const handleSelectRoute = async (routeId: number) => {
    setSelectedRouteId(routeId);
    setSelectedStopId(null);

    try {
      const stopsData = await routeService.getRouteStops(routeId);
      if (stopsData && stopsData.length > 0) {
        setSelectedRouteStops(stopsData);
        return;
      }
    } catch {
      // quiet fallback
    }

    // Fallback to local route stops
    const localRoute = DEHRADUN_ROUTES.find((r) => r.id === routeId);
    if (localRoute) {
      const generatedStops: RouteStop[] = localRoute.stopIds.map((sid, idx) => {
        const foundStop = DEHRADUN_STOPS.find((s) => s.id === sid);
        return {
          id: sid,
          route_id: routeId,
          stop_id: sid,
          stop_order: idx + 1,
          estimated_stop_duration: 30,
          stop: foundStop,
        };
      });
      setSelectedRouteStops(generatedStops);
    }
  };

  // When a bus is picked from the Journey Planner
  const handleSelectFromJourneyPlanner = (candidate: JourneyCandidate) => {
    setSelectedBusId(candidate.bus_id);
    handleSelectRoute(candidate.route_id);
  };

  const handleOpenJourneyPlanner = (busId?: number) => {
    setJourneyPlannerPreselectedBusId(busId || null);
    setIsJourneyPlannerOpen(true);
  };

  // Convert map locations to array or fallback to all 16 fleet buses with simulated coordinates
  const busLocationList: BusLocation[] = Array.from(locations.values());
  const effectiveBusLocations: BusLocation[] =
    busLocationList.length > 0
      ? busLocationList
      : FLEET_BUSES.map((fb, idx) => {
          const route = DEHRADUN_ROUTES.find((r) => r.id === fb.route_id) || DEHRADUN_ROUTES[0];
          const firstStopId = route.stopIds[idx % route.stopIds.length];
          const stop = DEHRADUN_STOPS.find((s) => s.id === firstStopId) || DEHRADUN_STOPS[0];

          return {
            bus_id: fb.id,
            latitude: stop.latitude + 0.001 * (idx % 2 === 0 ? 1 : -1),
            longitude: stop.longitude + 0.001 * (idx % 2 === 0 ? -1 : 1),
            speed: 24,
            heading: 120 + idx * 20,
            accuracy: 8,
            updated_at: new Date().toISOString(),
            bus: {
              id: fb.id,
              bus_number: fb.bus_number,
              registration_number: fb.registration_number,
              capacity: fb.capacity,
              status: 'active',
              created_at: new Date().toISOString(),
            },
            route: route,
            next_stop: stop,
            distance_to_next_stop: 0.8,
          };
        });

  // Effective buses list
  const allBuses =
    apiBuses.length >= FLEET_BUSES.length
      ? apiBuses
      : FLEET_BUSES.map((fb) => ({
          id: fb.id,
          bus_number: fb.bus_number,
          registration_number: fb.registration_number,
          capacity: fb.capacity,
          status: 'active' as const,
          created_at: new Date().toISOString(),
        }));

  const activeBusesCount = effectiveBusLocations.filter((b) => b.latitude && b.longitude).length;
  const selectedBus = allBuses.find((b) => b.id === selectedBusId) || null;
  const selectedBusLocation = selectedBusId
    ? effectiveBusLocations.find((b) => b.bus_id === selectedBusId) || null
    : null;
  const selectedStop = stops.find((s) => s.id === selectedStopId) || null;
  const selectedRoute = routes.find((r) => r.id === selectedRouteId) || null;

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100%', width: '100%', position: 'relative' }}>
      {/* Top Floating Stats & Action Bar */}
      <div
        style={{
          position: 'absolute',
          top: 'var(--space-md)',
          left: 'var(--space-md)',
          zIndex: 10,
          display: 'flex',
          gap: 'var(--space-md)',
          alignItems: 'center',
          pointerEvents: 'none',
          flexWrap: 'wrap',
        }}
      >
        {/* Big Smart Journey Planner CTA */}
        <button
          type="button"
          onClick={() => handleOpenJourneyPlanner()}
          className="btn btn-primary-filled"
          style={{
            pointerEvents: 'auto',
            display: 'flex',
            alignItems: 'center',
            gap: 'var(--space-sm)',
            padding: 'var(--space-sm) var(--space-lg)',
            boxShadow: 'var(--shadow-md)',
            backgroundColor: 'var(--accent-primary)',
            color: 'var(--bg-primary)',
            fontWeight: 'var(--font-weight-bold)',
          }}
        >
          <Compass size={18} />
          <span>Plan Journey / Best Bus</span>
          <span
            style={{
              padding: '2px 6px',
              backgroundColor: 'rgba(0,0,0,0.2)',
              borderRadius: 'var(--radius-sm)',
              fontSize: 'var(--font-size-xs)',
            }}
          >
            29 Stops
          </span>
        </button>

        <div style={{ pointerEvents: 'auto', minWidth: '150px' }}>
          <StatCard
            title="ACTIVE BUSES"
            value={busesLoading ? '16' : activeBusesCount || 16}
            icon={BusIcon}
            subtitle="Broadcasting live GPS"
          />
        </div>
        <div style={{ pointerEvents: 'auto', minWidth: '150px' }}>
          <StatCard
            title="TOTAL ROUTES"
            value={routes.length || 5}
            icon={RouteIcon}
            subtitle="Dehradun Network"
          />
        </div>
        <div style={{ pointerEvents: 'auto', minWidth: '150px' }}>
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
          busLocations={effectiveBusLocations}
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
            onOpenJourneyPlanner={handleOpenJourneyPlanner}
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
            approachingBuses={effectiveBusLocations.slice(0, 3)}
            onSelectBus={handleSelectBus}
          />
        )}
      </div>

      {/* Journey Planner Modal */}
      <JourneyPlannerModal
        isOpen={isJourneyPlannerOpen}
        onClose={() => setIsJourneyPlannerOpen(false)}
        preselectedBusId={journeyPlannerPreselectedBusId}
        onSelectBusAndRoute={handleSelectFromJourneyPlanner}
      />
    </div>
  );
};
