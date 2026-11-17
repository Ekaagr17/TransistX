import React, { useState, useEffect } from 'react';
import {
  X,
  MapPin,
  Clock,
  Compass,
  Star,
  CheckCircle,
  Navigation,
  ArrowRight,
  Sparkles,
  Footprints,
  ChevronDown,
  ChevronUp,
} from 'lucide-react';
import {
  journeyService,
  DEHRADUN_STOPS,
} from '../services/journey.service';
import type { JourneyCandidate, Stop } from '../types';

interface JourneyPlannerModalProps {
  isOpen: boolean;
  onClose: () => void;
  preselectedBusId?: number | null;
  onSelectBusAndRoute: (candidate: JourneyCandidate) => void;
}

export const JourneyPlannerModal: React.FC<JourneyPlannerModalProps> = ({
  isOpen,
  onClose,
  preselectedBusId,
  onSelectBusAndRoute,
}) => {
  // Starting point (defaults to Clock Tower #1)
  const [startStopId, setStartStopId] = useState<number>(1);
  // Ending point (defaults to ISBT Dehradun #6)
  const [endStopId, setEndStopId] = useState<number>(6);

  const [candidates, setCandidates] = useState<JourneyCandidate[]>([]);
  const [selectedCandidate, setSelectedCandidate] = useState<JourneyCandidate | null>(null);
  const [expandedBusScheduleId, setExpandedBusScheduleId] = useState<number | null>(null);
  const [passengerCoords, setPassengerCoords] = useState<{ lat: number; lng: number } | null>(null);
  const [favoritesMap, setFavoritesMap] = useState<Record<number, boolean>>({});
  const [currentTimeStr, setCurrentTimeStr] = useState<string>('');

  // Popular destination presets
  const popularDestinations = journeyService.getPopularEndPoints();

  // Load user geolocation if available
  useEffect(() => {
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(
        (pos) => {
          setPassengerCoords({
            lat: pos.coords.latitude,
            lng: pos.coords.longitude,
          });
        },
        () => {
          // Geolocation not granted or unavailable
        }
      );
    }
  }, []);

  // Update live clock string
  useEffect(() => {
    const updateTime = () => {
      const d = new Date();
      setCurrentTimeStr(
        d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' })
      );
    };
    updateTime();
    const interval = setInterval(updateTime, 1000);
    return () => clearInterval(interval);
  }, []);

  // Fetch or calculate journey whenever start or end changes
  useEffect(() => {
    if (!isOpen) return;

    const runPlanning = async () => {
      const results = await journeyService.planJourney({
        start_stop_id: startStopId,
        end_stop_id: endStopId,
        passenger_lat: passengerCoords?.lat,
        passenger_lng: passengerCoords?.lng,
        preferred_bus_id: preselectedBusId || undefined,
      });

      setCandidates(results);

      // Refresh favorites mapping
      const favs: Record<number, boolean> = {};
      results.forEach((c) => {
        favs[c.bus_id] = journeyService.isBusFavorite(c.bus_id);
      });
      setFavoritesMap(favs);

      // If a preselectedBusId was specified, focus on it; otherwise default to best pick
      if (preselectedBusId) {
        const found = results.find((c) => c.bus_id === preselectedBusId);
        if (found) {
          setSelectedCandidate(found);
          setExpandedBusScheduleId(found.bus_id);
          return;
        }
      }

      if (results.length > 0) {
        const best = results.find((c) => c.is_best_pick) || results[0];
        setSelectedCandidate(best);
        setExpandedBusScheduleId(best.bus_id);
      } else {
        setSelectedCandidate(null);
      }
    };

    runPlanning();
  }, [isOpen, startStopId, endStopId, preselectedBusId, passengerCoords]);

  if (!isOpen) return null;

  const handleToggleFavorite = (candidate: JourneyCandidate, e: React.MouseEvent) => {
    e.stopPropagation();
    const isNowFav = journeyService.toggleFavoriteBus(
      candidate.bus_id,
      candidate.bus_number,
      candidate.route_name
    );
    setFavoritesMap((prev) => ({ ...prev, [candidate.bus_id]: isNowFav }));
  };

  const handleConfirmSelection = (candidate: JourneyCandidate) => {
    setSelectedCandidate(candidate);
    onSelectBusAndRoute(candidate);
    onClose();
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div
        className="modal-content"
        onClick={(e) => e.stopPropagation()}
        style={{
          maxWidth: '820px',
          width: '95vw',
          maxHeight: '92vh',
          display: 'flex',
          flexDirection: 'column',
          padding: 0,
          overflow: 'hidden',
          backgroundColor: 'var(--bg-secondary)',
          border: '1px solid var(--border-color)',
          borderRadius: 'var(--radius-xl)',
        }}
      >
        {/* Modal Top Bar */}
        <div
          style={{
            padding: 'var(--space-md) var(--space-xl)',
            backgroundColor: 'var(--bg-elevated)',
            borderBottom: '1px solid var(--border-color)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
            <div
              style={{
                width: '32px',
                height: '32px',
                borderRadius: 'var(--radius-sm)',
                backgroundColor: 'rgba(37, 161, 142, 0.15)',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                color: 'var(--accent-primary)',
              }}
            >
              <Compass size={18} />
            </div>
            <div>
              <h2 style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-bold)' }}>
                Smart Journey Planner & Best Bus Picker
              </h2>
              <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                Real-time synchronized with clock: {currentTimeStr}
              </p>
            </div>
          </div>
          <button
            type="button"
            onClick={onClose}
            style={{ color: 'var(--text-muted)', display: 'flex', alignItems: 'center', cursor: 'pointer' }}
          >
            <X size={20} />
          </button>
        </div>

        {/* Origin & Destination Selectors */}
        <div
          style={{
            padding: 'var(--space-lg) var(--space-xl)',
            borderBottom: '1px solid var(--border-color)',
            backgroundColor: 'var(--bg-surface)',
          }}
        >
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 'var(--space-lg)' }}>
            {/* Origin Dropdown (29 Stops) */}
            <div className="input-group">
              <label style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)' }}>
                <MapPin size={14} style={{ color: 'var(--accent-primary)' }} />
                <span>SELECT YOUR STARTING POINT ({DEHRADUN_STOPS.length} STOPS)</span>
              </label>
              <select
                value={startStopId}
                onChange={(e) => setStartStopId(Number(e.target.value))}
                style={{
                  backgroundColor: 'var(--bg-secondary)',
                  borderColor: 'var(--border-color)',
                  fontWeight: 'var(--font-weight-medium)',
                }}
              >
                {DEHRADUN_STOPS.map((stop) => (
                  <option key={stop.id} value={stop.id}>
                    {stop.id}. {stop.stop_name}
                  </option>
                ))}
              </select>
            </div>

            {/* Destination Dropdown */}
            <div className="input-group">
              <label style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)' }}>
                <Navigation size={14} style={{ color: 'var(--accent-secondary)' }} />
                <span>SELECT YOUR DESTINATION POINT</span>
              </label>
              <select
                value={endStopId}
                onChange={(e) => setEndStopId(Number(e.target.value))}
                style={{
                  backgroundColor: 'var(--bg-secondary)',
                  borderColor: 'var(--border-color)',
                  fontWeight: 'var(--font-weight-medium)',
                }}
              >
                {DEHRADUN_STOPS.map((stop) => (
                  <option key={stop.id} value={stop.id} disabled={stop.id === startStopId}>
                    {stop.id}. {stop.stop_name}
                  </option>
                ))}
              </select>
            </div>
          </div>

          {/* Quick preset destination buttons */}
          <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-xs)', marginTop: 'var(--space-sm)', flexWrap: 'wrap' }}>
            <span style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)', marginRight: '4px' }}>
              Quick Terminals:
            </span>
            {popularDestinations.map((term) => (
              <button
                key={term.id}
                type="button"
                className={`btn btn-sm ${endStopId === term.id ? 'btn-primary-filled' : ''}`}
                style={{ padding: '2px 8px', fontSize: 'var(--font-size-xs)' }}
                onClick={() => setEndStopId(term.id)}
              >
                {term.stop_name}
              </button>
            ))}
          </div>
        </div>

        {/* Selected Bus & Boarding Distance Instructions (PROMINENT BANNER) */}
        {selectedCandidate && (
          <div
            style={{
              padding: 'var(--space-md) var(--space-xl)',
              backgroundColor: 'rgba(37, 161, 142, 0.08)',
              borderBottom: '1px solid rgba(37, 161, 142, 0.3)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              flexWrap: 'wrap',
              gap: 'var(--space-md)',
            }}
          >
            <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-md)' }}>
              <div
                style={{
                  width: '36px',
                  height: '36px',
                  borderRadius: 'var(--radius-sm)',
                  backgroundColor: 'var(--accent-primary)',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  color: 'var(--bg-primary)',
                  fontWeight: 'var(--font-weight-bold)',
                }}
              >
                <Footprints size={20} />
              </div>
              <div>
                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--accent-primary)', fontWeight: 'var(--font-weight-semibold)' }}>
                  WHERE YOU NEED TO GO:
                </div>
                <div style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
                  Walk to: {selectedCandidate.boarding_stop.stop_name}
                </div>
                <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)' }}>
                  Distance from your location: <strong style={{ color: 'var(--text-primary)' }}>{selectedCandidate.walking_distance_meters} meters</strong> (~{selectedCandidate.walking_duration_minutes} min walk) • Bus arrives at <strong style={{ color: 'var(--accent-primary)' }}>{selectedCandidate.arrival_time_at_boarding}</strong>
                </div>
              </div>
            </div>

            <button
              type="button"
              className="btn btn-primary-filled"
              onClick={() => handleConfirmSelection(selectedCandidate)}
              style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}
            >
              <span>Track Bus {selectedCandidate.bus_number}</span>
              <ArrowRight size={16} />
            </button>
          </div>
        )}

        {/* Candidates List with Real-Time Timings */}
        <div style={{ flex: 1, overflowY: 'auto', padding: 'var(--space-lg) var(--space-xl)' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-md)' }}>
            <div style={{ fontSize: 'var(--font-size-sm)', fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
              AVAILABLE BUSES FOR THIS TRIP ({candidates.length})
            </div>
            <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
              Calculated dynamically at {currentTimeStr}
            </div>
          </div>

          {candidates.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', padding: 'var(--space-xl)' }}>
              <Compass size={32} style={{ color: 'var(--text-muted)', margin: '0 auto var(--space-sm)' }} />
              <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
                No active connecting buses found
              </div>
              <p style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-secondary)', marginTop: '4px' }}>
                Try selecting a different starting point or terminal destination.
              </p>
            </div>
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: 'var(--space-md)' }}>
              {candidates.map((candidate) => {
                const isSelected = selectedCandidate?.bus_id === candidate.bus_id;
                const isFav = !!favoritesMap[candidate.bus_id];
                const isScheduleExpanded = expandedBusScheduleId === candidate.bus_id;

                return (
                  <div
                    key={candidate.bus_id}
                    onClick={() => setSelectedCandidate(candidate)}
                    className={`card ${isSelected ? 'card-elevated' : ''}`}
                    style={{
                      borderColor: candidate.is_best_pick
                        ? 'var(--accent-primary)'
                        : isSelected
                        ? 'var(--border-color)'
                        : 'var(--border-color)',
                      boxShadow: candidate.is_best_pick ? '0 0 12px rgba(37, 161, 142, 0.2)' : 'none',
                      cursor: 'pointer',
                      transition: 'all 0.2s ease',
                      padding: 'var(--space-md)',
                    }}
                  >
                    {/* Header with Bus details & Best Pick Badge */}
                    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 'var(--space-sm)', flexWrap: 'wrap', gap: 'var(--space-sm)' }}>
                      <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
                        <div
                          style={{
                            padding: '2px 8px',
                            backgroundColor: 'var(--bg-surface)',
                            border: '1px solid var(--border-color)',
                            borderRadius: 'var(--radius-sm)',
                            fontWeight: 'var(--font-weight-bold)',
                            fontSize: 'var(--font-size-sm)',
                            color: 'var(--accent-primary)',
                          }}
                        >
                          {candidate.route_code}
                        </div>
                        <span style={{ fontSize: 'var(--font-size-md)', fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)' }}>
                          Bus {candidate.bus_number}
                        </span>
                        <span style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
                          ({candidate.registration_number})
                        </span>
                      </div>

                      <div style={{ display: 'flex', alignItems: 'center', gap: 'var(--space-sm)' }}>
                        {candidate.is_best_pick && (
                          <span
                            className="badge badge-live"
                            style={{
                              display: 'inline-flex',
                              alignItems: 'center',
                              gap: '4px',
                              padding: '3px 8px',
                              backgroundColor: 'rgba(37, 161, 142, 0.2)',
                              color: 'var(--accent-primary)',
                              border: '1px solid var(--accent-primary)',
                            }}
                          >
                            <Sparkles size={12} />
                            RECOMMENDED BEST PICK
                          </span>
                        )}

                        {/* Favorite button */}
                        <button
                          type="button"
                          onClick={(e) => handleToggleFavorite(candidate, e)}
                          title={isFav ? 'Remove bus from favorites' : 'Add bus to favorites'}
                          style={{
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            width: '28px',
                            height: '28px',
                            borderRadius: 'var(--radius-sm)',
                            border: '1px solid var(--border-color)',
                            backgroundColor: 'var(--bg-surface)',
                            color: isFav ? 'var(--accent-secondary)' : 'var(--text-muted)',
                            cursor: 'pointer',
                          }}
                        >
                          <Star size={14} fill={isFav ? 'var(--accent-secondary)' : 'none'} />
                        </button>
                      </div>
                    </div>

                    {candidate.best_pick_reason && (
                      <div
                        style={{
                          fontSize: 'var(--font-size-xs)',
                          color: 'var(--accent-primary)',
                          marginBottom: 'var(--space-sm)',
                          display: 'flex',
                          alignItems: 'center',
                          gap: '4px',
                        }}
                      >
                        <CheckCircle size={12} />
                        <span>{candidate.best_pick_reason}</span>
                      </div>
                    )}

                    {/* Key Journey Timings Row */}
                    <div
                      style={{
                        display: 'grid',
                        gridTemplateColumns: 'repeat(auto-fit, minmax(140px, 1fr))',
                        gap: 'var(--space-sm)',
                        padding: 'var(--space-sm)',
                        backgroundColor: 'var(--bg-surface)',
                        borderRadius: 'var(--radius-md)',
                        marginBottom: 'var(--space-sm)',
                        fontSize: 'var(--font-size-xs)',
                      }}
                    >
                      <div>
                        <div style={{ color: 'var(--text-muted)' }}>Boarding At</div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
                          {candidate.boarding_stop.stop_name}
                        </div>
                        <div style={{ color: 'var(--accent-primary)', fontWeight: 'var(--font-weight-bold)' }}>
                          {candidate.arrival_time_at_boarding} (~{candidate.boarding_eta_minutes} min)
                        </div>
                      </div>

                      <div>
                        <div style={{ color: 'var(--text-muted)' }}>Drop-Off At</div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
                          {candidate.destination_stop.stop_name}
                        </div>
                        <div style={{ color: 'var(--accent-secondary)', fontWeight: 'var(--font-weight-bold)' }}>
                          {candidate.arrival_time_at_destination} (~{candidate.destination_eta_minutes} min)
                        </div>
                      </div>

                      <div>
                        <div style={{ color: 'var(--text-muted)' }}>Walking Distance</div>
                        <div style={{ fontWeight: 'var(--font-weight-semibold)', color: 'var(--text-primary)' }}>
                          {candidate.walking_distance_meters}m to stop
                        </div>
                        <div style={{ color: 'var(--text-secondary)' }}>
                          ~{candidate.walking_duration_minutes} min walk
                        </div>
                      </div>

                      <div>
                        <div style={{ color: 'var(--text-muted)' }}>Total Trip Time</div>
                        <div style={{ fontWeight: 'var(--font-weight-bold)', color: 'var(--text-primary)', fontSize: 'var(--font-size-sm)' }}>
                          {candidate.total_trip_minutes} mins
                        </div>
                        <div style={{ color: 'var(--text-muted)' }}>
                          Speed: {candidate.current_speed_kmh} km/h
                        </div>
                      </div>
                    </div>

                    {/* Accordion Toggle for Stop-by-Stop Timings */}
                    <button
                      type="button"
                      onClick={(e) => {
                        e.stopPropagation();
                        setExpandedBusScheduleId(isScheduleExpanded ? null : candidate.bus_id);
                      }}
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        width: '100%',
                        padding: '4px 8px',
                        fontSize: 'var(--font-size-xs)',
                        color: 'var(--text-secondary)',
                        backgroundColor: 'transparent',
                        cursor: 'pointer',
                        borderTop: '1px solid var(--border-color)',
                        marginTop: '4px',
                      }}
                    >
                      <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                        <Clock size={12} />
                        View Live Arrival Timings at Every Stop ({candidate.stop_timings.length} stops)
                      </span>
                      {isScheduleExpanded ? <ChevronUp size={14} /> : <ChevronDown size={14} />}
                    </button>

                    {/* Expanded Stop-by-Stop Timetable */}
                    {isScheduleExpanded && (
                      <div
                        style={{
                          marginTop: 'var(--space-sm)',
                          backgroundColor: 'var(--bg-primary)',
                          border: '1px solid var(--border-color)',
                          borderRadius: 'var(--radius-md)',
                          padding: 'var(--space-sm)',
                          fontSize: 'var(--font-size-xs)',
                        }}
                      >
                        <table style={{ width: '100%', borderCollapse: 'collapse', textAlign: 'left' }}>
                          <thead>
                            <tr style={{ color: 'var(--text-muted)', borderBottom: '1px solid var(--border-color)' }}>
                              <th style={{ padding: '4px' }}>#</th>
                              <th style={{ padding: '4px' }}>Transit Stop</th>
                              <th style={{ padding: '4px' }}>Scheduled</th>
                              <th style={{ padding: '4px' }}>Live Arrival</th>
                              <th style={{ padding: '4px' }}>Status</th>
                            </tr>
                          </thead>
                          <tbody>
                            {candidate.stop_timings.map((st) => {
                              const isBoarding = st.stop_id === candidate.boarding_stop.id;
                              const isDest = st.stop_id === candidate.destination_stop.id;

                              return (
                                <tr
                                  key={st.stop_id}
                                  style={{
                                    borderBottom: '1px solid rgba(57, 67, 75, 0.4)',
                                    backgroundColor: isBoarding
                                      ? 'rgba(37, 161, 142, 0.15)'
                                      : isDest
                                      ? 'rgba(230, 154, 67, 0.15)'
                                      : 'transparent',
                                  }}
                                >
                                  <td style={{ padding: '4px', color: 'var(--text-muted)' }}>{st.stop_order}</td>
                                  <td style={{ padding: '4px', fontWeight: isBoarding || isDest ? 'bold' : 'normal' }}>
                                    {st.stop_name}
                                    {isBoarding && (
                                      <span style={{ color: 'var(--accent-primary)', marginLeft: '6px' }}>
                                        [Your Boarding Stop]
                                      </span>
                                    )}
                                    {isDest && (
                                      <span style={{ color: 'var(--accent-secondary)', marginLeft: '6px' }}>
                                        [Your Destination]
                                      </span>
                                    )}
                                  </td>
                                  <td style={{ padding: '4px', color: 'var(--text-muted)' }}>{st.scheduled_time}</td>
                                  <td
                                    style={{
                                      padding: '4px',
                                      color: isBoarding ? 'var(--accent-primary)' : 'var(--text-primary)',
                                      fontWeight: isBoarding ? 'bold' : 'normal',
                                    }}
                                  >
                                    {st.estimated_time}
                                  </td>
                                  <td style={{ padding: '4px' }}>
                                    {st.is_passed ? (
                                      <span style={{ color: 'var(--text-muted)' }}>Departed</span>
                                    ) : (
                                      <span style={{ color: 'var(--color-success)' }}>
                                        in {st.eta_minutes_from_now} min
                                      </span>
                                    )}
                                  </td>
                                </tr>
                              );
                            })}
                          </tbody>
                        </table>
                      </div>
                    )}
                  </div>
                );
              })}
            </div>
          )}
        </div>

        {/* Modal Footer */}
        <div
          style={{
            padding: 'var(--space-md) var(--space-xl)',
            backgroundColor: 'var(--bg-elevated)',
            borderTop: '1px solid var(--border-color)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
          }}
        >
          <div style={{ fontSize: 'var(--font-size-xs)', color: 'var(--text-muted)' }}>
            Click any bus to preview schedule. Selected bus will track and center on the interactive map.
          </div>
          <button
            type="button"
            className="btn btn-sm"
            onClick={onClose}
          >
            Close
          </button>
        </div>
      </div>
    </div>
  );
};
