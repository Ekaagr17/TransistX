import React from 'react';
import { Outlet, Link } from 'react-router-dom';

export const AuthLayout: React.FC = () => {
  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        minHeight: '100vh',
        backgroundColor: 'var(--bg-primary)',
        padding: 'var(--space-xl)',
      }}
    >
      <div style={{ marginBottom: 'var(--space-2xl)', textAlign: 'center' }}>
        <Link to="/" style={{ display: 'inline-flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
          <div
            style={{
              width: '36px',
              height: '36px',
              backgroundColor: 'var(--accent-primary)',
              borderRadius: 'var(--radius-sm)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: 'var(--bg-primary)',
              fontWeight: 'var(--font-weight-bold)',
              fontSize: 'var(--font-size-lg)',
            }}
          >
            TX
          </div>
          <span
            style={{
              fontSize: 'var(--font-size-2xl)',
              fontWeight: 'var(--font-weight-bold)',
              color: 'var(--text-primary)',
              letterSpacing: '0.05em',
            }}
          >
            Transit<span style={{ color: 'var(--accent-primary)' }}>X</span>
          </span>
        </Link>
        <p style={{ color: 'var(--text-secondary)', fontSize: 'var(--font-size-sm)', marginTop: 'var(--space-xs)' }}>
          Real-Time Public Transit Intelligence
        </p>
      </div>

      <div
        style={{
          width: '100%',
          maxWidth: '420px',
          backgroundColor: 'var(--bg-secondary)',
          border: '1px solid var(--border-color)',
          borderRadius: 'var(--radius-xl)',
          padding: 'var(--space-2xl)',
          boxShadow: 'var(--shadow-lg)',
        }}
      >
        <Outlet />
      </div>
    </div>
  );
};
