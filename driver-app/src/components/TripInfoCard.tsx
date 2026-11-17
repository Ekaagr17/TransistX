import React from 'react';
import { Bus, MapPin, Clock, Route as RouteIcon, Send } from 'lucide-react';

interface TripInfoCardProps {
  busNumber?: string;
  routeName?: string;
  duration: string;
  distanceKm: number;
  updatesSent: number;
}

export const TripInfoCard: React.FC<TripInfoCardProps> = ({
  busNumber = 'Not Assigned',
  routeName = 'Not Assigned',
  duration,
  distanceKm,
  updatesSent,
}) => {
  return (
    <div
      style={{
        backgroundColor: '#1E242A',
        border: '1px solid #39434B',
        borderRadius: '12px',
        padding: '16px',
        display: 'flex',
        flexDirection: 'column',
        gap: '14px',
        width: '100%',
      }}
    >
      <div
        style={{
          borderBottom: '1px solid #2C343B',
          paddingBottom: '10px',
        }}
      >
        <span
          style={{
            fontSize: '0.75rem',
            fontWeight: 600,
            textTransform: 'uppercase',
            letterSpacing: '0.06em',
            color: '#74808A',
            display: 'flex',
            alignItems: 'center',
            gap: '6px',
          }}
        >
          <RouteIcon size={14} color="#25A18E" />
          Assignment & Metrics
        </span>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '10px' }}>
        <div
          style={{
            backgroundColor: '#252C32',
            padding: '12px',
            borderRadius: '8px',
            border: '1px solid #2C343B',
          }}
        >
          <div
            style={{
              fontSize: '0.7rem',
              color: '#74808A',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              marginBottom: '4px',
              display: 'flex',
              alignItems: 'center',
              gap: '4px',
            }}
          >
            <Bus size={12} color="#25A18E" />
            Assigned Bus
          </div>
          <div style={{ fontSize: '1rem', fontWeight: 700, color: '#E5E9EC', fontFamily: 'monospace' }}>
            {busNumber}
          </div>
        </div>

        <div
          style={{
            backgroundColor: '#252C32',
            padding: '12px',
            borderRadius: '8px',
            border: '1px solid #2C343B',
          }}
        >
          <div
            style={{
              fontSize: '0.7rem',
              color: '#74808A',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              marginBottom: '4px',
              display: 'flex',
              alignItems: 'center',
              gap: '4px',
            }}
          >
            <MapPin size={12} color="#E69A43" />
            Route
          </div>
          <div
            style={{
              fontSize: '0.85rem',
              fontWeight: 700,
              color: '#E5E9EC',
              whiteSpace: 'nowrap',
              overflow: 'hidden',
              textOverflow: 'ellipsis',
            }}
            title={routeName}
          >
            {routeName}
          </div>
        </div>
      </div>

      <div
        style={{
          display: 'grid',
          gridTemplateColumns: '1fr 1fr 1fr',
          gap: '8px',
          backgroundColor: '#171B20',
          border: '1px solid #2C343B',
          borderRadius: '8px',
          padding: '12px',
        }}
      >
        <div style={{ textAlign: 'center' }}>
          <div
            style={{
              fontSize: '0.65rem',
              color: '#74808A',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              marginBottom: '4px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '3px',
            }}
          >
            <Clock size={11} color="#9AA5AD" />
            Duration
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: '#E5E9EC', fontFamily: 'monospace' }}>
            {duration}
          </div>
        </div>

        <div style={{ textAlign: 'center', borderLeft: '1px solid #252C32', borderRight: '1px solid #252C32' }}>
          <div
            style={{
              fontSize: '0.65rem',
              color: '#74808A',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              marginBottom: '4px',
            }}
          >
            Distance
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: '#25A18E', fontFamily: 'monospace' }}>
            {distanceKm.toFixed(1)} km
          </div>
        </div>

        <div style={{ textAlign: 'center' }}>
          <div
            style={{
              fontSize: '0.65rem',
              color: '#74808A',
              textTransform: 'uppercase',
              letterSpacing: '0.04em',
              marginBottom: '4px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '3px',
            }}
          >
            <Send size={11} color="#9AA5AD" />
            Updates Sent
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: '#E5E9EC', fontFamily: 'monospace' }}>
            {updatesSent}
          </div>
        </div>
      </div>
    </div>
  );
};
