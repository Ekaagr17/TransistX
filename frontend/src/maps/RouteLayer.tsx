import React, { useEffect } from 'react';
import maplibregl from 'maplibre-gl';
import type { RouteStop } from '../types';

interface RouteLayerProps {
  map: maplibregl.Map | null;
  routeId: number;
  routeStops: RouteStop[];
  activeStopIndex?: number;
}

export const RouteLayer: React.FC<RouteLayerProps> = ({
  map,
  routeId,
  routeStops,
  activeStopIndex = 0,
}) => {
  useEffect(() => {
    if (!map || routeStops.length < 2) return;

    // Filter valid stops with coordinates
    const sortedStops = [...routeStops]
      .filter((rs) => rs.stop && rs.stop.longitude && rs.stop.latitude)
      .sort((a, b) => a.stop_order - b.stop_order);

    if (sortedStops.length < 2) return;

    const coordinates = sortedStops.map((rs) => [rs.stop!.longitude, rs.stop!.latitude]);

    const completedCoords = coordinates.slice(0, Math.max(activeStopIndex + 1, 1));
    const activeCoords = coordinates.slice(activeStopIndex);

    const sourceId = `route-source-${routeId}`;
    const completedLayerId = `route-completed-${routeId}`;
    const activeLayerId = `route-active-${routeId}`;

    const geojsonData: GeoJSON.FeatureCollection = {
      type: 'FeatureCollection',
      features: [
        {
          type: 'Feature',
          properties: { status: 'completed' },
          geometry: {
            type: 'LineString',
            coordinates: completedCoords.length > 1 ? completedCoords : coordinates,
          },
        },
        {
          type: 'Feature',
          properties: { status: 'active' },
          geometry: {
            type: 'LineString',
            coordinates: activeCoords.length > 1 ? activeCoords : coordinates,
          },
        },
      ],
    };

    if (map.getSource(sourceId)) {
      (map.getSource(sourceId) as maplibregl.GeoJSONSource).setData(geojsonData);
    } else {
      map.addSource(sourceId, {
        type: 'geojson',
        data: geojsonData,
      });

      // Completed / inactive route line (muted)
      map.addLayer({
        id: completedLayerId,
        type: 'line',
        source: sourceId,
        filter: ['==', 'status', 'completed'],
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': '#39434B',
          'line-width': 4,
          'line-dasharray': [2, 1],
        },
      });

      // Active / upcoming route line (accent primary #25A18E)
      map.addLayer({
        id: activeLayerId,
        type: 'line',
        source: sourceId,
        filter: ['==', 'status', 'active'],
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': '#25A18E',
          'line-width': 4,
          'line-opacity': 0.85,
        },
      });
    }

    return () => {
      if (map) {
        if (map.getLayer(activeLayerId)) map.removeLayer(activeLayerId);
        if (map.getLayer(completedLayerId)) map.removeLayer(completedLayerId);
        if (map.getSource(sourceId)) map.removeSource(sourceId);
      }
    };
  }, [map, routeId, routeStops, activeStopIndex]);

  return null;
};
