import React, { useEffect, useState, useCallback, useRef } from 'react';
import { useAuth } from '../hooks/useAuth';
import { useGeolocation, GeolocationPositionData } from '../hooks/useGeolocation';
import { useTrip } from '../hooks/useTrip';
import { useGpsSender } from '../hooks/useGpsSender';
import { DriverHeader } from '../components/DriverHeader';
import { StatusIndicator } from '../components/StatusIndicator';
import { ConnectionStatus } from '../components/ConnectionStatus';
import { GpsStatusCard } from '../components/GpsStatusCard';
import { TripInfoCard } from '../components/TripInfoCard';
import { TripControls } from '../components/TripControls';
import { TrackingWarning } from '../components/TrackingWarning';

export const DashboardPage: React.FC = () => {
  const { user, logout } = useAuth();

  // Active bus and route settings (either from user profile or demo defaults)
  const [busId] = useState<number>(user?.bus_id || 1);
  const [busNumber] = useState<string>(user?.bus_number || 'UK07PA7001');
  const [routeId] = useState<number>(user?.route_id || 1);
  const [routeName] = useState<string>(user?.route_name || 'Demo: Clock Tower - ISBT');

  // Geolocation hook
  const {
    position,
    accuracy,
    permissionStatus,
    error: geoError,
    isTracking,
    lastUpdate,
    isBackgrounded,
    isStale,
    startTracking,
    stopTracking,
  } = useGeolocation();

  // Trip hook
  const {
    currentTrip,
    tripStatus,
    formattedDuration,
    distanceKm,
    updatesSentCount,
    isLoading: isTripLoading,
    error: tripError,
    startTrip,
    pauseTrip,
    resumeTrip,
    endTrip,
    recordPositionForDistance,
    incrementUpdatesSent,
  } = useTrip();

  // GPS Sender hook
  const handleGpsSuccess = useCallback(() => {
    incrementUpdatesSent();
  }, [incrementUpdatesSent]);

  const {
    connectionStatus,
    lastTransmission,
    queueLength,
    sendGpsUpdate,
  } = useGpsSender({
    busId,
    tripId: currentTrip?.id,
    isTracking: tripStatus === 'active',
    onSuccess: handleGpsSuccess,
  });

  // Track position updates and send to backend
  const lastHandledTimestampRef = useRef<string | null>(null);

  useEffect(() => {
    if (position && tripStatus === 'active') {
      if (position.timestamp !== lastHandledTimestampRef.current) {
        lastHandledTimestampRef.current = position.timestamp;
        recordPositionForDistance(position);
        sendGpsUpdate(position);
      }
    }
  }, [position, tripStatus, recordPositionForDistance, sendGpsUpdate]);

  // Trip Control Handlers
  const handleStartTrip = async () => {
    try {
      await startTrip({ bus_id: busId, route_id: routeId });
      startTracking();
    } catch {
      // Error handled in useTrip
    }
  };

  const handlePauseTrip = async () => {
    await pauseTrip();
  };

  const handleResumeTrip = async () => {
    await resumeTrip();
    if (!isTracking) {
      startTracking();
    }
  };

  const handleEndTrip = async () => {
    await endTrip();
    stopTracking();
  };

  return (
    <div
      style={{
        minHeight: '100dvh',
        backgroundColor: '#171B20',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <DriverHeader driverName={user?.name || 'Driver'} onLogout={logout} />

      <main
        style={{
          flex: 1,
          width: '100%',
          maxWidth: '540px',
          margin: '0 auto',
          padding: '16px 12px 32px 12px',
          display: 'flex',
          flexDirection: 'column',
          gap: '12px',
        }}
      >
        {/* Tracking Warning Banners */}
        <TrackingWarning
          isBackgrounded={isBackgrounded}
          isStale={isStale}
          accuracy={accuracy}
          connectionStatus={connectionStatus}
          permissionStatus={permissionStatus}
          errorMessage={geoError || tripError}
        />

        {/* Prominent High-Level Status Indicator */}
        <StatusIndicator
          tripStatus={tripStatus}
          connectionStatus={connectionStatus}
          permissionStatus={permissionStatus}
          accuracy={accuracy}
          isStale={isStale}
          isBackgrounded={isBackgrounded}
        />

        {/* Backend Connection & Buffer Status */}
        <ConnectionStatus
          status={connectionStatus}
          lastTransmission={lastTransmission}
          queueLength={queueLength}
        />

        {/* GPS Hardware Telemetry Card */}
        <GpsStatusCard
          permissionStatus={permissionStatus}
          accuracy={accuracy}
          position={position}
          lastUpdate={lastUpdate}
          isTracking={isTracking}
        />

        {/* Vehicle Assignment & Progress Metrics */}
        <TripInfoCard
          busNumber={busNumber}
          routeName={routeName}
          duration={formattedDuration}
          distanceKm={distanceKm}
          updatesSent={updatesSentCount}
        />

        {/* Large Mobile Touch Controls */}
        <TripControls
          tripStatus={tripStatus}
          isLoading={isTripLoading}
          onStart={handleStartTrip}
          onPause={handlePauseTrip}
          onResume={handleResumeTrip}
          onEnd={handleEndTrip}
        />
      </main>
    </div>
  );
};
