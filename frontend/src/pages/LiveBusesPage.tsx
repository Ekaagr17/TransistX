import React, { useState } from 'react';
import { Bus as BusIcon, RefreshCw, Compass } from 'lucide-react';
import { useBuses } from '../hooks/useBuses';
import { BusCard } from '../components/BusCard';
import { MapView } from '../maps/MapView';
import { BusInfoPanel } from '../components/BusInfoPanel';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import { JourneyPlannerModal } from '../components/JourneyPlannerModal';
import { FLEET_BUSES, DEHRADUN_ROUTES, DEHRADUN_STOPS } from '../services/journey.service';
import type { BusLocation, JourneyCandidate } from '../types';

export const LiveBusesPage: React.FC = () => {
  const { buses: apiBuses, locations, loading, error, refetch } = useBuses();
  const [selectedBusId, setSelectedBusId] = useState<number | null>(null);

  // Journey planner modal state
  const [isJourneyPlannerOpen, setIsJourneyPlannerOpen] = useState(false);
  const [journeyPlannerBusId, setJourneyPlannerBusId] = useState<number | null>(null);

  // Fleet fallback ensuring all 16 buses are available
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

  // Bus locations
  const busLocationsList: BusLocation[] = Array.from(locations.values());
  const effectiveLocations: BusLocation[] =
    busLocationsList.length > 0
      ? busLocationsList
      : FLEET_BUSES.map((fb, idx) => {
          const route = DEHRADUN_ROUTES.find((r) => r.id === fb.route_id) || DEHRADUN_ROUTES[0];
          const stop = DEHRADUN_STOPS.find((s) => s.id === route.stopIds[idx % route.stopIds.length]) || DEHRADUN_STOPS[0];
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

  const selectedBus = allBuses.find((b) => b.id === selectedBusId) || null;
  const selectedLocation = selectedBusId
    ? effectiveLocations.find((b) => b.bus_id === selectedBusId) || null
    : null;

  const handleOpenJourneyPlanner = (busId?: number) => {
    setJourneyPlannerBusId(busId || null);
    setIsJourneyPlannerOpen(true);
  };

  const handleSelectFromJourneyPlanner = (candidate: JourneyCandidate) => {
    setSelectedBusId(candidate.bus_id);
  };

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
        <div
          style={{
            padding: 'var(--space-lg)',
            borderBottom: '1px solid var(--border-color)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
          }}
        >
          <div>
            <h1 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-bold)' }}>
              Live Bus Fleet
            </h1>
            <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
              {allBuses.length} active fleet vehicles in Dehradun
            </p>
          </div>
          <div style={{ display: 'flex', gap: 'var(--space-xs)' }}>
            <button
              type="button"
              className="btn btn-sm btn-primary"
              onClick={() => handleOpenJourneyPlanner()}
              title="Plan Journey"
            >
              <Compass size={14} />
            </button>
            <button
              type="button"
              className="btn btn-sm"
              onClick={refetch}
              title="Refresh buses"
            >
              <RefreshCw size={14} />
            </button>
          </div>
        </div>

        <div
          style={{
            flex: 1,
            overflowY: 'auto',
            padding: 'var(--space-md)',
            display: 'flex',
            flexDirection: 'column',
            gap: 'var(--space-sm)',
          }}
        >
          {loading && allBuses.length === 0 ? (
            <LoadingSpinner text="Connecting to fleet telemetry..." />
          ) : error && allBuses.length === 0 ? (
            <EmptyState
              icon={BusIcon}
              title="Unable to load fleet"
              description={error}
              action={{ label: 'Retry', onClick: refetch }}
            />
          ) : (
            allBuses.map((bus) => (
              <BusCard
                key={bus.id}
                bus={bus}
                location={effectiveLocations.find((l) => l.bus_id === bus.id)}
                isSelected={bus.id === selectedBusId}
                onClick={() => setSelectedBusId(bus.id)}
              />
            ))
          )}
        </div>
      </div>

      {/* Map View */}
      <div style={{ flex: 1, position: 'relative', height: '100%' }}>
        <MapView
          busLocations={effectiveLocations}
          selectedBusId={selectedBusId}
          onBusSelect={(id) => setSelectedBusId(id)}
        />

        {selectedBusId && (
          <BusInfoPanel
            bus={selectedBus}
            location={selectedLocation}
            onClose={() => setSelectedBusId(null)}
            onOpenJourneyPlanner={handleOpenJourneyPlanner}
          />
        )}
      </div>

      <JourneyPlannerModal
        isOpen={isJourneyPlannerOpen}
        onClose={() => setIsJourneyPlannerOpen(false)}
        preselectedBusId={journeyPlannerBusId}
        onSelectBusAndRoute={handleSelectFromJourneyPlanner}
      />
    </div>
  );
};
