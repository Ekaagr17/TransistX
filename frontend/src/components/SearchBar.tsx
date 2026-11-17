import React, { useRef, useEffect } from 'react';
import { Search, Bus, Route, MapPin, X } from 'lucide-react';
import { useSearch } from '../hooks/useSearch';
import type { SearchResult } from '../types';

interface SearchBarProps {
  onSelectResult?: (result: SearchResult) => void;
}

export const SearchBar: React.FC<SearchBarProps> = ({ onSelectResult }) => {
  const { query, setQuery, results, isSearching, clearSearch } = useSearch();
  const dropdownRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (dropdownRef.current && !dropdownRef.current.contains(e.target as Node)) {
        // click outside
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (result: SearchResult) => {
    onSelectResult?.(result);
    clearSearch();
  };

  return (
    <div style={{ position: 'relative', width: '100%', maxWidth: '400px' }} ref={dropdownRef}>
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          backgroundColor: 'var(--bg-surface)',
          border: '1px solid var(--border-color)',
          borderRadius: 'var(--radius-md)',
          padding: '0 var(--space-md)',
          height: '36px',
        }}
      >
        <Search size={16} style={{ color: 'var(--text-muted)', marginRight: 'var(--space-sm)' }} />
        <input
          type="text"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          placeholder="Search bus, route, or stop..."
          style={{
            background: 'none',
            border: 'none',
            outline: 'none',
            color: 'var(--text-primary)',
            fontSize: 'var(--font-size-sm)',
            width: '100%',
            padding: 0,
          }}
        />
        {query && (
          <button
            type="button"
            onClick={clearSearch}
            style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center' }}
          >
            <X size={14} />
          </button>
        )}
      </div>

      {query.length >= 2 && (
        <div
          style={{
            position: 'absolute',
            top: '42px',
            left: 0,
            right: 0,
            backgroundColor: 'var(--bg-secondary)',
            border: '1px solid var(--border-color)',
            borderRadius: 'var(--radius-md)',
            boxShadow: 'var(--shadow-md)',
            zIndex: 100,
            maxHeight: '300px',
            overflowY: 'auto',
          }}
        >
          {isSearching && (
            <div style={{ padding: 'var(--space-md)', color: 'var(--text-muted)', fontSize: 'var(--font-size-xs)' }}>
              Searching...
            </div>
          )}

          {!isSearching && results.length === 0 && (
            <div style={{ padding: 'var(--space-md)', color: 'var(--text-muted)', fontSize: 'var(--font-size-sm)' }}>
              No results found for "{query}"
            </div>
          )}

          {results.map((item) => (
            <div
              key={`${item.type}-${item.id}`}
              onClick={() => handleSelect(item)}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: 'var(--space-md)',
                padding: 'var(--space-sm) var(--space-md)',
                borderBottom: '1px solid var(--border-color)',
                cursor: 'pointer',
                transition: 'background 0.15s ease',
              }}
              onMouseEnter={(e) => (e.currentTarget.style.backgroundColor = 'var(--bg-surface)')}
              onMouseLeave={(e) => (e.currentTarget.style.backgroundColor = 'transparent')}
            >
              {item.type === 'bus' ? (
                <Bus size={16} style={{ color: 'var(--accent-primary)', flexShrink: 0 }} />
              ) : item.type === 'route' ? (
                <Route size={16} style={{ color: 'var(--accent-secondary)', flexShrink: 0 }} />
              ) : (
                <MapPin size={16} style={{ color: 'var(--text-muted)', flexShrink: 0 }} />
              )}
              <div style={{ overflow: 'hidden' }}>
                <div style={{ fontSize: 'var(--font-size-sm)', fontWeight: 'var(--font-weight-medium)', color: 'var(--text-primary)' }}>
                  {item.label}
                </div>
                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }} className="truncate">
                  {item.sublabel}
                </div>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};
