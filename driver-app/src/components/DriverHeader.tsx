import React from 'react';
import { Bus, LogOut, User } from 'lucide-react';

interface DriverHeaderProps {
  driverName?: string;
  onLogout: () => void;
}

export const DriverHeader: React.FC<DriverHeaderProps> = ({ driverName, onLogout }) => {
  return (
    <header
      style={{
        backgroundColor: '#1E242A',
        borderBottom: '1px solid #39434B',
        padding: '12px 16px',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        width: '100%',
        position: 'sticky',
        top: 0,
        zIndex: 50,
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
        <div
          style={{
            backgroundColor: '#252C32',
            border: '1px solid #39434B',
            borderRadius: '6px',
            padding: '6px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: '#25A18E',
          }}
        >
          <Bus size={20} />
        </div>
        <div>
          <div
            style={{
              fontSize: '1rem',
              fontWeight: 700,
              color: '#E5E9EC',
              letterSpacing: '-0.02em',
              lineHeight: 1.2,
            }}
          >
            Transit<span style={{ color: '#25A18E' }}>X</span>{' '}
            <span style={{ fontSize: '0.8rem', fontWeight: 500, color: '#9AA5AD' }}>Driver</span>
          </div>
          {driverName && (
            <div
              style={{
                fontSize: '0.75rem',
                color: '#74808A',
                display: 'flex',
                alignItems: 'center',
                gap: '4px',
                marginTop: '1px',
              }}
            >
              <User size={12} />
              <span>{driverName}</span>
            </div>
          )}
        </div>
      </div>

      <button
        onClick={onLogout}
        title="Sign Out"
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: '6px',
          backgroundColor: '#252C32',
          border: '1px solid #39434B',
          borderRadius: '6px',
          padding: '8px 12px',
          color: '#9AA5AD',
          fontSize: '0.85rem',
          fontWeight: 500,
          cursor: 'pointer',
          minHeight: '38px',
        }}
      >
        <LogOut size={15} />
        <span>Sign Out</span>
      </button>
    </header>
  );
};
