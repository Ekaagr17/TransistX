import React from 'react';
import { Compass, Navigation, Radio, ShieldCheck, ShieldAlert } from 'lucide-react';
import { PermissionStatusType, GeolocationPositionData } from '../hooks/useGeolocation';

interface GpsStatusCardProps {
  permissionStatus: PermissionStatusType;
  accuracy: number | null;
  position: GeolocationPositionData | null;
  lastUpdate: Date | null;
  isTracking: boolean;
}

export const GpsStatusCard: React.FC<GpsStatusCardProps> = ({
  permissionStatus,
  accuracy,
  position,
  lastUpdate,
  isTracking,
}) => {
  const getSignalQuality = (acc: number | null) => {
    if (acc === null) return { label: 'AQUIRING...', color: '#74808A' };
    if (acc < 10) return { label: 'EXCELLENT', color: '#55A88A' };
    if (acc < 30) return { label: 'GOOD', color: '#25A18E' };
    if (acc < 50) return { label: 'FAIR', color: '#E69A43' };
    return { label: 'POOR', color: '#D96C65' };
  };

  const getPermissionLabel = (perm: PermissionStatusType) => {
    switch (perm) {
      case 'granted':
        return { label: 'GRANTED', color: '#55A88A' };
      case 'denied':
        return { label: 'DENIED', color: '#D96C65' };
      case 'prompt':
        return { label: 'PROMPT', color: '#E69A43' };
      default:
        return { label: 'UNKNOWN', color: '#74808A' };
    }
  };

  const signalQuality = getSignalQuality(accuracy);
  const permDisplay = getPermissionLabel(permissionStatus);

  const formatTimestamp = (date: Date | null) => {
    if (!date) return 'Waiting for fix...';
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };

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
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
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
          <Radio size={14} color="#25A18E" />
          GPS Receiver Status
        </span>
        <span
          style={{
            fontSize: '0.75rem',
            fontWeight: 600,
            color: isTracking ? '#55A88A' : '#74808A',
            letterSpacing: '0.04em',
          }}
        >
          {isTracking ? 'RECEIVING STREAM' : 'STANDBY'}
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
            {permissionStatus === 'granted' ? <ShieldCheck size={12} color="#55A88A" /> : <ShieldAlert size={12} color="#D96C65" />}
            Permission
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: permDisplay.color }}>
            {permDisplay.label}
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
            }}
          >
            Signal Quality
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: signalQuality.color }}>
            {signalQuality.label}
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
            }}
          >
            Accuracy Radius
          </div>
          <div style={{ fontSize: '0.95rem', fontWeight: 700, color: '#E5E9EC' }}>
            {accuracy !== null ? `±${Math.round(accuracy)} m` : '--'}
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
            }}
          >
            Last GPS Fix
          </div>
          <div style={{ fontSize: '0.85rem', fontWeight: 600, color: '#E5E9EC' }}>
            {formatTimestamp(lastUpdate)}
          </div>
        </div>
      </div>

      <div
        style={{
          backgroundColor: '#171B20',
          border: '1px solid #2C343B',
          borderRadius: '8px',
          padding: '12px',
          display: 'flex',
          flexDirection: 'column',
          gap: '8px',
        }}
      >
        <div
          style={{
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'center',
            fontSize: '0.75rem',
            color: '#9AA5AD',
          }}
        >
          <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <Navigation size={13} color="#25A18E" />
            Coordinates
          </span>
          <span style={{ fontFamily: 'monospace', color: '#E5E9EC', fontSize: '0.8rem' }}>
            {position
              ? `${position.latitude.toFixed(5)}, ${position.longitude.toFixed(5)}`
              : 'Waiting for coordinates...'}
          </span>
        </div>

        <div
          style={{
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'center',
            fontSize: '0.75rem',
            color: '#9AA5AD',
            borderTop: '1px solid #252C32',
            paddingTop: '6px',
          }}
        >
          <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <Compass size={13} color="#E69A43" />
            Heading / Speed
          </span>
          <span style={{ fontFamily: 'monospace', color: '#E5E9EC', fontSize: '0.8rem' }}>
            {position
              ? `${position.heading !== null ? Math.round(position.heading) + '°' : '--'} / ${
                  position.speed !== null ? (position.speed * 3.6).toFixed(1) + ' km/h' : '0.0 km/h'
                }`
              : '-- / --'}
          </span>
        </div>
      </div>
    </div>
  );
};
