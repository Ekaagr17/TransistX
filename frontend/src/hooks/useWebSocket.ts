import { useState, useEffect, useCallback } from 'react';
import { wsService } from '../services/websocket.service';
import type { BusLocation } from '../types';

export const useWebSocket = () => {
  const [connected, setConnected] = useState<boolean>(false);
  const [lastLocation, setLastLocation] = useState<BusLocation | null>(null);

  useEffect(() => {
    wsService.connect();

    const unsubConnected = wsService.on('connected', () => {
      setConnected(true);
    });

    const unsubDisconnected = wsService.on('disconnected', () => {
      setConnected(false);
    });

    const unsubLocation = wsService.on('location_update', (loc) => {
      setLastLocation(loc);
    });

    return () => {
      unsubConnected();
      unsubDisconnected();
      unsubLocation();
    };
  }, []);

  const subscribeBus = useCallback((busId: number) => {
    wsService.subscribeBus(busId);
  }, []);

  const unsubscribeBus = useCallback((busId: number) => {
    wsService.unsubscribeBus(busId);
  }, []);

  const subscribeRoute = useCallback((routeId: number) => {
    wsService.subscribeRoute(routeId);
  }, []);

  const unsubscribeRoute = useCallback((routeId: number) => {
    wsService.unsubscribeRoute(routeId);
  }, []);

  return {
    connected,
    lastLocation,
    subscribeBus,
    unsubscribeBus,
    subscribeRoute,
    unsubscribeRoute,
  };
};
