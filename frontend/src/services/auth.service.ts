import api from './api';
import type { AuthResponse, User } from '../types';

export const authService = {
  async login(email: string, password: string): Promise<AuthResponse> {
    const res = await api.post<AuthResponse>('/auth/login', { email, password });
    const data = res.data;
    localStorage.setItem('transitx_token', data.token);
    localStorage.setItem('transitx_user', JSON.stringify(data.user));
    return data;
  },

  async register(name: string, email: string, password: string): Promise<AuthResponse> {
    const res = await api.post<AuthResponse>('/auth/register', { name, email, password });
    const data = res.data;
    localStorage.setItem('transitx_token', data.token);
    localStorage.setItem('transitx_user', JSON.stringify(data.user));
    return data;
  },

  async getMe(): Promise<User> {
    const res = await api.get<User>('/auth/me');
    return res.data;
  },

  logout(): void {
    localStorage.removeItem('transitx_token');
    localStorage.removeItem('transitx_user');
  },

  getToken(): string | null {
    return localStorage.getItem('transitx_token');
  },

  isAuthenticated(): boolean {
    return !!localStorage.getItem('transitx_token');
  },

  getStoredUser(): User | null {
    const raw = localStorage.getItem('transitx_user');
    if (!raw) return null;
    try {
      return JSON.parse(raw) as User;
    } catch {
      return null;
    }
  },
};
