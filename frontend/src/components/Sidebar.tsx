import React from 'react';
import { NavLink } from 'react-router-dom';
import {
  LayoutDashboard,
  Bus,
  Route,
  MapPin,
  Star,
  Settings,
  ShieldAlert,
  ChevronLeft,
  ChevronRight,
} from 'lucide-react';
import { useAuth } from '../hooks/useAuth';

interface SidebarProps {
  collapsed: boolean;
  onToggleCollapse: () => void;
}

export const Sidebar: React.FC<SidebarProps> = ({ collapsed, onToggleCollapse }) => {
  const { user } = useAuth();
  const isAdmin = user?.role === 'admin';

  const navItems = [
    { label: 'Overview', to: '/', icon: LayoutDashboard },
    { label: 'Live Buses', to: '/buses', icon: Bus },
    { label: 'Routes', to: '/routes', icon: Route },
    { label: 'Stops', to: '/stops', icon: MapPin },
    { label: 'Favourites', to: '/favorites', icon: Star },
    { label: 'Settings', to: '/settings', icon: Settings },
  ];

  if (isAdmin) {
    navItems.push({ label: 'Admin', to: '/admin', icon: ShieldAlert });
  }

  return (
    <aside
      style={{
        width: collapsed ? 'var(--sidebar-collapsed-width)' : 'var(--sidebar-width)',
        height: '100vh',
        backgroundColor: 'var(--bg-secondary)',
        borderRight: '1px solid var(--border-color)',
        display: 'flex',
        flexDirection: 'column',
        transition: 'width var(--transition-normal)',
        flexShrink: 0,
        zIndex: 50,
      }}
    >
      {/* Brand Header */}
      <div
        style={{
          height: 'var(--header-height)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: collapsed ? 'center' : 'space-between',
          padding: collapsed ? '0' : '0 var(--space-lg)',
          borderBottom: '1px solid var(--border-color)',
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', overflow: 'hidden' }}>
          <div
            style={{
              width: '28px',
              height: '28px',
              backgroundColor: 'var(--accent-primary)',
              borderRadius: 'var(--radius-sm)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: 'var(--bg-primary)',
              fontWeight: 'var(--font-weight-bold)',
              fontSize: 'var(--font-size-md)',
              flexShrink: 0,
            }}
          >
            TX
          </div>
          {!collapsed && (
            <span
              style={{
                fontSize: 'var(--font-size-md)',
                fontWeight: 'var(--font-weight-bold)',
                letterSpacing: '0.05em',
                color: 'var(--text-primary)',
              }}
            >
              Transit<span style={{ color: 'var(--accent-primary)' }}>X</span>
            </span>
          )}
        </div>

        <button
          type="button"
          onClick={onToggleCollapse}
          style={{
            display: collapsed ? 'none' : 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            width: '24px',
            height: '24px',
            color: 'var(--text-muted)',
            cursor: 'pointer',
          }}
          title={collapsed ? 'Expand' : 'Collapse'}
        >
          <ChevronLeft size={18} />
        </button>
      </div>

      {/* Nav List */}
      <nav style={{ flex: 1, padding: 'var(--space-md) 0', overflowY: 'auto' }}>
        <ul style={{ display: 'flex', flexDirection: 'column', gap: '2px' }}>
          {navItems.map((item) => {
            const Icon = item.icon;
            return (
              <li key={item.to}>
                <NavLink
                  to={item.to}
                  style={({ isActive }) => ({
                    display: 'flex',
                    alignItems: 'center',
                    gap: 'var(--space-md)',
                    padding: collapsed ? 'var(--space-md) 0' : 'var(--space-sm) var(--space-lg)',
                    justifyContent: collapsed ? 'center' : 'flex-start',
                    color: isActive ? 'var(--text-primary)' : 'var(--text-secondary)',
                    backgroundColor: isActive ? 'var(--bg-surface)' : 'transparent',
                    borderLeft: isActive ? '3px solid var(--accent-primary)' : '3px solid transparent',
                    fontWeight: isActive ? 'var(--font-weight-semibold)' : 'var(--font-weight-normal)',
                    fontSize: 'var(--font-size-sm)',
                    textDecoration: 'none',
                    transition: 'background var(--transition-fast)',
                  })}
                  title={collapsed ? item.label : undefined}
                >
                  <Icon size={18} style={{ flexShrink: 0 }} />
                  {!collapsed && <span>{item.label}</span>}
                </NavLink>
              </li>
            );
          })}
        </ul>
      </nav>

      {/* Collapse Button (When collapsed) */}
      {collapsed && (
        <div style={{ padding: 'var(--space-md) 0', display: 'flex', justifyContent: 'center' }}>
          <button
            type="button"
            onClick={onToggleCollapse}
            style={{ color: 'var(--text-muted)' }}
            title="Expand"
          >
            <ChevronRight size={18} />
          </button>
        </div>
      )}
    </aside>
  );
};
