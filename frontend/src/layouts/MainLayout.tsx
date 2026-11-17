import React, { useState, useEffect } from 'react';
import { Outlet, useNavigate } from 'react-router-dom';
import { Sidebar } from '../components/Sidebar';
import { Header } from '../components/Header';
import { AlertBanner } from '../components/AlertBanner';
import { alertService } from '../services/alert.service';
import type { ServiceAlert, SearchResult } from '../types';

export const MainLayout: React.FC = () => {
  const [collapsed, setCollapsed] = useState(false);
  const [alerts, setAlerts] = useState<ServiceAlert[]>([]);
  const [showAlerts, setShowAlerts] = useState(false);
  const navigate = useNavigate();

  useEffect(() => {
    const fetchAlerts = async () => {
      try {
        const data = await alertService.getAlerts();
        setAlerts(data);
      } catch {
        // quiet error
      }
    };
    fetchAlerts();
  }, []);

  const handleSearchResultSelect = (result: SearchResult) => {
    if (result.type === 'bus') {
      navigate('/buses', { state: { selectedBusId: result.id } });
    } else if (result.type === 'route') {
      navigate('/routes', { state: { selectedRouteId: result.id } });
    } else if (result.type === 'stop') {
      navigate('/stops', { state: { selectedStopId: result.id } });
    }
  };

  const handleDismissAlert = (id: number) => {
    setAlerts((prev) => prev.filter((a) => a.id !== id));
  };

  return (
    <div style={{ display: 'flex', height: '100vh', width: '100vw', overflow: 'hidden' }}>
      <Sidebar collapsed={collapsed} onToggleCollapse={() => setCollapsed(!collapsed)} />

      <div style={{ display: 'flex', flexDirection: 'column', flex: 1, minWidth: 0, height: '100%' }}>
        <Header
          onSearchResultSelect={handleSearchResultSelect}
          unreadAlertsCount={alerts.length}
          onToggleAlerts={() => setShowAlerts(!showAlerts)}
        />

        {showAlerts && alerts.length > 0 && (
          <div style={{ padding: 'var(--space-md) var(--space-xl)', borderBottom: '1px solid var(--border-color)', backgroundColor: 'var(--bg-secondary)' }}>
            <AlertBanner alerts={alerts} onDismiss={handleDismissAlert} />
          </div>
        )}

        <main style={{ flex: 1, position: 'relative', overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
          <Outlet />
        </main>
      </div>
    </div>
  );
};
