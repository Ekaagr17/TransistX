import React from 'react';
import { Wifi, WifiOff, RefreshCw } from 'lucide-react';
import { ConnectionStatusType } from '../hooks/useGpsSender';

interface ConnectionStatusProps {
  status: ConnectionStatusType;
  lastTransmission: Date | null;
  queueLength?: number;
}

export const ConnectionStatus: React.FC<ConnectionStatusProps> = ({
  status,
  lastTransmission,
  queueLength = 0,
}) => {
  const getStatusConfig = () => {
    switch (status) {
      case 'CONNECTED':
        return {
          color: '#55A88A',
          bgColor: 'rgba(85, 168, 138, 0.1)',
          borderColor: 'rgba(85, 168, 138, 0.3)',
          label: 'ONLINE / SYNCED',
          icon: <Wifi size={14} color="#55A88A" />,
          isPulse: false,
        };
      case 'RECONNECTING':
        return {
          color: '#E69A43',
          bgColor: 'rgba(230, 154, 67, 0.1)',
          borderColor: 'rgba(230, 154, 67, 0.3)',
          label: 'RECONNECTING...',
          icon: <RefreshCw size={14} color="#E69A43" className="animate-pulse" />,
          isPulse: true,
        };
      case 'DISCONNECTED':
      default:
        return {
          color: '#D96C65',
          bgColor: 'rgba(217, 108, 101, 0.1)',
          borderColor: 'rgba(217, 108, 101, 0.3)',
          label: 'DISCONNECTED',
          icon: <WifiOff size={14} color="#D96C65" />,
          isPulse: false,
        };
    }
  };

  const config = getStatusConfig();

  const formatLastTransmission = (date: Date | null) => {
    if (!date) return 'No packets sent yet';
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };

  return (
    <div
      style={{
        backgroundColor: '#1E242A',
        border: '1px solid #39434B',
        borderRadius: '10px',
        padding: '12px 16px',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        width: '100%',
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            padding: '6px',
            borderRadius: '6px',
            backgroundColor: config.bgColor,
            border: `1px solid ${config.borderColor}`,
          }}
        >
          {config.icon}
        </div>
        <div>
          <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
            <span
              style={{
                width: '8px',
                height: '8px',
                borderRadius: '50%',
                backgroundColor: config.color,
                display: 'inline-block',
                ...(config.isPulse ? { animation: 'pulse 1.2s infinite' } : {}),
              }}
            />
            <span
              style={{
                fontSize: '0.8rem',
                fontWeight: 700,
                color: config.color,
                letterSpacing: '0.04em',
              }}
            >
              {config.label}
            </span>
            {queueLength > 0 && (
              <span
                style={{
                  fontSize: '0.7rem',
                  backgroundColor: '#252C32',
                  border: '1px solid #39434B',
                  color: '#E69A43',
                  padding: '1px 6px',
                  borderRadius: '4px',
                  fontWeight: 600,
                }}
              >
                {queueLength} queued
              </span>
            )}
          </div>
          <div style={{ fontSize: '0.75rem', color: '#74808A', marginTop: '2px' }}>
            Last push: {formatLastTransmission(lastTransmission)}
          </div>
        </div>
      </div>
    </div>
  );
};
