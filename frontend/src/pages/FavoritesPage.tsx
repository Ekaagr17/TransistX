import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { Star, Bus as BusIcon, MapPin, Footprints, ArrowRight, Trash2, Route as RouteIcon } from 'lucide-react';
import { useAuth } from '../hooks/useAuth';
import { useFavorites } from '../hooks/useFavorites';
import { journeyService } from '../services/journey.service';
import { RouteCard } from '../components/RouteCard';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';
import type { FavoriteBus } from '../types';

export const FavoritesPage: React.FC = () => {
  const { isAuthenticated } = useAuth();
  const { favorites, loading, error, toggleFavorite } = useFavorites();
  const [favoriteBuses, setFavoriteBuses] = useState<FavoriteBus[]>([]);
  const navigate = useNavigate();

  useEffect(() => {
    setFavoriteBuses(journeyService.getFavoriteBuses());
  }, []);

  const handleRemoveFavoriteBus = (busId: number) => {
    journeyService.toggleFavoriteBus(busId, '');
    setFavoriteBuses(journeyService.getFavoriteBuses());
  };

  const handleTrackBus = (busId: number) => {
    navigate('/buses', { state: { selectedBusId: busId } });
  };

  return (
    <div style={{ padding: 'var(--space-2xl)', maxWidth: '1000px', margin: '0 auto', width: '100%', overflowY: 'auto' }}>
      <div style={{ marginBottom: 'var(--space-2xl)' }}>
        <h1 style={{ fontSize: 'var(--font-size-2xl)', fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
          Saved Favourites
        </h1>
        <p style={{ color: 'var(--text-secondary)', fontSize: 'var(--font-size-sm)', marginTop: 'var(--space-xs)' }}>
          Your bookmarked buses and commute lines with real-time boarding stop distances.
        </p>
      </div>

      {/* FAVORITE BUSES SECTION */}
      <div style={{ marginBottom: 'var(--space-3xl)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', marginBottom: 'var(--space-md)' }}>
          <BusIcon size={20} style={{ color: 'var(--accent-primary)' }} />
          <h2 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)' }}>
            Favorite Buses ({favoriteBuses.length})
          </h2>
        </div>

        {favoriteBuses.length === 0 ? (
          <div className="card" style={{ padding: 'var(--space-xl)', textAlign: 'center' }}>
            <Star size={28} style={{ color: 'var(--text-muted)', margin: '0 auto var(--space-sm)' }} />
            <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
              No favorite buses bookmarked yet
            </div>
            <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)', marginTop: '4px' }}>
              Click the star icon on any bus in the Journey Planner or Live Buses view to save it here.
            </p>
          </div>
        ) : (
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(300px, 1fr))', gap: 'var(--space-lg)' }}>
            {favoriteBuses.map((favBus) => (
              <div
                key={favBus.bus_id}
                className="card"
                style={{
                  display: 'flex',
                  flexDirection: 'column',
                  gap: 'var(--space-sm)',
                  border: '1px solid var(--border-color)',
                  backgroundColor: 'var(--bg-secondary)',
                }}
              >
                <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                  <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
                    <div
                      style={{
                        padding: '4px 8px',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-sm)',
                        fontWeight: 'var(--font-weight-bold)',
                        color: 'var(--accent-primary)',
                        fontSize: 'var(--font-size-sm)',
                      }}
                    >
                      Bus {favBus.bus_number}
                    </div>
                  </div>
                  <button
                    type="button"
                    onClick={() => handleRemoveFavoriteBus(favBus.bus_id)}
                    className="btn btn-sm"
                    style={{ color: 'var(--text-muted)' }}
                    title="Remove from favorites"
                  >
                    <Trash2 size={14} />
                  </button>
                </div>

                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)' }}>
                  {favBus.route_name || 'Dehradun City Line'}
                </div>

                {/* Where to go / Nearest Boarding Stop banner */}
                <div
                  style={{
                    padding: 'var(--space-sm)',
                    backgroundColor: 'var(--bg-surface)',
                    borderRadius: 'var(--radius-md)',
                    border: '1px solid var(--border-color)',
                    fontSize: 'var(--font-size-xs)',
                  }}
                >
                  <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', color: 'var(--accent-primary)', fontWeight: 'var(--font-weight-semibold)', marginBottom: '2px' }}>
                    <MapPin size={12} />
                    <span>YOUR BOARDING STOP:</span>
                  </div>
                  <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
                    Clock Tower Station
                  </div>
                  <div style={{ display: 'flex', alignItems: 'center', gap: '4px', color: 'var(--text-muted)', marginTop: '2px' }}>
                    <Footprints size={12} />
                    <span>320 meters away (~4 min walk)</span>
                  </div>
                </div>

                <button
                  type="button"
                  className="btn btn-primary btn-sm"
                  onClick={() => handleTrackBus(favBus.bus_id)}
                  style={{ marginTop: 'auto', display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 'var(--space-xs)' }}
                >
                  <span>Track on Live Map</span>
                  <ArrowRight size={14} />
                </button>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* FAVORITE ROUTES SECTION */}
      <div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)', marginBottom: 'var(--space-md)' }}>
          <RouteIcon size={20} style={{ color: 'var(--accent-secondary)' }} />
          <h2 style={{ fontSize: 'var(--font-size-lg)', fontWeight: 'var(--font-weight-semibold)' }}>
            Bookmarked Routes ({favorites.length})
          </h2>
        </div>

        {loading ? (
          <LoadingSpinner text="Fetching your bookmarked routes..." />
        ) : error ? (
          <EmptyState
            icon={Star}
            title="Unable to load routes"
            description={error}
          />
        ) : favorites.length === 0 ? (
          <div className="card" style={{ padding: 'var(--space-xl)', textAlign: 'center' }}>
            <RouteIcon size={28} style={{ color: 'var(--text-muted)', margin: '0 auto var(--space-sm)' }} />
            <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
              No favorite routes bookmarked
            </div>
            <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)', marginTop: '4px' }}>
              Star any transit route from the Routes page to keep it here.
            </p>
          </div>
        ) : (
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(280px, 1fr))', gap: 'var(--space-lg)' }}>
            {favorites.map((fav) => {
              if (!fav.route) return null;
              return (
                <RouteCard
                  key={fav.id}
                  route={fav.route}
                  isFavorite={true}
                  onToggleFavorite={() => toggleFavorite(fav.route_id)}
                />
              );
            })}
          </div>
        )}
      </div>
    </div>
  );
};
