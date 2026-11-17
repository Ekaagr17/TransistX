import React, { useState } from 'react';
import { Bell, User as UserIcon, LogOut } from 'lucide-react';
import { useAuth } from '../hooks/useAuth';
import { SearchBar } from './SearchBar';
import { LoginModal } from './LoginModal';
import type { SearchResult } from '../types';

interface HeaderProps {
  onSearchResultSelect?: (result: SearchResult) => void;
  unreadAlertsCount?: number;
  onToggleAlerts?: () => void;
}

export const Header: React.FC<HeaderProps> = ({
  onSearchResultSelect,
  unreadAlertsCount = 0,
  onToggleAlerts,
}) => {
  const { user, isAuthenticated, logout } = useAuth();
  const [loginModalOpen, setLoginModalOpen] = useState(false);

  return (
    <>
      <header
        style={{
          height: 'var(--header-height)',
          backgroundColor: 'var(--bg-secondary)',
          borderBottom: '1px solid var(--border-color)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          padding: '0 var(--space-xl)',
          zIndex: 40,
        }}
      >
        {/* Search */}
        <SearchBar onSelectResult={onSearchResultSelect} />

        {/* Right Actions */}
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-md)' }}>
          {/* Notifications bell */}
          <button
            type="button"
            onClick={onToggleAlerts}
            style={{
              position: 'relative',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              width: '36px',
              height: '36px',
              borderRadius: 'var(--radius-md)',
              border: '1px solid var(--border-color)',
              backgroundColor: 'var(--bg-surface)',
              color: 'var(--text-secondary)',
            }}
            title="Service Alerts"
          >
            <Bell size={16} />
            {unreadAlertsCount > 0 && (
              <span
                style={{
                  position: 'absolute',
                  top: '-3px',
                  right: '-3px',
                  width: '8px',
                  height: '8px',
                  borderRadius: '50%',
                  backgroundColor: 'var(--accent-secondary)',
                }}
              />
            )}
          </button>

          {/* User auth */}
          {isAuthenticated && user ? (
            <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
              <div
                style={{
                  display: 'flex',
                  alignItems: 'center',
                  gap: 'var(--space-sm)',
                  padding: 'var(--space-xs) var(--space-md)',
                  backgroundColor: 'var(--bg-surface)',
                  border: '1px solid var(--border-color)',
                  borderRadius: 'var(--radius-md)',
                  fontSize: 'var(--font-size-sm)',
                }}
              >
                <div
                  style={{
                    width: '22px',
                    height: '22px',
                    borderRadius: '50%',
                    backgroundColor: 'var(--bg-elevated)',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    color: 'var(--accent-primary)',
                  }}
                >
                  <UserIcon size={12} />
                </div>
                <span style={{ fontWeight: 'var(--font-weight-medium)', color: 'var(--text-primary)' }}>
                  {user.name}
                </span>
                <span style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                  ({user.role})
                </span>
              </div>
              <button
                type="button"
                onClick={logout}
                className="btn btn-sm"
                style={{ color: 'var(--text-muted)' }}
                title="Sign Out"
              >
                <LogOut size={16} />
              </button>
            </div>
          ) : (
            <button
              type="button"
              className="btn btn-primary btn-sm"
              onClick={() => setLoginModalOpen(true)}
            >
              Sign In
            </button>
          )}
        </div>
      </header>

      <LoginModal isOpen={loginModalOpen} onClose={() => setLoginModalOpen(false)} />
    </>
  );
};
