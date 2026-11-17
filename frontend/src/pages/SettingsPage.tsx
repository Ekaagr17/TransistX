import React, { useState } from 'react';
import { Settings as SettingsIcon, Bell, Moon, Shield, Info } from 'lucide-react';
import { useAuth } from '../hooks/useAuth';

export const SettingsPage: React.FC = () => {
  const { user } = useAuth();
  const [notificationsEnabled, setNotificationsEnabled] = useState(true);
  const [autoRefreshInterval, setAutoRefreshInterval] = useState('5');
  const [savedSuccess, setSavedSuccess] = useState(false);

  const handleSave = (e: React.FormEvent) => {
    e.preventDefault();
    setSavedSuccess(true);
    setTimeout(() => setSavedSuccess(false), 3000);
  };

  return (
    <div style={{ padding: 'var(--space-2xl)', maxWidth: '700px', margin: '0 auto', width: '100%' }}>
      <div style={{ marginBottom: 'var(--space-2xl)' }}>
        <h1 style={{ fontSize: 'var(--font-size-2xl)', fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
          System Settings
        </h1>
        <p style={{ color: 'var(--text-secondary)', fontSize: 'var(--font-size-sm)', marginTop: 'var(--space-xs)' }}>
          Manage your application preferences and telemetry stream settings.
        </p>
      </div>

      {savedSuccess && (
        <div
          style={{
            padding: 'var(--space-sm) var(--space-md)',
            backgroundColor: 'rgba(85, 168, 138, 0.1)',
            border: '1px solid var(--color-success)',
            borderRadius: 'var(--radius-md)',
            color: 'var(--color-success)',
            fontSize: 'var(--font-size-sm)',
            marginBottom: 'var(--space-lg)',
          }}
        >
          Preferences saved successfully!
        </div>
      )}

      <form onSubmit={handleSave} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-xl)' }}>
        {/* User Account Info */}
        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', marginBottom: 'var(--space-md)' }}>
            <Shield size={18} style={{ color: 'var(--accent-primary)' }} />
            <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)' }}>
              Account Information
            </h3>
          </div>
          {user ? (
            <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-xs)', fontSize: 'var(--font-size-sm)' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                <span style={{ color: 'var(--text-muted)' }}>Name:</span>
                <span>{user.name}</span>
              </div>
              <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                <span style={{ color: 'var(--text-muted)' }}>Email:</span>
                <span>{user.email}</span>
              </div>
              <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                <span style={{ color: 'var(--text-muted)' }}>Role:</span>
                <span style={{ textTransform: 'capitalize', color: 'var(--accent-primary)', fontWeight: 'var(--font-weight-medium)' }}>
                  {user.role}
                </span>
              </div>
            </div>
          ) : (
            <div style={{ fontSize: 'var(--font-size-sm)', color: 'var(--text-muted)' }}>
              Guest mode. Sign in to sync favorites and preferences.
            </div>
          )}
        </div>

        {/* Live Tracking Preferences */}
        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', marginBottom: 'var(--space-md)' }}>
            <Bell size={18} style={{ color: 'var(--accent-primary)' }} />
            <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)' }}>
              Telemetry & Notification Preferences
            </h3>
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
            <div className="input-group">
              <label>GPS WebSocket Polling Fallback (seconds)</label>
              <select
                value={autoRefreshInterval}
                onChange={(e) => setAutoRefreshInterval(e.target.value)}
              >
                <option value="3">3 seconds (High frequency)</option>
                <option value="5">5 seconds (Standard)</option>
                <option value="10">10 seconds (Low bandwidth)</option>
              </select>
            </div>

            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
              <div>
                <div style={{ fontSize: 'var(--font-size-sm)', fontWeight: 'var(--font-weight-medium)' }}>
                  Service Disruption Alerts
                </div>
                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                  Notify me of emergency delays or cancellations
                </div>
              </div>
              <input
                type="checkbox"
                checked={notificationsEnabled}
                onChange={(e) => setNotificationsEnabled(e.target.checked)}
                style={{ width: '18px', height: '18px', cursor: 'pointer' }}
              />
            </div>
          </div>
        </div>

        {/* System Diagnostics Info */}
        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', marginBottom: 'var(--space-md)' }}>
            <Info size={18} style={{ color: 'var(--accent-primary)' }} />
            <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)' }}>
              System Information
            </h3>
          </div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-xs)', fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
            <div>Client: TransitX Passenger Web App v1.0.0</div>
            <div>Basemap: Carto Dark / OpenStreetMap (WGS84 EPSG:4326)</div>
            <div>Region: Dehradun, Uttarakhand, India</div>
          </div>
        </div>

        <button type="submit" className="btn btn-primary-filled" style={{ alignSelf: 'flex-start' }}>
          Save Preferences
        </button>
      </form>
    </div>
  );
};
