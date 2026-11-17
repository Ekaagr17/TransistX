import api from './api';

export interface LoginPayload {
  email: string;
  password: string;
}

export interface DriverUser {
  id: number;
  name: string;
  email: string;
  role: string;
  driver_id: number;
  bus_id?: number;
  bus_number?: string;
  route_id?: number;
  route_name?: string;
}

export interface LoginResponse {
  token: string;
  user: DriverUser;
}

export const authService = {
  async login(payload: LoginPayload): Promise<LoginResponse> {
    const { data } = await api.post<LoginResponse>('/auth/login', payload);
    localStorage.setItem('driver_token', data.token);
    localStorage.setItem('driver_user', JSON.stringify(data.user));
    return data;
  },

  logout(): void {
    localStorage.removeItem('driver_token');
    localStorage.removeItem('driver_user');
  },

  getToken(): string | null {
    return localStorage.getItem('driver_token');
  },

  getUser(): DriverUser | null {
    const raw = localStorage.getItem('driver_user');
    if (!raw) return null;
    try {
      return JSON.parse(raw) as DriverUser;
    } catch {
      return null;
    }
  },

  isAuthenticated(): boolean {
    return !!this.getToken();
  },
};
