import React from 'react';
import { MapPin, Navigation } from 'lucide-react';
import type { Stop } from '../types';

interface StopCardProps {
  stop: Stop;
  distanceKm?: number;
  isSelected?: boolean;
  onClick?: () => void;
}

export const StopCard: React.FC<StopCardProps> = ({
  stop,
  distanceKm,
  isSelected,
  onClick,
}) => {
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
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-xs)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <MapPin size={16} style={{ color: 'var(--accent-primary)' }} />
          <span style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
            {stop.stop_name}
          </span>
        </div>
        {distanceKm !== undefined && (
          <div style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
            <Navigation size={10} />
            <span>{distanceKm < 1 ? `${Math.round(distanceKm * 1000)}m` : `${distanceKm.toFixed(1)}km`}</span>
          </div>
        )}
      </div>

      <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
        Coordinates: {stop.latitude.toFixed(4)}, {stop.longitude.toFixed(4)}
      </div>
    </div>
  );
};
