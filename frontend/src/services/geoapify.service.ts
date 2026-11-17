// Geoapify Integration Service for TransitX
// Powers real walking turn-by-turn routing, geocoding, and high-res dark map tiles

const GEOAPIFY_API_KEY =
  ((import.meta as any).env && (import.meta as any).env.VITE_GEOAPIFY_API_KEY) ||
  'ab8d7ec0f81c4b24b12c60b0caca3305';

export interface WalkingRouteResult {
  distanceMeters: number;
  durationMinutes: number;
  geometry?: [number, number][]; // [lng, lat] coordinates for polyline
  instructions?: string[];
  provider: 'geoapify' | 'haversine_fallback';
}

export class GeoapifyService {
  private static apiKey = GEOAPIFY_API_KEY;

  public static getApiKey(): string {
    return this.apiKey;
  }

  /**
   * Calculate turn-by-turn walking route and exact pedestrian distance from user to boarding stop
   */
  public static async getWalkingRoute(
    fromLat: number,
    fromLng: number,
    toLat: number,
    toLng: number
  ): Promise<WalkingRouteResult> {
    try {
      const url = `https://api.geoapify.com/v1/routing?waypoints=${fromLat},${fromLng}|${toLat},${toLng}&mode=walk&apiKey=${this.apiKey}`;
      const res = await fetch(url, { headers: { Accept: 'application/json' } });

      if (!res.ok) {
        throw new Error(`Geoapify routing error: ${res.status}`);
      }

      const data = await res.json();
      if (data.features && data.features.length > 0) {
        const feature = data.features[0];
        const distanceMeters = Math.round(feature.properties.distance || 0);
        const durationMinutes = Math.max(1, Math.round((feature.properties.time || 0) / 60));
        
        // Extract polyline coordinates
        const geometry: [number, number][] = feature.geometry.coordinates[0] || [
          [fromLng, fromLat],
          [toLng, toLat],
        ];

        const instructions: string[] = [];
        if (feature.properties.legs && feature.properties.legs[0]?.steps) {
          for (const step of feature.properties.legs[0].steps) {
            if (step.instruction?.text) {
              instructions.push(step.instruction.text);
            }
          }
        }

        return {
          distanceMeters,
          durationMinutes,
          geometry,
          instructions,
          provider: 'geoapify',
        };
      }
    } catch {
      // Graceful fallback to Haversine straight-line walking distance
    }

    // Fallback calculation (assuming average pedestrian walk speed: 4.8 km/h or 80 m/min)
    const R = 6371e3; // meters
    const phi1 = (fromLat * Math.PI) / 180;
    const phi2 = (toLat * Math.PI) / 180;
    const dPhi = ((toLat - fromLat) * Math.PI) / 180;
    const dLambda = ((toLng - fromLng) * Math.PI) / 180;

    const a =
      Math.sin(dPhi / 2) * Math.sin(dPhi / 2) +
      Math.cos(phi1) * Math.cos(phi2) * Math.sin(dLambda / 2) * Math.sin(dLambda / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    const distanceMeters = Math.round(R * c * 1.25); // 1.25 urban street winding factor
    const durationMinutes = Math.max(1, Math.round(distanceMeters / 80));

    return {
      distanceMeters,
      durationMinutes,
      geometry: [
        [fromLng, fromLat],
        [toLng, toLat],
      ],
      provider: 'haversine_fallback',
    };
  }

  /**
   * Get Geoapify Dark Matter Tile URL matching the TransitX #171B20 palette
   */
  public static getDarkTileUrl(): string {
    return `https://maps.geoapify.com/v1/tile/dark-matter-dark-grey/{z}/{x}/{y}.png?apiKey=${this.apiKey}`;
  }

  /**
   * Geocode a place name in Dehradun using Geoapify
   */
  public static async searchPlace(query: string): Promise<Array<{ name: string; lat: number; lng: number }>> {
    try {
      const url = `https://api.geoapify.com/v1/geocode/autocomplete?text=${encodeURIComponent(query)}&filter=circle:78.0322,30.3165,25000&apiKey=${this.apiKey}`;
      const res = await fetch(url);
      if (!res.ok) return [];
      const data = await res.json();
      return (data.features || []).map((f: any) => ({
        name: f.properties.formatted,
        lat: f.properties.lat,
        lng: f.properties.lon,
      }));
    } catch {
      return [];
    }
  }
}
