import React, { useEffect, useRef } from 'react';
import maplibregl from 'maplibre-gl';
import type { BusLocation } from '../types';

interface BusMarkerProps {
  map: maplibregl.Map | null;
  location: BusLocation;
  isSelected?: boolean;
  onClick?: (busId: number) => void;
}

export const BusMarker: React.FC<BusMarkerProps> = ({ map, location, isSelected, onClick }) => {
  const markerRef = useRef<maplibregl.Marker | null>(null);
  const elementRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (!map) return;

    const el = document.createElement('div');
    el.className = `bus-marker ${isSelected ? 'selected' : ''}`;
    el.style.width = '32px';
    el.style.height = '32px';
    el.style.borderRadius = '50%';
    el.style.backgroundColor = '#1E242A';
    el.style.border = isSelected ? '2px solid #E69A43' : '2px solid #25A18E';
    el.style.display = 'flex';
    el.style.alignItems = 'center';
    el.style.justifyContent = 'center';
    el.style.cursor = 'pointer';
    el.style.boxShadow = '0 2px 6px rgba(0, 0, 0, 0.5)';
    el.style.transition = 'transform 0.3s ease, border-color 0.2s ease';

    const icon = document.createElement('div');
    icon.innerHTML = `
      <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="${isSelected ? '#E69A43' : '#25A18E'}" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <path d="M8 6v6"></path>
        <path d="M16 6v6"></path>
        <path d="M2 12h20"></path>
        <path d="M18 18h.01"></path>
        <path d="M6 18h.01"></path>
        <path d="M4 6h16a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2z"></path>
      </svg>
    `;
    
    // Apply heading rotation if available
    if (location.heading !== null && location.heading !== undefined) {
      icon.style.transform = `rotate(${location.heading}deg)`;
      icon.style.transition = 'transform 0.3s ease';
    }

    el.appendChild(icon);

    el.onclick = (e) => {
      e.stopPropagation();
      onClick?.(location.bus_id);
    };

    elementRef.current = el;

    const marker = new maplibregl.Marker({
      element: el,
      rotationAlignment: 'map',
    })
      .setLngLat([location.longitude, location.latitude])
      .addTo(map);

    markerRef.current = marker;

    return () => {
      marker.remove();
      markerRef.current = null;
    };
  }, [map]);

  // Update position & rotation
  useEffect(() => {
    if (markerRef.current) {
      markerRef.current.setLngLat([location.longitude, location.latitude]);
    }
    if (elementRef.current) {
      elementRef.current.style.border = isSelected ? '2px solid #E69A43' : '2px solid #25A18E';
      const icon = elementRef.current.firstElementChild as HTMLElement;
      if (icon) {
        if (location.heading !== null && location.heading !== undefined) {
          icon.style.transform = `rotate(${location.heading}deg)`;
        }
        const svg = icon.querySelector('svg');
        if (svg) {
          svg.setAttribute('stroke', isSelected ? '#E69A43' : '#25A18E');
        }
      }
    }
  }, [location.latitude, location.longitude, location.heading, isSelected]);

  return null;
};
