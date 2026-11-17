import React from 'react';

interface StatusBadgeProps {
  status: 'LIVE' | 'STALE' | 'OFFLINE' | 'active' | 'inactive';
}

export const StatusBadge: React.FC<StatusBadgeProps> = ({ status }) => {
  const normalized = status.toUpperCase();
  let className = 'badge-offline';
  let label = 'OFFLINE';

  if (normalized === 'LIVE' || normalized === 'ACTIVE') {
    className = 'badge-live';
    label = 'LIVE';
  } else if (normalized === 'STALE') {
    className = 'badge-stale';
    label = 'STALE';
  } else {
    className = 'badge-offline';
    label = 'OFFLINE';
  }

  return (
    <span className={`badge ${className}`}>
      <span
        style={{
          width: '6px',
          height: '6px',
          borderRadius: '50%',
          backgroundColor: 'currentColor',
          marginRight: '6px',
        }}
      />
      {label}
    </span>
  );
};
