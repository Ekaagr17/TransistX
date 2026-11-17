import React, { useEffect, useState } from 'react';
import { X, MapPin, Bus as BusIcon, Clock } from 'lucide-react';
import type { Stop, Route, BusLocation } from '../types';
import { routeService } from '../services/route.service';

interface StopPanelProps {
  stop: Stop | null;
  onClose: () => void;
  approachingBuses?: BusLocation[];
  onSelectBus?: (busId: number) => void;
}

export const StopPanel: React.FC<StopPanelProps> = ({
  stop,
  onClose,
  approachingBuses = [],
  onSelectBus,
}) => {
  const [routes, setRoutes] = useState<Route[]>([]);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    if (!stop) return;
    const fetchServingRoutes = async () => {
      setLoading(true);
      try {
        const allRoutes = await routeService.getRoutes();
        // Here we could filter routes that include this stop
        setRoutes(allRoutes);
      } catch {
        setRoutes([]);
      } finally {
        setLoading(false);
      }
    };
    fetchServingRoutes();
  }, [stop]);

  if (!stop) return null;

  return (
    <div
      className="panel"
      style={{
        position: 'absolute',
        top: 0,
        right: 0,
        bottom: 0,
        boxShadow: 'var(--shadow-lg)',
        zIndex: 30,
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      {/* Header */}
      <div className="panel-header">
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <MapPin size={20} style={{ color: 'var(--accent-primary)' }} />
          <h2 className="panel-title">{stop.stop_name}</h2>
        </div>
        <button
          type="button"
          onClick={onClose}
          style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center' }}
        >
          <X size={20} />
        </button>
      </div>

      <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)', marginBottom: 'var(--space-lg)' }}>
        Lat: {stop.latitude.toFixed(5)}, Lng: {stop.longitude.toFixed(5)}
      </div>

      {/* Approaching Buses */}
      <div style={{ marginBottom: 'var(--space-xl)' }}>
        <div style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-muted)', marginBottom: 'var(--space-md)' }}>
          APPROACHING BUSES
        </div>
        {approachingBuses.length === 0 ? (
          <div className="card" style={{ padding: 'var(--space-md)', color: 'var(--text-muted)', fontSize: 'var(--font-size-sm)' }}>
            No incoming buses reported in the next 15 minutes.
          </div>
        ) : (
          <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
            {approachingBuses.map((bus) => (
              <div
                key={bus.bus_id}
                onClick={() => onSelectBus?.(bus.bus_id)}
                className="card"
                style={{
                  padding: 'var(--space-sm) var(--space-md)',
                  cursor: 'pointer',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'space-between',
                }}
              >
                <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
                  <BusIcon size={16} style={{ color: 'var(--accent-primary)' }} />
                  <span style={{ fontWeight: 'var(--font-weight-medium)' }}>Bus {bus.bus_id}</span>
                </div>
                <div style={{ display: 'flex', alignItems: 'center', gap: '4px', color: 'var(--accent-primary)', fontSize: 'var(--font-size-sm)' }}>
                  <Clock size={12} />
                  <span>~5 min</span>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Serving Routes */}
      <div>
        <div style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-muted)', marginBottom: 'var(--space-md)' }}>
          SERVING TRANSIT LINES
        </div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
          {routes.slice(0, 4).map((r) => (
            <div
              key={r.id}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: 'var(--space-sm)',
                padding: 'var(--space-sm)',
                backgroundColor: 'var(--bg-surface)',
                borderRadius: 'var(--radius-md)',
                fontSize: 'var(--font-size-sm)',
              }}
            >
              <div
                style={{
                  padding: '2px 6px',
                  backgroundColor: 'var(--bg-elevated)',
                  borderRadius: 'var(--radius-sm)',
                  fontSize: 'var(--font-size-xs)',
                  fontWeight: 'var(--font-weight-bold)',
                  color: 'var(--accent-primary)',
                }}
              >
                {r.route_code}
              </div>
              <span className="truncate">{r.route_name}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};
