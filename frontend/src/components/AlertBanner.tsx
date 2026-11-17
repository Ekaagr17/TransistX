import React from 'react';
import { AlertTriangle, Info, AlertCircle, X } from 'lucide-react';
import type { ServiceAlert } from '../types';

interface AlertBannerProps {
  alerts: ServiceAlert[];
  onDismiss?: (id: number) => void;
}

export const AlertBanner: React.FC<AlertBannerProps> = ({ alerts, onDismiss }) => {
  if (!alerts || alerts.length === 0) return null;

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-xs)' }}>
      {alerts.map((alert) => {
        const isCritical = alert.severity === 'critical';
        const isWarning = alert.severity === 'warning';
        const borderColor = isCritical
          ? 'var(--color-error)'
          : isWarning
          ? 'var(--color-warning)'
          : 'var(--accent-primary)';
        const bgColor = isCritical
          ? 'rgba(217, 108, 101, 0.1)'
          : isWarning
          ? 'rgba(230, 154, 67, 0.1)'
          : 'rgba(37, 161, 142, 0.1)';

        return (
          <div
            key={alert.id}
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              padding: 'var(--space-sm) var(--space-md)',
              backgroundColor: bgColor,
              border: `1px solid ${borderColor}`,
              borderRadius: 'var(--radius-md)',
              fontSize: 'var(--font-size-sm)',
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
              {isCritical ? (
                <AlertCircle size={16} color="var(--color-error)" />
              ) : isWarning ? (
                <AlertTriangle size={16} color="var(--color-warning)" />
              ) : (
                <Info size={16} color="var(--accent-primary)" />
              )}
              <span style={{ color: 'var(--text-primary)' }}>{alert.message}</span>
            </div>
            {onDismiss && (
              <button
                type="button"
                onClick={() => onDismiss(alert.id)}
                style={{
                  display: 'flex',
                  alignItems: 'center',
                  color: 'var(--text-muted)',
                  cursor: 'pointer',
                  padding: '2px',
                }}
              >
                <X size={14} />
              </button>
            )}
          </div>
        );
      })}
    </div>
  );
};
