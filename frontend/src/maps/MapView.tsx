import React, { useRef } from 'react';
import 'maplibre-gl/dist/maplibre-gl.css';
import { useMap } from '../hooks/useMap';
import { BusMarker } from './BusMarker';
import { StopMarker } from './StopMarker';
import { RouteLayer } from './RouteLayer';
import type { BusLocation, Stop, RouteStop } from '../types';

interface MapViewProps {
  busLocations?: BusLocation[];
  stops?: Stop[];
  selectedBusId?: number | null;
  selectedStopId?: number | null;
  selectedRouteStops?: RouteStop[];
  selectedRouteId?: number | null;
  onBusSelect?: (busId: number) => void;
  onStopSelect?: (stopId: number) => void;
  center?: [number, number];
  zoom?: number;
}

export const MapView: React.FC<MapViewProps> = ({
  busLocations = [],
  stops = [],
  selectedBusId,
  selectedStopId,
  selectedRouteStops = [],
  selectedRouteId,
  onBusSelect,
  onStopSelect,
  center = [78.0322, 30.3165],
  zoom = 13,
}) => {
  const mapContainerRef = useRef<HTMLDivElement>(null);
  const { map, isLoaded } = useMap(mapContainerRef, { center, zoom });

  return (
    <div style={{ position: 'relative', width: '100%', height: '100%' }}>
      <div
        ref={mapContainerRef}
        style={{ width: '100%', height: '100%', backgroundColor: '#171B20' }}
      />
      {isLoaded && map && (
        <>
          {/* Active Route Polylines */}
          {selectedRouteId && selectedRouteStops.length > 0 && (
            <RouteLayer
              map={map}
              routeId={selectedRouteId}
              routeStops={selectedRouteStops}
            />
          )}

          {/* Stops */}
          {stops.map((stop) => (
            <StopMarker
              key={stop.id}
              map={map}
              stop={stop}
              isSelected={stop.id === selectedStopId}
              onClick={onStopSelect}
            />
          ))}

          {/* Buses */}
          {busLocations.map((loc) => (
            <BusMarker
              key={loc.bus_id}
              map={map}
              location={loc}
              isSelected={loc.bus_id === selectedBusId}
              onClick={onBusSelect}
            />
          ))}
        </>
      )}
    </div>
  );
};
