import React from 'react';
import { TripStatus } from '../services/trip.service';
import { ConnectionStatusType } from '../hooks/useGpsSender';
import { PermissionStatusType } from '../hooks/useGeolocation';

interface StatusIndicatorProps {
  tripStatus: TripStatus;
  connectionStatus: ConnectionStatusType;
  permissionStatus: PermissionStatusType;
  accuracy: number | null;
  isStale: boolean;
  isBackgrounded: boolean;
}

export const StatusIndicator: React.FC<StatusIndicatorProps> = ({
  tripStatus,
  connectionStatus,
  permissionStatus,
  accuracy,
  isStale,
  isBackgrounded,
}) => {
  // Determine primary high-level status
  let label = 'STANDBY';
  let subtext = 'Vehicle ready for trip start';
  let color = '#74808A';
  let pulse = false;

  if (tripStatus === 'ended') {
    label = 'TRIP ENDED';
    subtext = 'Telemetry transmission completed';
    color = '#74808A';
    pulse = false;
  } else if (permissionStatus === 'denied') {
    label = 'PERMISSION DENIED';
    subtext = 'Enable GPS access in browser';
    color = '#D96C65';
    pulse = false;
  } else if (connectionStatus === 'DISCONNECTED') {
    label = 'CONNECTION LOST';
    subtext = 'Buffering updates offline';
    color = '#D96C65';
    pulse = true;
  } else if (tripStatus === 'active') {
    if (isStale) {
      label = 'LOCATION STALE';
      subtext = 'No GPS fix for > 30 seconds';
      color = '#E69A43';
      pulse = true;
    } else if (accuracy !== null && accuracy > 50) {
      label = 'GPS SIGNAL WEAK';
      subtext = `Degraded fix (±${Math.round(accuracy)}m)`;
      color = '#E69A43';
      pulse = true;
    } else if (isBackgrounded) {
      label = 'RUNNING IN BACKGROUND';
      subtext = 'Foreground recommended for reliable GPS';
      color = '#E69A43';
      pulse = true;
    } else {
      label = 'LIVE';
      subtext = 'Transmitting real-time GPS coordinates';
      color = '#55A88A';
      pulse = true;
    }
  } else if (tripStatus === 'paused') {
    label = 'TRIP PAUSED';
    subtext = 'Transmission paused by operator';
    color = '#E69A43';
    pulse = false;
  } else {
    label = 'READY';
    subtext = 'GPS receiver standing by';
    color = '#9AA5AD';
    pulse = false;
  }

  return (
    <div
      style={{
        backgroundColor: '#1E242A',
        border: '1px solid #39434B',
        borderRadius: '12px',
        padding: '20px 16px',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        textAlign: 'center',
        width: '100%',
        position: 'relative',
        overflow: 'hidden',
      }}
    >
      <div
        style={{
          position: 'relative',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          marginBottom: '14px',
        }}
      >
        {pulse && (
          <div
            style={{
              position: 'absolute',
              width: '46px',
              height: '46px',
              borderRadius: '50%',
              backgroundColor: color,
              opacity: 0.25,
              animation: 'pulse 1.8s ease-out infinite',
            }}
          />
        )}
        <div
          style={{
            width: '24px',
            height: '24px',
            borderRadius: '50%',
            backgroundColor: color,
            border: `3px solid #171B20`,
            boxShadow: `0 0 12px ${color}88`,
          }}
        />
      </div>

      <div
        style={{
          fontSize: '1.25rem',
          fontWeight: 800,
          color: color,
          letterSpacing: '0.06em',
          lineHeight: 1.2,
        }}
      >
        {label}
      </div>

      <div
        style={{
          fontSize: '0.78rem',
          color: '#9AA5AD',
          marginTop: '4px',
          maxWidth: '280px',
        }}
      >
        {subtext}
      </div>
    </div>
  );
};
