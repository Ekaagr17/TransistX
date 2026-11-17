import { useRef, useEffect, useState } from 'react';
import maplibregl from 'maplibre-gl';
import { GeoapifyService } from '../services/geoapify.service';

interface UseMapOptions {
  center?: [number, number]; // [lng, lat]
  zoom?: number;
}

export const useMap = (
  containerRef: React.RefObject<HTMLDivElement>,
  options: UseMapOptions = {}
) => {
  const { center = [78.0322, 30.3165], zoom = 13 } = options;
  const mapInstanceRef = useRef<maplibregl.Map | null>(null);
  const [isLoaded, setIsLoaded] = useState<boolean>(false);

  useEffect(() => {
    if (!containerRef.current || mapInstanceRef.current) return;

    const apiKey = GeoapifyService.getApiKey();

    // Dark MapLibre vector style with Geoapify / CartoDB dark basemap
    const map = new maplibregl.Map({
      container: containerRef.current,
      style: {
        version: 8,
        sources: {
          'dark-tiles': {
            type: 'raster',
            tiles: [
              `https://maps.geoapify.com/v1/tile/dark-matter-dark-grey/{z}/{x}/{y}.png?apiKey=${apiKey}`,
              'https://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png',
            ],
            tileSize: 256,
            attribution: '&copy; <a href="https://www.geoapify.com/">Geoapify</a> &copy; <a href="https://carto.com/">CARTO</a>',
          },
        },
        layers: [
          {
            id: 'dark-tiles-layer',
            type: 'raster',
            source: 'dark-tiles',
            minzoom: 0,
            maxzoom: 20,
          },
        ],
      },
      center,
      zoom,
      attributionControl: false,
    });

    map.addControl(new maplibregl.NavigationControl({ showCompass: false }), 'bottom-right');

    map.on('load', () => {
      mapInstanceRef.current = map;
      setIsLoaded(true);
    });

    return () => {
      map.remove();
      mapInstanceRef.current = null;
      setIsLoaded(false);
    };
  }, [containerRef]);

  return {
    map: mapInstanceRef.current,
    isLoaded,
  };
};
