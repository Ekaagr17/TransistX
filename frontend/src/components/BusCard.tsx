import React from 'react';
import { Bus as BusIcon, Navigation } from 'lucide-react';
import type { Bus, BusLocation } from '../types';
import { StatusBadge } from './StatusBadge';

interface BusCardProps {
  bus: Bus;
  location?: BusLocation | null;
  isSelected?: boolean;
  onClick?: () => void;
}

export const BusCard: React.FC<BusCardProps> = ({
  bus,
  location,
  isSelected,
  onClick,
}) => {
  const isLive = location && location.latitude && location.longitude;
  const status = isLive ? 'LIVE' : (bus.status === 'active' ? 'STALE' : 'OFFLINE');

  return (
    <div
      onClick={onClick}
      className={`card ${isSelected ? 'card-elevated' : ''}`}
      style={{
        cursor: 'pointer',
        borderColor: isSelected ? 'var(--accent-primary)' : 'var(--border-color)',
        transition: 'all 0.2s ease',
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-sm)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <BusIcon size={18} style={{ color: 'var(--accent-primary)' }} />
          <span style={{ fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
            Bus {bus.bus_number}
          </span>
        </div>
        <StatusBadge status={status} />
      </div>

      <div style={{ fontSize: 'var(--font-size-sm)', color: 'var(--text-secondary)', marginBottom: 'var(--space-xs)' }}>
        Reg: {bus.registration_number || 'N/A'} • Capacity: {bus.capacity || 40}
      </div>

      {isLive && location ? (
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
          <Navigation size={12} style={{ color: 'var(--accent-primary)' }} />
          <span>Speed: {location.speed ? `${Math.round(location.speed)} km/h` : '0 km/h'}</span>
          <span>•</span>
          <span>Next: {location.next_stop?.stop_name || 'Approaching'}</span>
        </div>
      ) : (
        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
          No active GPS stream
        </div>
      )}
    </div>
  );
};
