import React from 'react';
import { Star } from 'lucide-react';

interface FavoriteButtonProps {
  isFavorite: boolean;
  onToggle: (e: React.MouseEvent) => void;
  disabled?: boolean;
}

export const FavoriteButton: React.FC<FavoriteButtonProps> = ({
  isFavorite,
  onToggle,
  disabled = false,
}) => {
  return (
    <button
      type="button"
      onClick={(e) => {
        e.stopPropagation();
        onToggle(e);
      }}
      disabled={disabled}
      title={isFavorite ? 'Remove from favorites' : 'Add to favorites'}
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: '32px',
        height: '32px',
        borderRadius: 'var(--radius-sm)',
        border: '1px solid var(--border-color)',
        backgroundColor: 'var(--bg-surface)',
        color: isFavorite ? '#E69A43' : 'var(--text-muted)',
        cursor: disabled ? 'not-allowed' : 'pointer',
        padding: 0,
      }}
    >
      <Star
        size={16}
        fill={isFavorite ? '#E69A43' : 'none'}
        stroke={isFavorite ? '#E69A43' : 'currentColor'}
      />
    </button>
  );
};
