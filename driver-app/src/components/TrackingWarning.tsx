import React from 'react';
import { AlertTriangle, AlertOctagon, EyeOff, SignalLow } from 'lucide-react';
import { ConnectionStatusType } from '../hooks/useGpsSender';
import { PermissionStatusType } from '../hooks/useGeolocation';

interface TrackingWarningProps {
  isBackgrounded: boolean;
  isStale: boolean;
  accuracy: number | null;
  connectionStatus: ConnectionStatusType;
  permissionStatus: PermissionStatusType;
  errorMessage?: string | null;
}

export const TrackingWarning: React.FC<TrackingWarningProps> = ({
  isBackgrounded,
  isStale,
  accuracy,
  connectionStatus,
  permissionStatus,
  errorMessage,
}) => {
  const warnings: { id: string; type: 'warning' | 'error'; title: string; message: string; icon: React.ReactNode }[] = [];

  if (permissionStatus === 'denied') {
    warnings.push({
      id: 'permission-denied',
      type: 'error',
      title: 'GPS Access Denied',
      message: 'TransitX cannot track without location access. Please enable location permissions in your browser or device settings.',
      icon: <AlertOctagon size={18} color="#D96C65" />,
    });
  }

  if (connectionStatus === 'DISCONNECTED') {
    warnings.push({
      id: 'conn-lost',
      type: 'error',
      title: 'Backend Connection Lost',
      message: 'Offline. Location telemetry is being cached locally and will flush once network connectivity is re-established.',
      icon: <AlertOctagon size={18} color="#D96C65" />,
    });
  }

  if (isBackgrounded) {
    warnings.push({
      id: 'backgrounded',
      type: 'warning',
      title: 'App In Background',
      message: 'Mobile browsers heavily throttle background GPS. Keep this screen active and in foreground to prevent telemetry drops.',
      icon: <EyeOff size={18} color="#E69A43" />,
    });
  }

  if (isStale) {
    warnings.push({
      id: 'stale-fix',
      type: 'warning',
      title: 'Location Fix Stale (>30s)',
      message: 'No recent GPS coordinates received from hardware sensors. Ensure sky visibility or restart device GPS.',
      icon: <SignalLow size={18} color="#E69A43" />,
    });
  }

  if (accuracy !== null && accuracy > 50) {
    warnings.push({
      id: 'poor-accuracy',
      type: 'warning',
      title: `Suboptimal Accuracy (±${Math.round(accuracy)}m)`,
      message: 'GPS precision is degraded. Positions with accuracy >200m are discarded to prevent erratic passenger map data.',
      icon: <AlertTriangle size={18} color="#E69A43" />,
    });
  }

  if (errorMessage && !warnings.some((w) => w.id === 'permission-denied')) {
    warnings.push({
      id: 'general-error',
      type: 'error',
      title: 'Sensor Notice',
      message: errorMessage,
      icon: <AlertOctagon size={18} color="#D96C65" />,
    });
  }

  if (warnings.length === 0) {
    return null;
  }

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '8px', width: '100%' }}>
      {warnings.map((w) => {
        const isError = w.type === 'error';
        const borderColor = isError ? 'rgba(217, 108, 101, 0.4)' : 'rgba(230, 154, 67, 0.4)';
        const bgColor = isError ? 'rgba(217, 108, 101, 0.1)' : 'rgba(230, 154, 67, 0.1)';
        const headerColor = isError ? '#D96C65' : '#E69A43';

        return (
          <div
            key={w.id}
            style={{
              backgroundColor: bgColor,
              border: `1px solid ${borderColor}`,
              borderRadius: '8px',
              padding: '12px 14px',
              display: 'flex',
              gap: '12px',
              alignItems: 'flex-start',
            }}
          >
            <div style={{ marginTop: '2px', flexShrink: 0 }}>{w.icon}</div>
            <div style={{ display: 'flex', flexDirection: 'column', gap: '2px' }}>
              <div style={{ fontSize: '0.85rem', fontWeight: 700, color: headerColor }}>
                {w.title}
              </div>
              <div style={{ fontSize: '0.78rem', color: '#9AA5AD', lineHeight: 1.4 }}>
                {w.message}
              </div>
            </div>
          </div>
        );
      })}
    </div>
  );
};
