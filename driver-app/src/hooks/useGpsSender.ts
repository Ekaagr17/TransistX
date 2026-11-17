import { useState, useEffect, useRef, useCallback } from 'react';
import { gpsService, GpsPayload } from '../services/gps.service';
import { GeolocationPositionData } from './useGeolocation';

export type ConnectionStatusType = 'CONNECTED' | 'DISCONNECTED' | 'RECONNECTING';

export interface UseGpsSenderProps {
  busId?: number;
  tripId?: number;
  isTracking: boolean;
  onSuccess?: () => void;
  onError?: (err: string) => void;
}

export interface UseGpsSenderReturn {
  connectionStatus: ConnectionStatusType;
  lastTransmission: Date | null;
  queueLength: number;
  isSending: boolean;
  sendGpsUpdate: (pos: GeolocationPositionData) => Promise<boolean>;
}

export function useGpsSender({
  busId,
  tripId,
  isTracking,
  onSuccess,
  onError,
}: UseGpsSenderProps): UseGpsSenderReturn {
  const [connectionStatus, setConnectionStatus] = useState<ConnectionStatusType>('CONNECTED');
  const [lastTransmission, setLastTransmission] = useState<Date | null>(null);
  const [queueLength, setQueueLength] = useState<number>(0);
  const [isSending, setIsSending] = useState<boolean>(false);

  const queueRef = useRef<GpsPayload[]>([]);
  const isFlushingRef = useRef<boolean>(false);
  const retryCountRef = useRef<number>(0);
  const retryTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  // Monitor online / offline browser events
  useEffect(() => {
    const handleOnline = () => {
      setConnectionStatus('RECONNECTING');
      triggerFlush();
    };

    const handleOffline = () => {
      setConnectionStatus('DISCONNECTED');
    };

    window.addEventListener('online', handleOnline);
    window.addEventListener('offline', handleOffline);

    return () => {
      window.removeEventListener('online', handleOnline);
      window.removeEventListener('offline', handleOffline);
      if (retryTimerRef.current) {
        clearTimeout(retryTimerRef.current);
      }
    };
  }, []);

  const triggerFlush = useCallback(async () => {
    if (isFlushingRef.current || queueRef.current.length === 0) {
      if (queueRef.current.length === 0 && navigator.onLine) {
        setConnectionStatus('CONNECTED');
      }
      return;
    }

    if (!navigator.onLine) {
      setConnectionStatus('DISCONNECTED');
      return;
    }

    isFlushingRef.current = true;
    setConnectionStatus('RECONNECTING');

    while (queueRef.current.length > 0) {
      const payload = queueRef.current[0];
      try {
        await gpsService.sendUpdate(payload);
        queueRef.current.shift();
        setQueueLength(queueRef.current.length);
        setLastTransmission(new Date());
        retryCountRef.current = 0;
        setConnectionStatus('CONNECTED');
      } catch {
        // Failed to send, apply exponential backoff
        retryCountRef.current++;
        const backoffMs = Math.min(1000 * Math.pow(2, retryCountRef.current), 15000);
        setConnectionStatus('DISCONNECTED');
        isFlushingRef.current = false;

        retryTimerRef.current = setTimeout(() => {
          triggerFlush();
        }, backoffMs);
        return;
      }
    }

    isFlushingRef.current = false;
    setConnectionStatus('CONNECTED');
  }, []);

  const sendGpsUpdate = useCallback(
    async (pos: GeolocationPositionData): Promise<boolean> => {
      if (!isTracking || !busId || !tripId) {
        return false;
      }

      // Validate accuracy
      if (pos.accuracy > 200) {
        return false;
      }

      const payload: GpsPayload = {
        latitude: pos.latitude,
        longitude: pos.longitude,
        speed: pos.speed,
        heading: pos.heading,
        accuracy: pos.accuracy,
        timestamp: pos.timestamp,
        bus_id: busId,
        trip_id: tripId,
      };

      // If already offline, push to queue directly
      if (!navigator.onLine) {
        queueRef.current.push(payload);
        // keep queue within reasonable bounds (e.g. 50 items)
        if (queueRef.current.length > 50) {
          queueRef.current.shift();
        }
        setQueueLength(queueRef.current.length);
        setConnectionStatus('DISCONNECTED');
        return false;
      }

      // If items exist in queue, enqueue this one too and flush
      if (queueRef.current.length > 0) {
        queueRef.current.push(payload);
        if (queueRef.current.length > 50) {
          queueRef.current.shift();
        }
        setQueueLength(queueRef.current.length);
        triggerFlush();
        return false;
      }

      setIsSending(true);
      try {
        await gpsService.sendUpdate(payload);
        setLastTransmission(new Date());
        setConnectionStatus('CONNECTED');
        retryCountRef.current = 0;
        onSuccess?.();
        return true;
      } catch (err: unknown) {
        const errorMsg =
          err instanceof Error
            ? err.message
            : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
              'Failed to transmit GPS update';

        setConnectionStatus('DISCONNECTED');
        onError?.(errorMsg);

        // Queue for retry
        queueRef.current.push(payload);
        setQueueLength(queueRef.current.length);

        const backoffMs = Math.min(1000 * Math.pow(2, retryCountRef.current), 15000);
        retryCountRef.current++;
        if (retryTimerRef.current) {
          clearTimeout(retryTimerRef.current);
        }
        retryTimerRef.current = setTimeout(() => {
          triggerFlush();
        }, backoffMs);

        return false;
      } finally {
        setIsSending(false);
      }
    },
    [isTracking, busId, tripId, onSuccess, onError, triggerFlush]
  );

  return {
    connectionStatus,
    lastTransmission,
    queueLength,
    isSending,
    sendGpsUpdate,
  };
}
