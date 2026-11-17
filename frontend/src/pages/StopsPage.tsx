import React, { useState, useEffect } from 'react';
import { MapPin, Navigation } from 'lucide-react';
import { stopService } from '../services/stop.service';
import { StopCard } from '../components/StopCard';
import { StopPanel } from '../components/StopPanel';
import { MapView } from '../maps/MapView';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import type { Stop } from '../types';

export const StopsPage: React.FC = () => {
  const [stops, setStops] = useState<Stop[]>([]);
  const [selectedStopId, setSelectedStopId] = useState<number | null>(null);
  const [loading, setLoading] = useState(true);
  const [nearbyLoading, setNearbyLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const fetchStops = async () => {
    try {
      setLoading(true);
      setError(null);
      const data = await stopService.getStops();
      setStops(data);
    } catch (err: any) {
      setError(err?.message || 'Failed to load transit stops');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchStops();
  }, []);

  const handleLocateNearby = () => {
    if (!navigator.geolocation) {
      alert('Geolocation is not supported by your browser');
      return;
    }
    setNearbyLoading(true);
    navigator.geolocation.getCurrentPosition(
      async (pos) => {
        try {
          const nearby = await stopService.getNearbyStops(
            pos.coords.latitude,
            pos.coords.longitude,
            5 // 5km
          );
          setStops(nearby);
        } catch {
          // fallback to all stops
          fetchStops();
        } finally {
          setNearbyLoading(false);
        }
      },
      () => {
        setNearbyLoading(false);
        alert('Could not retrieve your location.');
      }
    );
  };

  const selectedStop = stops.find((s) => s.id === selectedStopId) || null;

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
              Transit Stops
            </h1>
            <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
              {stops.length} stations in Dehradun
            </p>
          </div>
          <button
            type="button"
            className="btn btn-sm btn-primary"
            onClick={handleLocateNearby}
            disabled={nearbyLoading}
          >
            <Navigation size={14} />
            <span>{nearbyLoading ? 'Locating...' : 'Nearby'}</span>
          </button>
        </div>

        <div style={{ flex: 1, overflowY: 'auto', padding: 'var(--space-md)', display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
          {loading ? (
            <LoadingSpinner text="Loading transit stops..." />
          ) : error ? (
            <EmptyState
              icon={MapPin}
              title="Unable to load stops"
              description={error}
              action={{ label: 'Retry', onClick: fetchStops }}
            />
          ) : stops.length === 0 ? (
            <EmptyState
              icon={MapPin}
              title="No stops found"
              description="No stops in this area."
              action={{ label: 'View All Stops', onClick: fetchStops }}
            />
          ) : (
            stops.map((stop) => (
              <StopCard
                key={stop.id}
                stop={stop}
                isSelected={stop.id === selectedStopId}
                onClick={() => setSelectedStopId(stop.id)}
              />
            ))
          )}
        </div>
      </div>

      {/* Map View */}
      <div style={{ flex: 1, position: 'relative', height: '100%' }}>
        <MapView
          stops={stops}
          selectedStopId={selectedStopId}
          onStopSelect={(id) => setSelectedStopId(id)}
        />

        {selectedStopId && (
          <StopPanel
            stop={selectedStop}
            onClose={() => setSelectedStopId(null)}
          />
        )}
      </div>
    </div>
  );
};
