import type { BusLocation, WebSocketMessage } from '../types';

type EventHandler = (data: BusLocation) => void;

class WebSocketService {
  private ws: WebSocket | null = null;
  private url: string;
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 10;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  private listeners: Map<string, Set<EventHandler>> = new Map();
  private subscribedBuses: Set<number> = new Set();
  private subscribedRoutes: Set<number> = new Set();

  constructor(url: string = `ws://${window.location.host}/ws/live-buses`) {
    this.url = url;
  }

  connect(): void {
    if (this.ws?.readyState === WebSocket.OPEN) return;

    try {
      this.ws = new WebSocket(this.url);

      this.ws.onopen = () => {
        this.reconnectAttempts = 0;
        this.resubscribe();
        this.emit('connected', {} as BusLocation);
      };

      this.ws.onmessage = (event) => {
        try {
          const message: WebSocketMessage = JSON.parse(event.data);
          if (message.type === 'location_update') {
            const loc = message.data as BusLocation;
            this.emit('location_update', loc);
            this.emit(`bus:${loc.bus_id}`, loc);
          }
        } catch {
          // ignore malformed messages
        }
      };

      this.ws.onclose = () => {
        this.emit('disconnected', {} as BusLocation);
        this.scheduleReconnect();
      };

      this.ws.onerror = () => {
        this.ws?.close();
      };
    } catch {
      this.scheduleReconnect();
    }
  }

  disconnect(): void {
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    this.reconnectAttempts = this.maxReconnectAttempts;
    this.ws?.close();
    this.ws = null;
  }

  subscribeBus(busId: number): void {
    this.subscribedBuses.add(busId);
    this.sendSubscription('subscribe_bus', busId);
  }

  unsubscribeBus(busId: number): void {
    this.subscribedBuses.delete(busId);
    this.sendSubscription('unsubscribe_bus', busId);
  }

  subscribeRoute(routeId: number): void {
    this.subscribedRoutes.add(routeId);
    this.sendSubscription('subscribe_route', routeId);
  }

  unsubscribeRoute(routeId: number): void {
    this.subscribedRoutes.delete(routeId);
    this.sendSubscription('unsubscribe_route', routeId);
  }

  on(event: string, handler: EventHandler): () => void {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, new Set());
    }
    this.listeners.get(event)!.add(handler);
    return () => {
      this.listeners.get(event)?.delete(handler);
    };
  }

  private emit(event: string, data: BusLocation): void {
    this.listeners.get(event)?.forEach((handler) => handler(data));
  }

  private sendSubscription(type: string, id: number): void {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify({ type, id }));
    }
  }

  private resubscribe(): void {
    this.subscribedBuses.forEach((id) => this.sendSubscription('subscribe_bus', id));
    this.subscribedRoutes.forEach((id) => this.sendSubscription('subscribe_route', id));
  }

  private scheduleReconnect(): void {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) return;
    const delay = Math.min(1000 * Math.pow(2, this.reconnectAttempts), 30000);
    this.reconnectAttempts++;
    this.reconnectTimer = setTimeout(() => this.connect(), delay);
  }
}

export const wsService = new WebSocketService();
