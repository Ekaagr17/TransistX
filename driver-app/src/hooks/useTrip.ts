import { useState, useEffect, useRef, useCallback } from 'react';
import { tripService, Trip, TripStatus, StartTripPayload } from '../services/trip.service';
import { GeolocationPositionData } from './useGeolocation';

// Calculate distance in kilometers using Haversine formula
function calculateDistanceKm(lat1: number, lon1: number, lat2: number, lon2: number): number {
  const R = 6371; // Earth's radius in km
  const toRad = (deg: number) => (deg * Math.PI) / 180;
  const dLat = toRad(lat2 - lat1);
  const dLon = toRad(lon2 - lon1);
  const a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon / 2) * Math.sin(dLon / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

export interface UseTripReturn {
  currentTrip: Trip | null;
  tripStatus: TripStatus;
  durationSeconds: number;
  formattedDuration: string;
  distanceKm: number;
  updatesSentCount: number;
  isLoading: boolean;
  error: string | null;
  startTrip: (payload: StartTripPayload) => Promise<Trip>;
  pauseTrip: () => Promise<void>;
  resumeTrip: () => Promise<void>;
  endTrip: () => Promise<void>;
  recordPositionForDistance: (pos: GeolocationPositionData) => void;
  incrementUpdatesSent: () => void;
  clearError: () => void;
}

export function useTrip(): UseTripReturn {
  const [currentTrip, setCurrentTrip] = useState<Trip | null>(() => {
    const saved = localStorage.getItem('driver_current_trip');
    if (saved) {
      try {
        return JSON.parse(saved);
      } catch {
        return null;
      }
    }
    return null;
  });

  const [tripStatus, setTripStatus] = useState<TripStatus>(() => {
    const saved = localStorage.getItem('driver_trip_status');
    return (saved as TripStatus) || 'idle';
  });

  const [durationSeconds, setDurationSeconds] = useState<number>(0);
  const [distanceKm, setDistanceKm] = useState<number>(0);
  const [updatesSentCount, setUpdatesSentCount] = useState<number>(0);
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  const prevCoordRef = useRef<{ lat: number; lon: number } | null>(null);
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);

  // Sync state to localStorage
  useEffect(() => {
    if (currentTrip) {
      localStorage.setItem('driver_current_trip', JSON.stringify(currentTrip));
    } else {
      localStorage.removeItem('driver_current_trip');
    }
    localStorage.setItem('driver_trip_status', tripStatus);
  }, [currentTrip, tripStatus]);

  // Duration timer when trip is active
  useEffect(() => {
    if (tripStatus === 'active') {
      timerRef.current = setInterval(() => {
        setDurationSeconds((prev) => prev + 1);
      }, 1000);
    } else if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
      }
    };
  }, [tripStatus]);

  const formatDuration = (totalSec: number): string => {
    const hours = Math.floor(totalSec / 3600);
    const minutes = Math.floor((totalSec % 3600) / 60);
    const seconds = totalSec % 60;

    const pad = (n: number) => n.toString().padStart(2, '0');
    if (hours > 0) {
      return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
    }
    return `${pad(minutes)}:${pad(seconds)}`;
  };

  const recordPositionForDistance = useCallback((pos: GeolocationPositionData) => {
    if (!prevCoordRef.current) {
      prevCoordRef.current = { lat: pos.latitude, lon: pos.longitude };
      return;
    }

    const delta = calculateDistanceKm(
      prevCoordRef.current.lat,
      prevCoordRef.current.lon,
      pos.latitude,
      pos.longitude
    );

    // Filter unrealistic jumps (> 150 km/h in 3s is > 0.125 km)
    if (delta > 0.001 && delta < 1.0) {
      setDistanceKm((prev) => prev + delta);
      prevCoordRef.current = { lat: pos.latitude, lon: pos.longitude };
    }
  }, []);

  const incrementUpdatesSent = useCallback(() => {
    setUpdatesSentCount((prev) => prev + 1);
  }, []);

  const startTrip = useCallback(async (payload: StartTripPayload): Promise<Trip> => {
    setIsLoading(true);
    setError(null);
    try {
      const trip = await tripService.startTrip(payload);
      setCurrentTrip(trip);
      setTripStatus('active');
      setDurationSeconds(0);
      setDistanceKm(0);
      setUpdatesSentCount(0);
      prevCoordRef.current = null;
      return trip;
    } catch (err: unknown) {
      const message =
        err instanceof Error
          ? err.message
          : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
            'Failed to start trip';
      setError(message);
      throw err;
    } finally {
      setIsLoading(false);
    }
  }, []);

  const pauseTrip = useCallback(async () => {
    if (!currentTrip) return;
    setIsLoading(true);
    setError(null);
    try {
      await tripService.pauseTrip(currentTrip.id);
      setTripStatus('paused');
    } catch (err: unknown) {
      const message =
        err instanceof Error
          ? err.message
          : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
            'Failed to pause trip';
      setError(message);
    } finally {
      setIsLoading(false);
    }
  }, [currentTrip]);

  const resumeTrip = useCallback(async () => {
    if (!currentTrip) return;
    setIsLoading(true);
    setError(null);
    try {
      await tripService.resumeTrip(currentTrip.id);
      setTripStatus('active');
    } catch (err: unknown) {
      const message =
        err instanceof Error
          ? err.message
          : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
            'Failed to resume trip';
      setError(message);
    } finally {
      setIsLoading(false);
    }
  }, [currentTrip]);

  const endTrip = useCallback(async () => {
    if (!currentTrip) return;
    setIsLoading(true);
    setError(null);
    try {
      await tripService.endTrip(currentTrip.id);
      setTripStatus('ended');
      setTimeout(() => {
        setTripStatus('idle');
        setCurrentTrip(null);
      }, 3000);
    } catch (err: unknown) {
      const message =
        err instanceof Error
          ? err.message
          : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
            'Failed to end trip';
      setError(message);
    } finally {
      setIsLoading(false);
    }
  }, [currentTrip]);

  const clearError = useCallback(() => setError(null), []);

  return {
    currentTrip,
    tripStatus,
    durationSeconds,
    formattedDuration: formatDuration(durationSeconds),
    distanceKm: Math.round(distanceKm * 100) / 100,
    updatesSentCount,
    isLoading,
    error,
    startTrip,
    pauseTrip,
    resumeTrip,
    endTrip,
    recordPositionForDistance,
    incrementUpdatesSent,
    clearError,
  };
}
