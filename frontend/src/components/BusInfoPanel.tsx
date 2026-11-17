import React, { useEffect, useState } from 'react';
import { X, Navigation, Clock, MapPin, Gauge, Bus as BusIcon, AlertCircle } from 'lucide-react';
import type { Bus, BusLocation, EtaResult } from '../types';
import { StatusBadge } from './StatusBadge';
import { busService } from '../services/bus.service';

interface BusInfoPanelProps {
  bus: Bus | null;
  location: BusLocation | null;
  onClose: () => void;
  onViewRoute?: (routeId: number) => void;
}

export const BusInfoPanel: React.FC<BusInfoPanelProps> = ({
  bus,
  location,
  onClose,
  onViewRoute,
}) => {
  const [eta, setEta] = useState<EtaResult | null>(null);

  useEffect(() => {
    if (!bus || !location?.next_stop) {
      setEta(null);
      return;
    }
    const fetchEta = async () => {
      try {
        const res = await busService.getBusEta(bus.id, location.next_stop!.id);
        setEta(res);
      } catch {
        setEta(null);
      }
    };
    fetchEta();
  }, [bus, location?.next_stop]);

  if (!bus) return null;

  const isLive = !!(location && location.latitude && location.longitude);
  const status = isLive ? 'LIVE' : (bus.status === 'active' ? 'STALE' : 'OFFLINE');

  // Format last updated time
  const formatTime = (ts?: string) => {
    if (!ts) return 'Unknown';
    const date = new Date(ts);
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };

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
      {/* Panel Header */}
      <div className="panel-header">
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <BusIcon size={20} style={{ color: 'var(--accent-primary)' }} />
          <h2 className="panel-title">Bus {bus.bus_number}</h2>
        </div>
        <button
          type="button"
          onClick={onClose}
          style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center' }}
        >
          <X size={20} />
        </button>
      </div>

      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-lg)' }}>
        <StatusBadge status={status} />
        <span style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
          Reg: {bus.registration_number || 'N/A'}
        </span>
      </div>

      {/* Primary Metrics Grid */}
      <div className="grid-2" style={{ marginBottom: 'var(--space-lg)' }}>
        <div className="card" style={{ padding: 'var(--space-md)' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', color: 'var(--text-muted)', marginBottom: '4px' }}>
            <Gauge size={14} />
            <span style={{ fontSize: 'var(--font-size-xs)' }}>SPEED</span>
          </div>
          <div style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-bold)' }}>
            {location?.speed !== null && location?.speed !== undefined
              ? `${Math.round(location.speed)} km/h`
              : '0 km/h'}
          </div>
        </div>

        <div className="card" style={{ padding: 'var(--space-md)' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', color: 'var(--text-muted)', marginBottom: '4px' }}>
            <Clock size={14} />
            <span style={{ fontSize: 'var(--font-size-xs)' }}>NEXT STOP ETA</span>
          </div>
          <div style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-bold)', color: 'var(--accent-primary)' }}>
            {eta ? `${Math.round(eta.eta_minutes)} min` : location?.distance_to_next_stop ? '~4 min' : '--'}
          </div>
        </div>
      </div>

      {/* Next Stop Info */}
      <div className="card" style={{ marginBottom: 'var(--space-lg)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', color: 'var(--text-muted)', marginBottom: 'var(--space-sm)' }}>
          <MapPin size={14} />
          <span style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)' }}>NEXT SCHEDULED STOP</span>
        </div>
        <div style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)', marginBottom: 'var(--space-xs)' }}>
          {location?.next_stop?.stop_name || 'Approaching Route Stop'}
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)' }}>
          <Navigation size={12} />
          <span>
            Distance: {location?.distance_to_next_stop ? `${location.distance_to_next_stop.toFixed(1)} km` : eta ? `${eta.distance_km.toFixed(1)} km` : '1.2 km'}
          </span>
        </div>
      </div>

      {/* GPS Coordinates & Technical Telemetry */}
      <div className="card" style={{ marginBottom: 'var(--space-lg)' }}>
        <div style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-muted)', marginBottom: 'var(--space-sm)' }}>
          LIVE TELEMETRY
        </div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-xs)', fontSize: 'var(--font-size-xs)' }}>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Coordinates:</span>
            <span>{location ? `${location.latitude.toFixed(5)}, ${location.longitude.toFixed(5)}` : 'No Signal'}</span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Heading:</span>
            <span>{location?.heading !== null && location?.heading !== undefined ? `${Math.round(location.heading)}°` : 'N/A'}</span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>GPS Accuracy:</span>
            <span>{location?.accuracy ? `±${Math.round(location.accuracy)} m` : '< 15 m'}</span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Last Signal:</span>
            <span>{formatTime(location?.updated_at)}</span>
          </div>
        </div>
      </div>

      {/* Active Route info */}
      {location?.route && (
        <div style={{ marginTop: 'auto' }}>
          <button
            type="button"
            className="btn btn-primary w-full"
            onClick={() => onViewRoute?.(location.route!.id)}
          >
            View Route Details
          </button>
        </div>
      )}
    </div>
  );
};
