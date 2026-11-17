import React, { useState, useEffect } from 'react';
import {
  ShieldAlert,
  Bus as BusIcon,
  Route as RouteIcon,
  MapPin,
  Users,
  Calendar,
  AlertTriangle,
  Plus,
  Trash2,
  Edit2,
  Check,
} from 'lucide-react';
import { useAuth } from '../hooks/useAuth';
import { busService } from '../services/bus.service';
import { routeService } from '../services/route.service';
import { stopService } from '../services/stop.service';
import { alertService } from '../services/alert.service';
import api from '../services/api';
import { LoadingSpinner } from '../components/LoadingSpinner';
import type { Bus, Route, Stop, ServiceAlert, Driver, Trip } from '../types';

export const AdminPage: React.FC = () => {
  const { user, isAuthenticated } = useAuth();
  const [activeTab, setActiveTab] = useState<'buses' | 'routes' | 'stops' | 'drivers' | 'trips' | 'alerts'>('buses');
  
  // States for data
  const [buses, setBuses] = useState<Bus[]>([]);
  const [routes, setRoutes] = useState<Route[]>([]);
  const [stops, setStops] = useState<Stop[]>([]);
  const [alerts, setAlerts] = useState<ServiceAlert[]>([]);
  const [drivers, setDrivers] = useState<Driver[]>([]);
  const [trips, setTrips] = useState<Trip[]>([]);
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState<string | null>(null);

  // Forms
  const [newBus, setNewBus] = useState({ bus_number: '', registration_number: '', capacity: 45 });
  const [newRoute, setNewRoute] = useState({ route_name: '', route_code: '', start_point: '', end_point: '' });
  const [newStop, setNewStop] = useState({ stop_name: '', latitude: 30.3165, longitude: 78.0322 });
  const [newAlert, setNewAlert] = useState({ message: '', severity: 'info' as const, route_id: '' });
  const [newTrip, setNewTrip] = useState({ bus_id: '', route_id: '', driver_id: '' });

  // Initial load
  useEffect(() => {
    loadAll();
  }, []);

  const loadAll = async () => {
    setLoading(true);
    try {
      const [bData, rData, sData, aData] = await Promise.all([
        busService.getBuses().catch(() => []),
        routeService.getRoutes().catch(() => []),
        stopService.getStops().catch(() => []),
        alertService.getAlerts().catch(() => []),
      ]);
      setBuses(bData);
      setRoutes(rData);
      setStops(sData);
      setAlerts(aData);

      // Fetch drivers and trips from admin/direct endpoints
      try {
        const driversRes = await api.get('/drivers');
        setDrivers(driversRes.data || []);
      } catch {
        // fallback
      }
      try {
        const tripsRes = await api.get('/trips');
        setTrips(tripsRes.data || []);
      } catch {
        // fallback
      }
    } finally {
      setLoading(false);
    }
  };

  const showSuccess = (msg: string) => {
    setMessage(msg);
    setTimeout(() => setMessage(null), 3000);
  };

  // Bus Handlers
  const handleCreateBus = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await busService.createBus({
        bus_number: newBus.bus_number,
        registration_number: newBus.registration_number,
        capacity: Number(newBus.capacity),
        status: 'active',
      });
      setBuses((prev) => [...prev, res]);
      setNewBus({ bus_number: '', registration_number: '', capacity: 45 });
      showSuccess('Bus created successfully');
    } catch (err: any) {
      alert(err?.response?.data?.message || err?.message || 'Failed to create bus');
    }
  };

  const handleDeleteBus = async (id: number) => {
    if (!confirm('Are you sure you want to delete this bus?')) return;
    try {
      await busService.deleteBus(id);
      setBuses((prev) => prev.filter((b) => b.id !== id));
      showSuccess('Bus deleted');
    } catch (err: any) {
      alert(err?.message || 'Failed to delete bus');
    }
  };

  // Route Handlers
  const handleCreateRoute = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await routeService.createRoute({
        route_name: newRoute.route_name,
        route_code: newRoute.route_code,
        start_point: newRoute.start_point,
        end_point: newRoute.end_point,
        status: 'active',
      });
      setRoutes((prev) => [...prev, res]);
      setNewRoute({ route_name: '', route_code: '', start_point: '', end_point: '' });
      showSuccess('Route created successfully');
    } catch (err: any) {
      alert(err?.message || 'Failed to create route');
    }
  };

  const handleDeleteRoute = async (id: number) => {
    if (!confirm('Are you sure you want to delete this route?')) return;
    try {
      await routeService.deleteRoute(id);
      setRoutes((prev) => prev.filter((r) => r.id !== id));
      showSuccess('Route deleted');
    } catch (err: any) {
      alert(err?.message || 'Failed to delete route');
    }
  };

  // Stop Handlers
  const handleCreateStop = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await stopService.createStop({
        stop_name: newStop.stop_name,
        latitude: Number(newStop.latitude),
        longitude: Number(newStop.longitude),
      });
      setStops((prev) => [...prev, res]);
      setNewStop({ stop_name: '', latitude: 30.3165, longitude: 78.0322 });
      showSuccess('Stop created successfully');
    } catch (err: any) {
      alert(err?.message || 'Failed to create stop');
    }
  };

  const handleDeleteStop = async (id: number) => {
    if (!confirm('Are you sure you want to delete this stop?')) return;
    try {
      await stopService.deleteStop(id);
      setStops((prev) => prev.filter((s) => s.id !== id));
      showSuccess('Stop deleted');
    } catch (err: any) {
      alert(err?.message || 'Failed to delete stop');
    }
  };

  // Alert Handlers
  const handleCreateAlert = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await alertService.createAlert({
        message: newAlert.message,
        severity: newAlert.severity,
        route_id: newAlert.route_id ? Number(newAlert.route_id) : null,
      });
      setAlerts((prev) => [...prev, res]);
      setNewAlert({ message: '', severity: 'info', route_id: '' });
      showSuccess('Alert broadcasted successfully');
    } catch (err: any) {
      alert(err?.message || 'Failed to create alert');
    }
  };

  const handleDeleteAlert = async (id: number) => {
    try {
      await alertService.deleteAlert(id);
      setAlerts((prev) => prev.filter((a) => a.id !== id));
      showSuccess('Alert removed');
    } catch (err: any) {
      alert(err?.message || 'Failed to remove alert');
    }
  };

  // Trip Handlers
  const handleCreateTrip = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      const res = await api.post('/trips', {
        bus_id: Number(newTrip.bus_id),
        route_id: Number(newTrip.route_id),
        driver_id: Number(newTrip.driver_id),
        status: 'scheduled',
      });
      setTrips((prev) => [...prev, res.data]);
      setNewTrip({ bus_id: '', route_id: '', driver_id: '' });
      showSuccess('Trip scheduled successfully');
    } catch (err: any) {
      alert(err?.message || 'Failed to schedule trip');
    }
  };

  return (
    <div style={{ padding: 'var(--space-2xl)', maxWidth: '1200px', margin: '0 auto', width: '100%', overflowY: 'auto' }}>
      {/* Header */}
      <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-xl)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-md)' }}>
          <div
            style={{
              padding: 'var(--space-sm)',
              backgroundColor: 'rgba(37, 161, 142, 0.1)',
              borderRadius: 'var(--radius-md)',
              color: 'var(--accent-primary)',
            }}
          >
            <ShieldAlert size={24} />
          </div>
          <div>
            <h1 style={{ fontSize: 'var(--font-size-2xl)', fontWeight: 'var(--font-weight-bold)' }}>
              TransitX Operations Control Center
            </h1>
            <p style={{ fontSize: 'var(--font-size-sm)', color: 'var(--text-secondary)' }}>
              Admin management dashboard for fleet, routes, stops, schedules, and alerts
            </p>
          </div>
        </div>

        {message && (
          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: 'var(--space-xs)',
              padding: 'var(--space-sm) var(--space-md)',
              backgroundColor: 'rgba(85, 168, 138, 0.1)',
              border: '1px solid var(--color-success)',
              borderRadius: 'var(--radius-md)',
              color: 'var(--color-success)',
              fontSize: 'var(--font-size-sm)',
            }}
          >
            <Check size={16} />
            {message}
          </div>
        )}
      </div>

      {/* Tabs */}
      <div className="tabs" style={{ marginBottom: 'var(--space-xl)' }}>
        {[
          { key: 'buses', label: 'Buses', icon: BusIcon },
          { key: 'routes', label: 'Routes', icon: RouteIcon },
          { key: 'stops', label: 'Stops', icon: MapPin },
          { key: 'drivers', label: 'Drivers', icon: Users },
          { key: 'trips', label: 'Trips', icon: Calendar },
          { key: 'alerts', label: 'Alerts', icon: AlertTriangle },
        ].map((t) => {
          const Icon = t.icon;
          return (
            <button
              key={t.key}
              type="button"
              className={`tab ${activeTab === t.key ? 'active' : ''}`}
              onClick={() => setActiveTab(t.key as any)}
              style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}
            >
              <Icon size={16} />
              {t.label}
            </button>
          );
        })}
      </div>

      {loading ? (
        <LoadingSpinner text="Loading telemetry & registry data..." />
      ) : (
        <>
          {/* BUSES TAB */}
          {activeTab === 'buses' && (
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: 'var(--space-xl)' }}>
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Fleet Vehicles ({buses.length})
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
                  {buses.map((bus) => (
                    <div
                      key={bus.id}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: 'var(--space-sm) var(--space-md)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        border: '1px solid var(--border-color)',
                      }}
                    >
                      <div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)' }}>Bus {bus.bus_number}</div>
                        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          Reg: {bus.registration_number || 'N/A'} • Capacity: {bus.capacity}
                        </div>
                      </div>
                      <button
                        type="button"
                        onClick={() => handleDeleteBus(bus.id)}
                        className="btn btn-sm btn-danger"
                      >
                        <Trash2 size={14} />
                      </button>
                    </div>
                  ))}
                </div>
              </div>

              {/* Add Bus Form */}
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Add New Bus
                </h3>
                <form onSubmit={handleCreateBus} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
                  <div className="input-group">
                    <label>Bus Number</label>
                    <input
                      type="text"
                      placeholder="e.g. UK07PA7009"
                      value={newBus.bus_number}
                      onChange={(e) => setNewBus({ ...newBus, bus_number: e.target.value })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Registration Number</label>
                    <input
                      type="text"
                      placeholder="e.g. UK-07-PA-7009"
                      value={newBus.registration_number}
                      onChange={(e) => setNewBus({ ...newBus, registration_number: e.target.value })}
                    />
                  </div>
                  <div className="input-group">
                    <label>Seating Capacity</label>
                    <input
                      type="number"
                      value={newBus.capacity}
                      onChange={(e) => setNewBus({ ...newBus, capacity: Number(e.target.value) })}
                    />
                  </div>
                  <button type="submit" className="btn btn-primary-filled">
                    <Plus size={16} /> Add Bus
                  </button>
                </form>
              </div>
            </div>
          )}

          {/* ROUTES TAB */}
          {activeTab === 'routes' && (
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: 'var(--space-xl)' }}>
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Transit Routes ({routes.length})
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
                  {routes.map((route) => (
                    <div
                      key={route.id}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: 'var(--space-sm) var(--space-md)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        border: '1px solid var(--border-color)',
                      }}
                    >
                      <div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)' }}>
                          <span style={{ color: 'var(--accent-primary)', marginRight: '8px' }}>{route.route_code}</span>
                          {route.route_name}
                        </div>
                        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          {route.start_point} → {route.end_point}
                        </div>
                      </div>
                      <button
                        type="button"
                        onClick={() => handleDeleteRoute(route.id)}
                        className="btn btn-sm btn-danger"
                      >
                        <Trash2 size={14} />
                      </button>
                    </div>
                  ))}
                </div>
              </div>

              {/* Add Route Form */}
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Create Route
                </h3>
                <form onSubmit={handleCreateRoute} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
                  <div className="input-group">
                    <label>Route Code</label>
                    <input
                      type="text"
                      placeholder="e.g. DR006"
                      value={newRoute.route_code}
                      onChange={(e) => setNewRoute({ ...newRoute, route_code: e.target.value })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Route Name</label>
                    <input
                      type="text"
                      placeholder="e.g. Clock Tower to Premnagar"
                      value={newRoute.route_name}
                      onChange={(e) => setNewRoute({ ...newRoute, route_name: e.target.value })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Start Point</label>
                    <input
                      type="text"
                      placeholder="Clock Tower"
                      value={newRoute.start_point}
                      onChange={(e) => setNewRoute({ ...newRoute, start_point: e.target.value })}
                    />
                  </div>
                  <div className="input-group">
                    <label>End Point</label>
                    <input
                      type="text"
                      placeholder="Premnagar"
                      value={newRoute.end_point}
                      onChange={(e) => setNewRoute({ ...newRoute, end_point: e.target.value })}
                    />
                  </div>
                  <button type="submit" className="btn btn-primary-filled">
                    <Plus size={16} /> Create Route
                  </button>
                </form>
              </div>
            </div>
          )}

          {/* STOPS TAB */}
          {activeTab === 'stops' && (
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: 'var(--space-xl)' }}>
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Transit Stops ({stops.length})
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)', maxHeight: '600px', overflowY: 'auto' }}>
                  {stops.map((stop) => (
                    <div
                      key={stop.id}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: 'var(--space-sm) var(--space-md)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        border: '1px solid var(--border-color)',
                      }}
                    >
                      <div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)' }}>{stop.stop_name}</div>
                        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          {stop.latitude.toFixed(5)}, {stop.longitude.toFixed(5)}
                        </div>
                      </div>
                      <button
                        type="button"
                        onClick={() => handleDeleteStop(stop.id)}
                        className="btn btn-sm btn-danger"
                      >
                        <Trash2 size={14} />
                      </button>
                    </div>
                  ))}
                </div>
              </div>

              {/* Add Stop Form */}
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Add New Stop
                </h3>
                <form onSubmit={handleCreateStop} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
                  <div className="input-group">
                    <label>Stop Name</label>
                    <input
                      type="text"
                      placeholder="e.g. Saharanpur Chowk"
                      value={newStop.stop_name}
                      onChange={(e) => setNewStop({ ...newStop, stop_name: e.target.value })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Latitude</label>
                    <input
                      type="number"
                      step="0.0001"
                      value={newStop.latitude}
                      onChange={(e) => setNewStop({ ...newStop, latitude: Number(e.target.value) })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Longitude</label>
                    <input
                      type="number"
                      step="0.0001"
                      value={newStop.longitude}
                      onChange={(e) => setNewStop({ ...newStop, longitude: Number(e.target.value) })}
                      required
                    />
                  </div>
                  <button type="submit" className="btn btn-primary-filled">
                    <Plus size={16} /> Add Stop
                  </button>
                </form>
              </div>
            </div>
          )}

          {/* DRIVERS TAB */}
          {activeTab === 'drivers' && (
            <div className="card">
              <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                Registered Drivers ({drivers.length})
              </h3>
              <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
                {drivers.length === 0 ? (
                  <div style={{ color: 'var(--text-muted)', fontSize: 'var(--font-size-sm)' }}>
                    No driver accounts found. Drivers register through the driver portal.
                  </div>
                ) : (
                  drivers.map((drv) => (
                    <div
                      key={drv.id}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: 'var(--space-md)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        border: '1px solid var(--border-color)',
                      }}
                    >
                      <div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)' }}>{drv.user?.name || `Driver #${drv.id}`}</div>
                        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          License: {drv.license_number} • Email: {drv.user?.email || 'N/A'}
                        </div>
                      </div>
                      <span className="badge badge-live">Verified Driver</span>
                    </div>
                  ))
                )}
              </div>
            </div>
          )}

          {/* TRIPS TAB */}
          {activeTab === 'trips' && (
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: 'var(--space-xl)' }}>
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Active & Scheduled Trips ({trips.length})
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
                  {trips.length === 0 ? (
                    <div style={{ color: 'var(--text-muted)', fontSize: 'var(--font-size-sm)' }}>
                      No trips currently scheduled. Use the form to assign a bus and route to a driver.
                    </div>
                  ) : (
                    trips.map((trip) => (
                      <div
                        key={trip.id}
                        style={{
                          display: 'flex',
                          alignItems: 'center',
                          justifyContent: 'space-between',
                          padding: 'var(--space-md)',
                          backgroundColor: 'var(--bg-surface)',
                          borderRadius: 'var(--radius-md)',
                          border: '1px solid var(--border-color)',
                        }}
                      >
                        <div>
                          <div style={{ fontWeight: 'var(--font-weight-semibold)' }}>
                            Bus #{trip.bus_id} on Route #{trip.route_id}
                          </div>
                          <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                            Driver ID: {trip.driver_id} • Status: {trip.status.toUpperCase()}
                          </div>
                        </div>
                        <span className={`badge ${trip.status === 'active' ? 'badge-live' : 'badge-stale'}`}>
                          {trip.status}
                        </span>
                      </div>
                    ))
                  )}
                </div>
              </div>

              {/* Schedule Trip Form */}
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Schedule New Trip
                </h3>
                <form onSubmit={handleCreateTrip} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
                  <div className="input-group">
                    <label>Select Bus</label>
                    <select
                      value={newTrip.bus_id}
                      onChange={(e) => setNewTrip({ ...newTrip, bus_id: e.target.value })}
                      required
                    >
                      <option value="">-- Choose Bus --</option>
                      {buses.map((b) => (
                        <option key={b.id} value={b.id}>Bus {b.bus_number}</option>
                      ))}
                    </select>
                  </div>

                  <div className="input-group">
                    <label>Select Route</label>
                    <select
                      value={newTrip.route_id}
                      onChange={(e) => setNewTrip({ ...newTrip, route_id: e.target.value })}
                      required
                    >
                      <option value="">-- Choose Route --</option>
                      {routes.map((r) => (
                        <option key={r.id} value={r.id}>{r.route_code}: {r.route_name}</option>
                      ))}
                    </select>
                  </div>

                  <div className="input-group">
                    <label>Driver ID</label>
                    <input
                      type="number"
                      placeholder="Driver user ID"
                      value={newTrip.driver_id}
                      onChange={(e) => setNewTrip({ ...newTrip, driver_id: e.target.value })}
                      required
                    />
                  </div>

                  <button type="submit" className="btn btn-primary-filled">
                    <Plus size={16} /> Schedule Trip
                  </button>
                </form>
              </div>
            </div>
          )}

          {/* ALERTS TAB */}
          {activeTab === 'alerts' && (
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: 'var(--space-xl)' }}>
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Live System Alerts ({alerts.length})
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-sm)' }}>
                  {alerts.map((alert) => (
                    <div
                      key={alert.id}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: 'var(--space-sm) var(--space-md)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        border: '1px solid var(--border-color)',
                      }}
                    >
                      <div>
                        <div style={{ fontWeight: 'var(--font-weight-medium)' }}>{alert.message}</div>
                        <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          Severity: {alert.severity.toUpperCase()} • Route: {alert.route_id ? `Route #${alert.route_id}` : 'All Lines'}
                        </div>
                      </div>
                      <button
                        type="button"
                        onClick={() => handleDeleteAlert(alert.id)}
                        className="btn btn-sm btn-danger"
                      >
                        <Trash2 size={14} />
                      </button>
                    </div>
                  ))}
                </div>
              </div>

              {/* Broadcast Alert Form */}
              <div className="card">
                <h3 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-semibold)', marginBottom: 'var(--space-lg)' }}>
                  Broadcast Alert
                </h3>
                <form onSubmit={handleCreateAlert} style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
                  <div className="input-group">
                    <label>Alert Message</label>
                    <textarea
                      rows={3}
                      placeholder="e.g. Construction delay on Rajpur Road"
                      value={newAlert.message}
                      onChange={(e) => setNewAlert({ ...newAlert, message: e.target.value })}
                      required
                    />
                  </div>
                  <div className="input-group">
                    <label>Severity Level</label>
                    <select
                      value={newAlert.severity}
                      onChange={(e) => setNewAlert({ ...newAlert, severity: e.target.value as any })}
                    >
                      <option value="info">Info</option>
                      <option value="warning">Warning</option>
                      <option value="critical">Critical</option>
                    </select>
                  </div>
                  <div className="input-group">
                    <label>Target Route (Optional)</label>
                    <select
                      value={newAlert.route_id}
                      onChange={(e) => setNewAlert({ ...newAlert, route_id: e.target.value })}
                    >
                      <option value="">All Routes (System-wide)</option>
                      {routes.map((r) => (
                        <option key={r.id} value={r.id}>{r.route_code}: {r.route_name}</option>
                      ))}
                    </select>
                  </div>
                  <button type="submit" className="btn btn-primary-filled">
                    <Plus size={16} /> Broadcast Alert
                  </button>
                </form>
              </div>
            </div>
          )}
        </>
      )}
    </div>
  );
};
