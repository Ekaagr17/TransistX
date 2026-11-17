import React, { useState } from 'react';
import { Bus as BusIcon, RefreshCw } from 'lucide-react';
import { useBuses } from '../hooks/useBuses';
import { BusCard } from '../components/BusCard';
import { MapView } from '../maps/MapView';
import { BusInfoPanel } from '../components/BusInfoPanel';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import type { BusLocation } from '../types';

export const LiveBusesPage: React.FC = () => {
  const { buses, locations, loading, error, refetch } = useBuses();
  const [selectedBusId, setSelectedBusId] = useState<number | null>(null);

  const busLocationsList: BusLocation[] = Array.from(locations.values());
  const selectedBus = buses.find((b) => b.id === selectedBusId) || null;
  const selectedLocation = selectedBusId ? locations.get(selectedBusId) || null : null;

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
              {buses.length} registered buses ({busLocationsList.length} active)
            </p>
          </div>
          <button
            type="button"
            className="btn btn-sm"
            onClick={refetch}
            title="Refresh buses"
          >
            <RefreshCw size={14} />
          </button>
        </div>

        <div style={{ flex: 1, overflowY: 'auto', padding: 'var(--space-md)', display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
          {loading ? (
            <LoadingSpinner text="Connecting to fleet telemetry..." />
          ) : error ? (
            <EmptyState
              icon={BusIcon}
              title="Unable to load fleet"
              description={error}
              action={{ label: 'Retry', onClick: refetch }}
            />
          ) : buses.length === 0 ? (
            <EmptyState
              icon={BusIcon}
              title="No buses found"
              description="No transit buses registered in the system."
            />
          ) : (
            buses.map((bus) => (
              <BusCard
                key={bus.id}
                bus={bus}
                location={locations.get(bus.id)}
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
          busLocations={busLocationsList}
          selectedBusId={selectedBusId}
          onBusSelect={(id) => setSelectedBusId(id)}
        />

        {selectedBusId && (
          <BusInfoPanel
            bus={selectedBus}
            location={selectedLocation}
            onClose={() => setSelectedBusId(null)}
          />
        )}
      </div>
    </div>
  );
};
