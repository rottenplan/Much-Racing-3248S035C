'use client';

import { useState } from 'react';
import Link from 'next/link';
import { useRouter } from 'next/navigation';
import { User, Lock, Mail, UserPlus, AlertCircle, Zap } from 'lucide-react';

export default function LoginPage() {
  const [activeTab, setActiveTab] = useState<'signin' | 'register'>('signin');
  const [formData, setFormData] = useState({
    email: '',
    password: '',
    confirmPassword: '',
    name: ''
  });
  const [passwordError, setPasswordError] = useState('');

  // Allowed characters: 0-9, a-z, A-Z, ! # $ % & ' ( ) * + , – . @ : ; =
  const ALLOWED_PASSWORD_CHARS = /^[0-9a-zA-Z!#$%&'()*+,\-.@:;=]*$/;

  const validatePassword = (password: string): string => {
    if (!password) return '';
    if (!ALLOWED_PASSWORD_CHARS.test(password)) {
      return 'Password contains invalid characters. Only allowed: 0-9, a-z, A-Z, ! # $ % & \' ( ) * + , – . @ : ; =';
    }
    return '';
  };

  const handlePasswordChange = (value: string) => {
    setFormData({ ...formData, password: value });
    setPasswordError(validatePassword(value));
  };


  const router = useRouter();
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setLoading(true);

    if (activeTab === 'register') {
      const passError = validatePassword(formData.password);
      if (passError) {
        setPasswordError(passError);
        setLoading(false);
        return;
      }
      if (formData.password !== formData.confirmPassword) {
        setPasswordError('Passwords do not match');
        setLoading(false);
        return;
      }
    }

    try {
      // Use the same API for both for now (mock)
      const res = await fetch('/api/auth/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          email: formData.email,
          password: formData.password
        })
      });

      const data = await res.json();

      if (res.ok && data.success) {
        // Set cookie (valid for 7 days)
        const expires = new Date();
        expires.setDate(expires.getDate() + 7);
        document.cookie = `auth_token=${data.token}; path=/; expires=${expires.toUTCString()}`;

        // Redirect to dashboard
        // Redirect based on action
        if (activeTab === 'register') {
          router.push('/setup-device');
        } else {
          router.push('/dashboard');
        }
      } else {
        setError(data.message || 'Authentication failed');
      }
    } catch (err) {
      setError('Connection error. Please check server.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen bg-background flex flex-col items-center justify-center p-4">
      {/* Logo Container */}
      <div className="w-full flex justify-center mb-8">
        <Link href="/" className="relative w-[600px] h-[200px] group">
          <h1 className="sr-only">Much Racing</h1>
          <img
            src="/logo.png"
            alt="Much Racing Logo"
            className="w-full h-full object-contain group-hover:scale-105 transition-transform"
          />
        </Link>
      </div>

      {/* Login Card */}
      <div className="w-full max-w-md">
        <div className="carbon-bg backdrop-blur-md border border-border-color rounded-xl overflow-hidden shadow-2xl">
          {/* Tabs */}
          <div className="flex border-b border-border-color">
            <button
              onClick={() => setActiveTab('signin')}
              className={`flex-1 flex items-center justify-center gap-2 py-4 px-6 transition font-racing text-sm ${activeTab === 'signin'
                ? 'bg-primary text-white shadow-[0_0_20px_rgba(220,38,38,0.3)]'
                : 'bg-background-secondary text-text-secondary hover:text-foreground'
                }`}
            >
              <User className="w-5 h-5" />
              <span>SIGN IN</span>
            </button>
            <button
              onClick={() => setActiveTab('register')}
              className={`flex-1 flex items-center justify-center gap-2 py-4 px-6 transition font-racing text-sm ${activeTab === 'register'
                ? 'bg-primary text-white shadow-[0_0_20px_rgba(220,38,38,0.3)]'
                : 'bg-background-secondary text-text-secondary hover:text-foreground'
                }`}
            >
              <UserPlus className="w-5 h-5" />
              <span>REGISTER</span>
            </button>
          </div>

          {/* Form Content */}
          <div className="p-8">
            <form onSubmit={handleSubmit} className="space-y-6">
              {activeTab === 'signin' ? (
                /* Sign In Form */
                <>
                  {/* Username/Email Field */}
                  <div>
                    <label className="block text-text-secondary text-sm font-medium mb-2">
                      Username (or Email)
                    </label>
                    <div className="relative">
                      <input
                        type="text"
                        value={formData.email}
                        onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                        className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition"
                        placeholder="Enter your username or email"
                        required
                      />
                      <User className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                  </div>

                  {/* Password Field */}
                  <div>
                    <div className="flex items-center justify-between mb-2">
                      <label className="block text-text-secondary text-sm font-medium">
                        Password
                      </label>
                      <Link href="/forgot-password" className="text-primary hover:text-primary-hover text-sm transition">
                        Lost Password?
                      </Link>
                    </div>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.password}
                        onChange={(e) => setFormData({ ...formData, password: e.target.value })}
                        className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition"
                        placeholder="Enter your password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                  </div>

                  {/* Sign In Button */}
                  <div className="flex justify-end">
                    <button
                      type="submit"
                      disabled={loading}
                      className="bg-primary hover:bg-primary-hover text-white font-racing px-8 py-3 rounded-lg transition shadow-lg hover:shadow-xl disabled:opacity-50"
                    >
                      {loading ? 'SIGNING IN...' : 'SIGN IN'}
                    </button>
                  </div>

                  {/* Register Link */}
                  <div className="text-center pt-4 border-t border-border-color">
                    <p className="text-text-secondary text-sm">
                      Don't have an account yet?{' '}
                      <button
                        type="button"
                        onClick={() => setActiveTab('register')}
                        className="text-primary hover:text-primary-hover font-racing transition"
                      >
                        REGISTER
                      </button>
                    </p>
                  </div>
                </>
              ) : (
                /* Register Form */
                <>
                  {/* Name Field */}
                  <div>
                    <label className="block text-text-secondary text-sm font-medium mb-2">
                      Full Name
                    </label>
                    <div className="relative">
                      <input
                        type="text"
                        value={formData.name}
                        onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                        className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition"
                        placeholder="Enter your full name"
                        required
                      />
                      <User className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                  </div>

                  {/* Email Field */}
                  <div>
                    <label className="block text-text-secondary text-sm font-medium mb-2">
                      Email Address
                    </label>
                    <div className="relative">
                      <input
                        type="email"
                        value={formData.email}
                        onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                        className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition"
                        placeholder="Enter your email"
                        required
                      />
                      <Mail className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                  </div>

                  {/* Password Field */}
                  <div>
                    <label className="block text-text-secondary text-sm font-medium mb-2">
                      Password
                    </label>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.password}
                        onChange={(e) => handlePasswordChange(e.target.value)}
                        className={`w-full bg-background-secondary border rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none transition ${passwordError ? 'border-warning focus:border-warning' : 'border-border-color focus:border-primary'
                          }`}
                        placeholder="Create a password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                    {passwordError && (
                      <div className="flex items-start gap-2 mt-2 text-warning text-xs">
                        <AlertCircle className="w-4 h-4 mt-0.5 flex-shrink-0" />
                        <span>{passwordError}</span>
                      </div>
                    )}
                    <p className="text-text-secondary text-xs mt-2">
                      Allowed: 0-9, a-z, A-Z, ! # $ % &amp; ' ( ) * + , – . @ : ; =
                    </p>
                  </div>

                  {/* Confirm Password Field */}
                  <div>
                    <label className="block text-text-secondary text-sm font-medium mb-2">
                      Confirm Password
                    </label>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.confirmPassword}
                        onChange={(e) => setFormData({ ...formData, confirmPassword: e.target.value })}
                        className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 pr-12 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition"
                        placeholder="Confirm your password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-text-secondary" />
                    </div>
                  </div>

                  {/* WiFi Notice */}
                  <div className="bg-primary/10 border border-primary/30 rounded-lg p-3">
                    <p className="text-primary text-xs">
                      <strong>Note:</strong> Your device requires WiFi connection. If your WiFi password contains unsupported characters, create a 2.4 GHz hotspot with a compatible password.
                    </p>
                  </div>

                  {/* Register Button */}
                  <div className="flex justify-end">
                    <button
                      type="submit"
                      disabled={!!passwordError || loading}
                      className="bg-primary hover:bg-primary-hover disabled:bg-background-secondary disabled:cursor-not-allowed text-white font-racing px-8 py-3 rounded-lg transition shadow-lg hover:shadow-xl"
                    >
                      {loading ? 'REGISTERING...' : 'REGISTER'}
                    </button>
                  </div>

                  {/* Sign In Link */}
                  <div className="text-center pt-4 border-t border-border-color">
                    <p className="text-text-secondary text-sm">
                      Already have an account?{' '}
                      <button
                        type="button"
                        onClick={() => setActiveTab('signin')}
                        className="text-primary hover:text-primary-hover font-racing transition"
                      >
                        SIGN IN
                      </button>
                    </p>
                  </div>
                </>
              )}

              {/* Error Message */}
              {error && (
                <div className="bg-warning/10 border border-warning/30 rounded-lg p-3 flex items-start gap-2">
                  <AlertCircle className="w-5 h-5 text-warning flex-shrink-0 mt-0.5" />
                  <span className="text-warning text-sm">{error}</span>
                </div>
              )}
            </form>
          </div>
        </div>

        {/* Footer */}
        <div className="text-center mt-8">
          <p className="text-text-secondary text-sm">
            © Copyright 2024. All Rights Reserved.
          </p>
        </div>
      </div>
    </div>
  );
}
