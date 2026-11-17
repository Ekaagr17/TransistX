import React from 'react';
import { Route as RouteIcon, MapPin } from 'lucide-react';
import type { Route } from '../types';
import { FavoriteButton } from './FavoriteButton';

interface RouteCardProps {
  route: Route;
  isSelected?: boolean;
  isFavorite?: boolean;
  onSelect?: () => void;
  onToggleFavorite?: (e: React.MouseEvent) => void;
}

export const RouteCard: React.FC<RouteCardProps> = ({
  route,
  isSelected,
  isFavorite = false,
  onSelect,
  onToggleFavorite,
}) => {
  return (
    <div
      onClick={onSelect}
      className={`card ${isSelected ? 'card-elevated' : ''}`}
      style={{
        cursor: 'pointer',
        borderColor: isSelected ? 'var(--accent-primary)' : 'var(--border-color)',
        transition: 'all 0.2s ease',
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-sm)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <div
            style={{
              padding: '2px 6px',
              backgroundColor: 'var(--bg-surface)',
              border: '1px solid var(--border-color)',
              borderRadius: 'var(--radius-sm)',
              fontSize: 'var(--font-size-xs)',
              fontWeight: 'var(--font-weight-bold)',
              color: 'var(--accent-primary)',
            }}
          >
            {route.route_code}
          </div>
          <span style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
            {route.route_name}
          </span>
        </div>
        {onToggleFavorite && (
          <FavoriteButton isFavorite={isFavorite} onToggle={onToggleFavorite} />
        )}
      </div>

      <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', fontSize: 'var(--font-size-sm)', color: 'var(--text-secondary)' }}>
        <MapPin size={14} style={{ color: 'var(--text-muted)', flexShrink: 0 }} />
        <span className="truncate">
          {route.start_point || 'Start'} → {route.end_point || 'Destination'}
        </span>
      </div>
    </div>
  );
};
