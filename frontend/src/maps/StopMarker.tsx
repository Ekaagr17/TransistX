import React, { useEffect, useRef } from 'react';
import maplibregl from 'maplibre-gl';
import type { Stop } from '../types';

interface StopMarkerProps {
  map: maplibregl.Map | null;
  stop: Stop;
  isSelected?: boolean;
  onClick?: (stopId: number) => void;
}

export const StopMarker: React.FC<StopMarkerProps> = ({ map, stop, isSelected, onClick }) => {
  const markerRef = useRef<maplibregl.Marker | null>(null);

  useEffect(() => {
    if (!map) return;

    const el = document.createElement('div');
    el.className = `stop-marker ${isSelected ? 'selected' : ''}`;
    el.style.width = isSelected ? '14px' : '10px';
    el.style.height = isSelected ? '14px' : '10px';
    el.style.borderRadius = '50%';
    el.style.backgroundColor = isSelected ? '#25A18E' : '#2C343B';
    el.style.border = '2px solid #E5E9EC';
    el.style.cursor = 'pointer';
    el.style.boxShadow = '0 1px 3px rgba(0,0,0,0.4)';
    el.title = stop.stop_name;

    el.onclick = (e) => {
      e.stopPropagation();
      onClick?.(stop.id);
    };

    const marker = new maplibregl.Marker({
      element: el,
    })
      .setLngLat([stop.longitude, stop.latitude])
      .addTo(map);

    markerRef.current = marker;

    return () => {
      marker.remove();
      markerRef.current = null;
    };
  }, [map, stop.id, stop.latitude, stop.longitude, stop.stop_name, isSelected, onClick]);

  return null;
};
