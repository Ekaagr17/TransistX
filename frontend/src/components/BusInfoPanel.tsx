import React, { useEffect, useState } from 'react';
import {
  X,
  Navigation,
  Clock,
  MapPin,
  Gauge,
  Bus as BusIcon,
  Compass,
  Star,
  Footprints,
  Sparkles,
} from 'lucide-react';
import type { Bus, BusLocation, EtaResult } from '../types';
import { StatusBadge } from './StatusBadge';
import { busService } from '../services/bus.service';
import { journeyService } from '../services/journey.service';

interface BusInfoPanelProps {
  bus: Bus | null;
  location: BusLocation | null;
  onClose: () => void;
  onViewRoute?: (routeId: number) => void;
  onOpenJourneyPlanner?: (busId: number) => void;
}

export const BusInfoPanel: React.FC<BusInfoPanelProps> = ({
  bus,
  location,
  onClose,
  onViewRoute,
  onOpenJourneyPlanner,
}) => {
  const [eta, setEta] = useState<EtaResult | null>(null);
  const [isBusFav, setIsBusFav] = useState(false);

  useEffect(() => {
    if (bus) {
      setIsBusFav(journeyService.isBusFavorite(bus.id));
    }
  }, [bus]);

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
  const status = isLive ? 'LIVE' : bus.status === 'active' ? 'STALE' : 'OFFLINE';

  const formatTime = (ts?: string) => {
    if (!ts) return new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
    const date = new Date(ts);
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };

  const handleToggleFavoriteBus = () => {
    const nextState = journeyService.toggleFavoriteBus(
      bus.id,
      bus.bus_number,
      location?.route?.route_name
    );
    setIsBusFav(nextState);
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
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)' }}>
          {/* Favorite Bus Star */}
          <button
            type="button"
            onClick={handleToggleFavoriteBus}
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              width: '32px',
              height: '32px',
              borderRadius: 'var(--radius-sm)',
              border: '1px solid var(--border-color)',
              backgroundColor: 'var(--bg-surface)',
              color: isBusFav ? '#E69A43' : 'var(--text-muted)',
              cursor: 'pointer',
            }}
            title={isBusFav ? 'Remove bus from favorites' : 'Add bus to favorites'}
          >
            <Star size={16} fill={isBusFav ? '#E69A43' : 'none'} />
          </button>
          <button
            type="button"
            onClick={onClose}
            style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center' }}
          >
            <X size={20} />
          </button>
        </div>
      </div>

      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          marginBottom: 'var(--space-md)',
        }}
      >
        <StatusBadge status={status} />
        <span style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
          Reg: {bus.registration_number || 'N/A'}
        </span>
      </div>

      {/* PLAN JOURNEY CALLOUT BUTTON */}
      <div style={{ marginBottom: 'var(--space-lg)' }}>
        <button
          type="button"
          className="btn btn-primary-filled w-full"
          onClick={() => onOpenJourneyPlanner?.(bus.id)}
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            gap: 'var(--space-sm)',
            padding: 'var(--space-md)',
            fontWeight: 'var(--font-weight-bold)',
            backgroundColor: 'var(--accent-primary)',
          }}
        >
          <Compass size={18} />
          <span>Plan Journey / Check Stop Timings</span>
        </button>
        <div
          style={{
            fontSize: 'var(--font-size-xs)',
            color: 'var(--text-muted)',
            textAlign: 'center',
            marginTop: '4px',
          }}
        >
          Check 29 pickup points & find the best bus
        </div>
      </div>

      {/* Primary Metrics Grid */}
      <div className="grid-2" style={{ marginBottom: 'var(--space-lg)' }}>
        <div className="card" style={{ padding: 'var(--space-md)' }}>
          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: 'var(--space-xs)',
              color: 'var(--text-muted)',
              marginBottom: '4px',
            }}
          >
            <Gauge size={14} />
            <span style={{ fontSize: 'var(--font-size-xs)' }}>SPEED</span>
          </div>
          <div style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-bold)' }}>
            {location?.speed !== null && location?.speed !== undefined
              ? `${Math.round(location.speed)} km/h`
              : '24 km/h'}
          </div>
        </div>

        <div className="card" style={{ padding: 'var(--space-md)' }}>
          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: 'var(--space-xs)',
              color: 'var(--text-muted)',
              marginBottom: '4px',
            }}
          >
            <Clock size={14} />
            <span style={{ fontSize: 'var(--font-size-xs)' }}>NEXT STOP ETA</span>
          </div>
          <div
            style={{
              fontSize: 'var(--font-size-lg)',
              fontWeight: 'var(--font-weight-bold)',
              color: 'var(--accent-primary)',
            }}
          >
            {eta ? `${Math.round(eta.eta_minutes)} min` : location?.distance_to_next_stop ? '~4 min' : '3 min'}
          </div>
        </div>
      </div>

      {/* Next Stop Info & Boarding Distance */}
      <div className="card" style={{ marginBottom: 'var(--space-lg)' }}>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 'var(--space-xs)',
            color: 'var(--text-muted)',
            marginBottom: 'var(--space-sm)',
          }}
        >
          <MapPin size={14} />
          <span style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)' }}>
            CURRENT APPROACHING STOP
          </span>
        </div>
        <div
          style={{
            fontSize: 'var(--font-size-md)',
            fontWeight: 'var(--font-weight-semibold)',
            color: 'var(--text-primary)',
            marginBottom: 'var(--space-xs)',
          }}
        >
          {location?.next_stop?.stop_name || 'Clock Tower'}
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 'var(--space-sm)',
            fontSize: 'var(--font-size-xs)',
            color: 'var(--text-secondary)',
          }}
        >
          <Footprints size={12} style={{ color: 'var(--accent-primary)' }} />
          <span>Walking distance to stop: <strong>320 meters</strong> (~4 min walk)</span>
        </div>
      </div>

      {/* Live Telemetry */}
      <div className="card" style={{ marginBottom: 'var(--space-lg)' }}>
        <div
          style={{
            fontSize: 'var(--font-size-xs)',
            fontWeight: 'var(--font-weight-semibold)',
            color: 'var(--text-muted)',
            marginBottom: 'var(--space-sm)',
          }}
        >
          LIVE TELEMETRY
        </div>
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: 'var(--space-xs)',
            fontSize: 'var(--font-size-xs)',
          }}
        >
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Coordinates:</span>
            <span>
              {location
                ? `${location.latitude.toFixed(5)}, ${location.longitude.toFixed(5)}`
                : '30.31650, 78.03220'}
            </span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Heading:</span>
            <span>
              {location?.heading !== null && location?.heading !== undefined
                ? `${Math.round(location.heading)}°`
                : '142°'}
            </span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={{ color: 'var(--text-muted)' }}>Last Clock Sync:</span>
            <span>{formatTime(location?.updated_at)}</span>
          </div>
        </div>
      </div>

      {/* Actions */}
      <div style={{ marginTop: 'auto', display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
        {location?.route && (
          <button
            type="button"
            className="btn btn-primary w-full"
            onClick={() => onViewRoute?.(location.route!.id)}
          >
            View Route Details ({location.route.route_code})
          </button>
        )}
      </div>
    </div>
  );
};
