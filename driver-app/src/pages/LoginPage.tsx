import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { Bus, KeyRound, Mail, AlertCircle, ArrowRight, Loader2 } from 'lucide-react';
import { useAuth } from '../hooks/useAuth';

export const LoginPage: React.FC = () => {
  const [email, setEmail] = useState('driver1@demo.com');
  const [password, setPassword] = useState('password123');
  const [error, setError] = useState<string | null>(null);
  const [isSubmitting, setIsSubmitting] = useState(false);

  const { login } = useAuth();
  const navigate = useNavigate();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!email || !password) {
      setError('Please fill in both email and password.');
      return;
    }

    setError(null);
    setIsSubmitting(true);

    try {
      await login({ email, password });
      navigate('/driver');
    } catch (err: unknown) {
      const msg =
        err instanceof Error
          ? err.message
          : (err as { response?: { data?: { message?: string } } })?.response?.data?.message ||
            'Invalid credentials or server unreachable.';
      setError(msg);
    } finally {
      setIsSubmitting(false);
    }
  };

  const handleQuickFill = (driverEmail: string) => {
    setEmail(driverEmail);
    setPassword('password123');
    setError(null);
  };

  return (
    <div
      style={{
        minHeight: '100dvh',
        backgroundColor: '#171B20',
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
        padding: '20px 16px',
      }}
    >
      <div
        style={{
          width: '100%',
          maxWidth: '420px',
          backgroundColor: '#1E242A',
          border: '1px solid #39434B',
          borderRadius: '14px',
          padding: '28px 20px',
          display: 'flex',
          flexDirection: 'column',
          gap: '20px',
        }}
      >
        <div style={{ textAlign: 'center', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
          <div
            style={{
              width: '52px',
              height: '52px',
              backgroundColor: '#252C32',
              border: '1px solid #39434B',
              borderRadius: '12px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: '#25A18E',
              marginBottom: '12px',
            }}
          >
            <Bus size={28} />
          </div>
          <h1
            style={{
              fontSize: '1.45rem',
              fontWeight: 800,
              color: '#E5E9EC',
              letterSpacing: '-0.02em',
              margin: 0,
            }}
          >
            Transit<span style={{ color: '#25A18E' }}>X</span> Driver
          </h1>
          <p style={{ fontSize: '0.82rem', color: '#9AA5AD', marginTop: '6px', margin: '6px 0 0' }}>
            In-Vehicle Telemetry & GPS Dispatch Console
          </p>
        </div>

        {error && (
          <div
            style={{
              backgroundColor: 'rgba(217, 108, 101, 0.1)',
              border: '1px solid rgba(217, 108, 101, 0.4)',
              borderRadius: '8px',
              padding: '12px',
              display: 'flex',
              alignItems: 'center',
              gap: '10px',
              color: '#D96C65',
              fontSize: '0.85rem',
            }}
          >
            <AlertCircle size={18} style={{ flexShrink: 0 }} />
            <span>{error}</span>
          </div>
        )}

        <form onSubmit={handleSubmit} style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
            <label
              htmlFor="email"
              style={{
                fontSize: '0.75rem',
                fontWeight: 600,
                textTransform: 'uppercase',
                letterSpacing: '0.04em',
                color: '#9AA5AD',
                display: 'flex',
                alignItems: 'center',
                gap: '6px',
              }}
            >
              <Mail size={13} color="#25A18E" />
              Driver Email
            </label>
            <input
              id="email"
              type="email"
              required
              autoComplete="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              placeholder="e.g. driver1@demo.com"
            />
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
            <label
              htmlFor="password"
              style={{
                fontSize: '0.75rem',
                fontWeight: 600,
                textTransform: 'uppercase',
                letterSpacing: '0.04em',
                color: '#9AA5AD',
                display: 'flex',
                alignItems: 'center',
                gap: '6px',
              }}
            >
              <KeyRound size={13} color="#25A18E" />
              PIN / Password
            </label>
            <input
              id="password"
              type="password"
              required
              autoComplete="current-password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="••••••••"
            />
          </div>

          <button
            type="submit"
            disabled={isSubmitting}
            className="btn btn-primary"
            style={{
              marginTop: '8px',
              backgroundColor: '#25A18E',
              border: 'none',
              borderRadius: '8px',
              color: '#fff',
              fontWeight: 700,
              fontSize: '1rem',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '8px',
              cursor: isSubmitting ? 'not-allowed' : 'pointer',
              opacity: isSubmitting ? 0.7 : 1,
            }}
          >
            {isSubmitting ? (
              <>
                <Loader2 size={18} className="animate-pulse" />
                <span>Authenticating...</span>
              </>
            ) : (
              <>
                <span>Sign In to Terminal</span>
                <ArrowRight size={18} />
              </>
            )}
          </button>
        </form>

        <div
          style={{
            borderTop: '1px solid #2C343B',
            paddingTop: '16px',
            display: 'flex',
            flexDirection: 'column',
            gap: '8px',
          }}
        >
          <div style={{ fontSize: '0.7rem', color: '#74808A', textTransform: 'uppercase', letterSpacing: '0.05em' }}>
            Quick Demo Select
          </div>
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '8px' }}>
            {['driver1@demo.com', 'driver2@demo.com'].map((dEmail, idx) => (
              <button
                key={dEmail}
                type="button"
                onClick={() => handleQuickFill(dEmail)}
                style={{
                  backgroundColor: '#252C32',
                  border: '1px solid #39434B',
                  borderRadius: '6px',
                  padding: '8px',
                  fontSize: '0.75rem',
                  color: email === dEmail ? '#25A18E' : '#9AA5AD',
                  fontWeight: 600,
                  textAlign: 'center',
                }}
              >
                Driver #{idx + 1}
              </button>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
};
