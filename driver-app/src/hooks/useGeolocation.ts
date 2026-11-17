import { useState, useEffect, useRef, useCallback } from 'react';

export type PermissionStatusType = 'granted' | 'denied' | 'prompt' | 'unknown';

export interface GeolocationPositionData {
  latitude: number;
  longitude: number;
  speed: number | null;
  heading: number | null;
  accuracy: number;
  timestamp: string;
}

export interface GeolocationState {
  position: GeolocationPositionData | null;
  accuracy: number | null;
  permissionStatus: PermissionStatusType;
  error: string | null;
  isTracking: boolean;
  lastUpdate: Date | null;
  isBackgrounded: boolean;
  isStale: boolean;
  startTracking: () => void;
  stopTracking: () => void;
}

// Calculate distance in meters using Haversine formula
function calculateDistance(lat1: number, lon1: number, lat2: number, lon2: number): number {
  const R = 6371e3; // Earth's radius in meters
  const toRad = (deg: number) => (deg * Math.PI) / 180;
  const dLat = toRad(lat2 - lat1);
  const dLon = toRad(lon2 - lon1);
  const a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon / 2) * Math.sin(dLon / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

export function useGeolocation(autoStart = false): GeolocationState {
  const [position, setPosition] = useState<GeolocationPositionData | null>(null);
  const [accuracy, setAccuracy] = useState<number | null>(null);
  const [permissionStatus, setPermissionStatus] = useState<PermissionStatusType>('unknown');
  const [error, setError] = useState<string | null>(null);
  const [isTracking, setIsTracking] = useState<boolean>(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);
  const [isBackgrounded, setIsBackgrounded] = useState<boolean>(
    typeof document !== 'undefined' ? document.visibilityState === 'hidden' : false
  );
  const [isStale, setIsStale] = useState<boolean>(false);

  const watchIdRef = useRef<number | null>(null);
  const lastProcessedPosRef = useRef<GeolocationPositionData | null>(null);
  const lastEmitTimeRef = useRef<number>(0);

  // Monitor browser permission status
  useEffect(() => {
    if ('permissions' in navigator && navigator.permissions?.query) {
      navigator.permissions
        .query({ name: 'geolocation' as PermissionName })
        .then((permissionObj) => {
          setPermissionStatus(permissionObj.state as PermissionStatusType);
          permissionObj.onchange = () => {
            setPermissionStatus(permissionObj.state as PermissionStatusType);
          };
        })
        .catch(() => {
          setPermissionStatus('unknown');
        });
    }
  }, []);

  // Monitor page visibility
  useEffect(() => {
    const handleVisibilityChange = () => {
      const hidden = document.visibilityState === 'hidden';
      setIsBackgrounded(hidden);
    };

    document.addEventListener('visibilitychange', handleVisibilityChange);
    return () => {
      document.removeEventListener('visibilitychange', handleVisibilityChange);
    };
  }, []);

  // Monitor staleness (> 30s without update while tracking)
  useEffect(() => {
    const interval = setInterval(() => {
      if (isTracking && lastUpdate) {
        const elapsedMs = Date.now() - lastUpdate.getTime();
        setIsStale(elapsedMs > 30000);
      } else {
        setIsStale(false);
      }
    }, 2000);

    return () => clearInterval(interval);
  }, [isTracking, lastUpdate]);

  const handlePositionSuccess = useCallback((pos: GeolocationPosition) => {
    const { latitude, longitude, speed, heading, accuracy: rawAccuracy } = pos.coords;
    setAccuracy(rawAccuracy);
    setPermissionStatus('granted');

    // Reject poor accuracy > 200m
    if (rawAccuracy > 200) {
      setError(`GPS accuracy too low (${Math.round(rawAccuracy)}m > 200m). Searching for better signal...`);
      return;
    }

    const now = Date.now();
    const timeSinceLastEmit = now - lastEmitTimeRef.current;

    // Check duplicate or minimal movement
    const prevPos = lastProcessedPosRef.current;
    if (prevPos) {
      const dist = calculateDistance(prevPos.latitude, prevPos.longitude, latitude, longitude);
      // If position changed < 1m and emitted less than 5s ago, debounce/skip to avoid redundant network updates
      if (dist < 1.0 && timeSinceLastEmit < 5000) {
        return;
      }
    }

    // Rate-limit updates to at least every 3 seconds unless first position
    if (prevPos && timeSinceLastEmit < 3000) {
      return;
    }

    const newPosData: GeolocationPositionData = {
      latitude,
      longitude,
      speed: speed !== null && !isNaN(speed) ? Math.max(0, speed) : null,
      heading: heading !== null && !isNaN(heading) ? heading : null,
      accuracy: rawAccuracy,
      timestamp: new Date(pos.timestamp).toISOString(),
    };

    lastProcessedPosRef.current = newPosData;
    lastEmitTimeRef.current = now;
    const updateTime = new Date();

    setPosition(newPosData);
    setLastUpdate(updateTime);
    setIsStale(false);
    setError(null);
  }, []);

  const handlePositionError = useCallback((err: GeolocationPositionError) => {
    switch (err.code) {
      case err.PERMISSION_DENIED:
        setPermissionStatus('denied');
        setError('Location permission denied. Please allow GPS access in settings.');
        break;
      case err.POSITION_UNAVAILABLE:
        setError('GPS position unavailable. Please ensure device location is enabled.');
        break;
      case err.TIMEOUT:
        setError('GPS request timed out. Weak satellite signal.');
        break;
      default:
        setError(err.message || 'Unknown GPS error occurred.');
        break;
    }
  }, []);

  const startTracking = useCallback(() => {
    if (!('geolocation' in navigator)) {
      setError('Geolocation is not supported by your browser.');
      return;
    }

    if (watchIdRef.current !== null) {
      navigator.geolocation.clearWatch(watchIdRef.current);
      watchIdRef.current = null;
    }

    setError(null);
    setIsTracking(true);

    try {
      const id = navigator.geolocation.watchPosition(
        handlePositionSuccess,
        handlePositionError,
        {
          enableHighAccuracy: true,
          timeout: 10000,
          maximumAge: 0,
        }
      );
      watchIdRef.current = id;
    } catch (e: unknown) {
      const msg = e instanceof Error ? e.message : 'Failed to start GPS tracking';
      setError(msg);
      setIsTracking(false);
    }
  }, [handlePositionSuccess, handlePositionError]);

  const stopTracking = useCallback(() => {
    if (watchIdRef.current !== null) {
      navigator.geolocation.clearWatch(watchIdRef.current);
      watchIdRef.current = null;
    }
    setIsTracking(false);
  }, []);

  useEffect(() => {
    if (autoStart) {
      startTracking();
    }
    return () => {
      if (watchIdRef.current !== null) {
        navigator.geolocation.clearWatch(watchIdRef.current);
      }
    };
  }, [autoStart, startTracking]);

  return {
    position,
    accuracy,
    permissionStatus,
    error,
    isTracking,
    lastUpdate,
    isBackgrounded,
    isStale,
    startTracking,
    stopTracking,
  };
}
