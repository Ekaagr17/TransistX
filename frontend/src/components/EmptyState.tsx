import React from 'react';
import { LucideIcon } from 'lucide-react';

interface EmptyStateProps {
  icon?: LucideIcon;
  title: string;
  description?: string;
  action?: {
    label: string;
    onClick: () => void;
  };
}

export const EmptyState: React.FC<EmptyStateProps> = ({
  icon: Icon,
  title,
  description,
  action,
}) => {
  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        textAlign: 'center',
        padding: 'var(--space-2xl) var(--space-lg)',
        border: '1px dashed var(--border-color)',
        borderRadius: 'var(--radius-lg)',
        backgroundColor: 'var(--bg-secondary)',
      }}
    >
      {Icon && (
        <div style={{ color: 'var(--text-muted)', marginBottom: 'var(--space-md)' }}>
          <Icon size={36} />
        </div>
      )}
      <h3
        style={{
          fontSize: 'var(--font-size-md)',
          fontWeight: 'var(--font-weight-semibold)',
          color: 'var(--text-primary)',
          marginBottom: 'var(--space-xs)',
        }}
      >
        {title}
      </h3>
      {description && (
        <p
          style={{
            fontSize: 'var(--font-size-sm)',
            color: 'var(--text-secondary)',
            maxWidth: '300px',
            marginBottom: action ? 'var(--space-lg)' : 0,
          }}
        >
          {description}
        </p>
      )}
      {action && (
        <button
          type="button"
          className="btn btn-primary btn-sm"
          onClick={action.onClick}
        >
          {action.label}
        </button>
      )}
    </div>
  );
};
