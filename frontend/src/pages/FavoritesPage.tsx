import React from 'react';
import { Star, Route as RouteIcon, LogIn } from 'lucide-react';
import { useAuth } from '../hooks/useAuth';
import { useFavorites } from '../hooks/useFavorites';
import { RouteCard } from '../components/RouteCard';
import { LoadingSpinner } from '../components/LoadingSpinner';
import { EmptyState } from '../components/EmptyState';

export const FavoritesPage: React.FC = () => {
  const { isAuthenticated } = useAuth();
  const { favorites, loading, error, toggleFavorite, refetch } = useFavorites();

  if (!isAuthenticated) {
    return (
      <div style={{ padding: 'var(--space-2xl)', maxWidth: '600px', margin: '0 auto' }}>
        <EmptyState
          icon={LogIn}
          title="Sign In Required"
          description="Sign in to your passenger account to bookmark transit routes and quickly check their live bus arrival status."
        />
      </div>
    );
  }

  return (
    <div style={{ padding: 'var(--space-2xl)', maxWidth: '900px', margin: '0 auto', width: '100%' }}>
      <div style={{ marginBottom: 'var(--space-2xl)' }}>
        <h1 style={{ fontSize: 'var(--font-size-2xl)', fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
          Saved Favourites
        </h1>
        <p style={{ color: 'var(--text-secondary)', fontSize: 'var(--font-size-sm)', marginTop: 'var(--space-xs)' }}>
          Quick access to your regular commute lines and real-time updates.
        </p>
      </div>

      {loading ? (
        <LoadingSpinner text="Fetching your bookmarked routes..." />
      ) : error ? (
        <EmptyState
          icon={Star}
          title="Unable to load favourites"
          description={error}
          action={{ label: 'Retry', onClick: refetch }}
        />
      ) : favorites.length === 0 ? (
        <EmptyState
          icon={Star}
          title="No favourite routes yet"
          description="Click the star icon on any route to pin it to your personal dashboard."
        />
      ) : (
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(280px, 1fr))', gap: 'var(--space-lg)' }}>
          {favorites.map((fav) => {
            if (!fav.route) {
              return (
                <div key={fav.id} className="card">
                  <div style={{ fontSize: 'var(--font-size-sm)', color: 'var(--text-primary)', fontWeight: 'var(--font-weight-medium)' }}>
                    Route #{fav.route_id}
                  </div>
                  <button
                    type="button"
                    className="btn btn-sm btn-danger"
                    style={{ marginTop: 'var(--space-sm)' }}
                    onClick={() => toggleFavorite(fav.route_id)}
                  >
                    Remove
                  </button>
                </div>
              );
            }
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
  );
};
