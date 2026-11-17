import React from 'react';
import { Play, Pause, Square, AlertCircle, Loader2 } from 'lucide-react';
import { TripStatus } from '../services/trip.service';

interface TripControlsProps {
  tripStatus: TripStatus;
  isLoading: boolean;
  onStart: () => void;
  onPause: () => void;
  onResume: () => void;
  onEnd: () => void;
}

export const TripControls: React.FC<TripControlsProps> = ({
  tripStatus,
  isLoading,
  onStart,
  onPause,
  onResume,
  onEnd,
}) => {
  const getStatusLabel = () => {
    switch (tripStatus) {
      case 'active':
        return { text: 'TRIP IN PROGRESS', color: '#55A88A' };
      case 'paused':
        return { text: 'TRIP PAUSED', color: '#E69A43' };
      case 'ended':
        return { text: 'TRIP CONCLUDED', color: '#74808A' };
      case 'idle':
      default:
        return { text: 'READY TO DISPATCH', color: '#9AA5AD' };
    }
  };

  const status = getStatusLabel();

  return (
    <div
      style={{
        backgroundColor: '#1E242A',
        border: '1px solid #39434B',
        borderRadius: '12px',
        padding: '16px',
        display: 'flex',
        flexDirection: 'column',
        gap: '14px',
        width: '100%',
      }}
    >
      <div
        style={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          borderBottom: '1px solid #2C343B',
          paddingBottom: '10px',
        }}
      >
        <span
          style={{
            fontSize: '0.75rem',
            fontWeight: 600,
            textTransform: 'uppercase',
            letterSpacing: '0.06em',
            color: '#74808A',
          }}
        >
          Trip Operations
        </span>
        <span
          style={{
            fontSize: '0.75rem',
            fontWeight: 700,
            letterSpacing: '0.04em',
            color: status.color,
            display: 'flex',
            alignItems: 'center',
            gap: '6px',
          }}
        >
          <span
            style={{
              width: '8px',
              height: '8px',
              borderRadius: '50%',
              backgroundColor: status.color,
              display: 'inline-block',
            }}
          />
          {status.text}
        </span>
      </div>

      <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
        {(tripStatus === 'idle' || tripStatus === 'ended') && (
          <button
            type="button"
            onClick={onStart}
            disabled={isLoading}
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '10px',
              minHeight: '56px',
              width: '100%',
              padding: '16px',
              backgroundColor: 'transparent',
              border: '2px solid #25A18E',
              borderRadius: '8px',
              color: '#25A18E',
              fontSize: '1.05rem',
              fontWeight: 700,
              cursor: isLoading ? 'not-allowed' : 'pointer',
              opacity: isLoading ? 0.6 : 1,
            }}
          >
            {isLoading ? <Loader2 size={20} className="animate-pulse" /> : <Play size={20} fill="#25A18E" />}
            <span>START TRIP</span>
          </button>
        )}

        {tripStatus === 'active' && (
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '10px' }}>
            <button
              type="button"
              onClick={onPause}
              disabled={isLoading}
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '8px',
                minHeight: '56px',
                padding: '14px',
                backgroundColor: '#E69A43',
                border: '1px solid #E69A43',
                borderRadius: '8px',
                color: '#171B20',
                fontSize: '0.95rem',
                fontWeight: 700,
                cursor: isLoading ? 'not-allowed' : 'pointer',
                opacity: isLoading ? 0.6 : 1,
              }}
            >
              {isLoading ? <Loader2 size={18} className="animate-pulse" /> : <Pause size={18} />}
              <span>PAUSE</span>
            </button>

            <button
              type="button"
              onClick={onEnd}
              disabled={isLoading}
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '8px',
                minHeight: '56px',
                padding: '14px',
                backgroundColor: 'transparent',
                border: '2px solid #D96C65',
                borderRadius: '8px',
                color: '#D96C65',
                fontSize: '0.95rem',
                fontWeight: 700,
                cursor: isLoading ? 'not-allowed' : 'pointer',
                opacity: isLoading ? 0.6 : 1,
              }}
            >
              {isLoading ? <Loader2 size={18} className="animate-pulse" /> : <Square size={18} />}
              <span>END TRIP</span>
            </button>
          </div>
        )}

        {tripStatus === 'paused' && (
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '10px' }}>
            <button
              type="button"
              onClick={onResume}
              disabled={isLoading}
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '8px',
                minHeight: '56px',
                padding: '14px',
                backgroundColor: '#25A18E',
                border: '1px solid #25A18E',
                borderRadius: '8px',
                color: '#171B20',
                fontSize: '0.95rem',
                fontWeight: 700,
                cursor: isLoading ? 'not-allowed' : 'pointer',
                opacity: isLoading ? 0.6 : 1,
              }}
            >
              {isLoading ? <Loader2 size={18} className="animate-pulse" /> : <Play size={18} fill="#171B20" />}
              <span>RESUME</span>
            </button>

            <button
              type="button"
              onClick={onEnd}
              disabled={isLoading}
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '8px',
                minHeight: '56px',
                padding: '14px',
                backgroundColor: 'transparent',
                border: '2px solid #D96C65',
                borderRadius: '8px',
                color: '#D96C65',
                fontSize: '0.95rem',
                fontWeight: 700,
                cursor: isLoading ? 'not-allowed' : 'pointer',
                opacity: isLoading ? 0.6 : 1,
              }}
            >
              {isLoading ? <Loader2 size={18} className="animate-pulse" /> : <Square size={18} />}
              <span>END TRIP</span>
            </button>
          </div>
        )}
      </div>

      {tripStatus === 'paused' && (
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '8px',
            backgroundColor: '#252C32',
            padding: '10px 12px',
            borderRadius: '6px',
            border: '1px solid #39434B',
            color: '#E69A43',
            fontSize: '0.8rem',
          }}
        >
          <AlertCircle size={16} />
          <span>GPS transmission is paused while stopped at terminus or layover.</span>
        </div>
      )}
    </div>
  );
};
