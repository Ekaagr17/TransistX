import React from 'react';
import { X, MapPin, Route as RouteIcon } from 'lucide-react';
import type { Route, RouteStop } from '../types';
import { FavoriteButton } from './FavoriteButton';

interface RoutePanelProps {
  route: Route | null;
  routeStops: RouteStop[];
  isFavorite?: boolean;
  onToggleFavorite?: () => void;
  onClose: () => void;
  onSelectStop?: (stopId: number) => void;
}

export const RoutePanel: React.FC<RoutePanelProps> = ({
  route,
  routeStops,
  isFavorite = false,
  onToggleFavorite,
  onClose,
  onSelectStop,
}) => {
  if (!route) return null;

  const sortedStops = [...routeStops].sort((a, b) => a.stop_order - b.stop_order);

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
          <h2 className="panel-title" style={{ fontSize: 'var(--font-size-md)' }}>
            {route.route_name}
          </h2>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)' }}>
          {onToggleFavorite && (
            <FavoriteButton isFavorite={isFavorite} onToggle={onToggleFavorite} />
          )}
          <button
            type="button"
            onClick={onClose}
            style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center' }}
          >
            <X size={20} />
          </button>
        </div>
      </div>

      <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', fontSize: 'var(--font-size-sm)', color: 'var(--text-secondary)', marginBottom: 'var(--space-lg)' }}>
        <MapPin size={14} style={{ color: 'var(--text-muted)' }} />
        <span>{route.start_point || 'Origin'} → {route.end_point || 'Destination'}</span>
      </div>

      {/* Stops Sequence */}
      <div style={{ fontSize: 'var(--font-size-xs)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-muted)', marginBottom: 'var(--space-md)' }}>
        SCHEDULED STOPS ({sortedStops.length})
      </div>

      <div style={{ flex: 1, overflowY: 'auto', display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
        {sortedStops.length === 0 ? (
          <div style={{ color: 'var(--text-muted)', fontSize: 'var(--font-size-sm)' }}>
            No stops defined for this route.
          </div>
        ) : (
          sortedStops.map((rs, idx) => (
            <div
              key={rs.id}
              onClick={() => onSelectStop?.(rs.stop_id)}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: 'var(--space-md)',
                padding: 'var(--space-sm) var(--space-md)',
                backgroundColor: 'var(--bg-surface)',
                border: '1px solid var(--border-color)',
                borderRadius: 'var(--radius-md)',
                cursor: 'pointer',
              }}
            >
              <div
                style={{
                  width: '24px',
                  height: '24px',
                  borderRadius: '50%',
                  backgroundColor: 'var(--bg-elevated)',
                  border: '1px solid var(--border-color)',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  fontSize: 'var(--font-size-xs)',
                  fontWeight: 'var(--font-weight-bold)',
                  color: 'var(--accent-primary)',
                  flexShrink: 0,
                }}
              >
                {rs.stop_order}
              </div>
              <div style={{ flex: 1, overflow: 'hidden' }}>
                <div style={{ fontSize: 'var(--font-size-sm)', fontWeight: 'var(--font-weight-medium)', color: 'var(--text-primary)' }} className="truncate">
                  {rs.stop?.stop_name || `Stop #${rs.stop_id}`}
                </div>
                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                  Est. dwell: {rs.estimated_stop_duration || 30}s
                </div>
              </div>
            </div>
          ))
        )}
      </div>
    </div>
  );
};
