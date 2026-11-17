import React from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { AuthProvider, useAuth } from './hooks/useAuth';
import { LoginPage } from './pages/LoginPage';
import { DashboardPage } from './pages/DashboardPage';

const ProtectedRoute: React.FC<{ children: React.ReactElement }> = ({ children }) => {
  const { isAuthenticated, isLoading } = useAuth();

  if (isLoading) {
    return (
      <div
        style={{
          minHeight: '100dvh',
          backgroundColor: '#171B20',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          color: '#9AA5AD',
        }}
      >
        Authenticating terminal session...
      </div>
    );
  }

  if (!isAuthenticated) {
    return <Navigate to="/driver/login" replace />;
  }

  return children;
};

export const App: React.FC = () => {
  return (
    <AuthProvider>
      <BrowserRouter>
        <Routes>
          <Route path="/driver/login" element={<LoginPage />} />
          <Route
            path="/driver"
            element={
              <ProtectedRoute>
                <DashboardPage />
              </ProtectedRoute>
            }
          />
          <Route path="/" element={<Navigate to="/driver" replace />} />
          <Route path="*" element={<Navigate to="/driver" replace />} />
        </Routes>
      </BrowserRouter>
    </AuthProvider>
  );
};

export default App;
