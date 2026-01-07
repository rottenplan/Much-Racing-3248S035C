'use client';

import { useState } from 'react';
import Link from 'next/link';
import { User, Lock, Mail, UserPlus } from 'lucide-react';

export default function LoginPage() {
  const [activeTab, setActiveTab] = useState<'signin' | 'register'>('signin');
  const [formData, setFormData] = useState({
    email: '',
    password: '',
    confirmPassword: '',
    name: ''
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    // TODO: Implement authentication logic
    console.log('Form submitted:', formData);
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 flex items-center justify-center p-4">
      <div className="w-full max-w-md">
        {/* Logo */}
        <div className="text-center mb-8">
          <Link href="/" className="inline-block relative w-64 h-24">
            <h1 className="sr-only">Much Racing</h1>
             {/* Using standard img tag for simplicity or Next.js Image if preferred, sticking to img for now to avoid config issues quickly */}
            <img 
              src="/logo.png" 
              alt="Much Racing Logo" 
              className="w-full h-full object-contain"
            />
          </Link>
        </div>

        {/* Login Card */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl overflow-hidden shadow-2xl">
          {/* Tabs */}
          <div className="flex border-b border-slate-700">
            <button
              onClick={() => setActiveTab('signin')}
              className={`flex-1 flex items-center justify-center space-x-2 py-4 px-6 transition ${
                activeTab === 'signin'
                  ? 'bg-blue-600 text-white'
                  : 'bg-slate-700/50 text-slate-400 hover:text-white'
              }`}
            >
              <User className="w-5 h-5" />
              <span className="font-semibold">Sign In</span>
            </button>
            <button
              onClick={() => setActiveTab('register')}
              className={`flex-1 flex items-center justify-center space-x-2 py-4 px-6 transition ${
                activeTab === 'register'
                  ? 'bg-blue-600 text-white'
                  : 'bg-slate-700/50 text-slate-400 hover:text-white'
              }`}
            >
              <UserPlus className="w-5 h-5" />
              <span className="font-semibold">Register</span>
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
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Username (or Email)
                    </label>
                    <div className="relative">
                      <input
                        type="text"
                        value={formData.email}
                        onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Enter your username or email"
                        required
                      />
                      <User className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Password Field */}
                  <div>
                    <div className="flex items-center justify-between mb-2">
                      <label className="block text-slate-300 text-sm font-medium">
                        Password
                      </label>
                      <Link href="/forgot-password" className="text-blue-400 hover:text-blue-300 text-sm transition">
                        Lost Password?
                      </Link>
                    </div>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.password}
                        onChange={(e) => setFormData({ ...formData, password: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Enter your password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Sign In Button */}
                  <div className="flex justify-end">
                    <button
                      type="submit"
                      className="bg-blue-600 hover:bg-blue-700 text-white font-semibold px-8 py-3 rounded-lg transition shadow-lg hover:shadow-xl"
                    >
                      Sign In
                    </button>
                  </div>

                  {/* Register Link */}
                  <div className="text-center pt-4 border-t border-slate-700">
                    <p className="text-slate-400 text-sm">
                      Don't have an account yet?{' '}
                      <button
                        type="button"
                        onClick={() => setActiveTab('register')}
                        className="text-blue-400 hover:text-blue-300 font-semibold transition"
                      >
                        Register
                      </button>
                    </p>
                  </div>
                </>
              ) : (
                /* Register Form */
                <>
                  {/* Name Field */}
                  <div>
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Full Name
                    </label>
                    <div className="relative">
                      <input
                        type="text"
                        value={formData.name}
                        onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Enter your full name"
                        required
                      />
                      <User className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Email Field */}
                  <div>
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Email Address
                    </label>
                    <div className="relative">
                      <input
                        type="email"
                        value={formData.email}
                        onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Enter your email"
                        required
                      />
                      <Mail className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Password Field */}
                  <div>
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Password
                    </label>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.password}
                        onChange={(e) => setFormData({ ...formData, password: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Create a password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Confirm Password Field */}
                  <div>
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Confirm Password
                    </label>
                    <div className="relative">
                      <input
                        type="password"
                        value={formData.confirmPassword}
                        onChange={(e) => setFormData({ ...formData, confirmPassword: e.target.value })}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Confirm your password"
                        required
                      />
                      <Lock className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Register Button */}
                  <div className="flex justify-end">
                    <button
                      type="submit"
                      className="bg-blue-600 hover:bg-blue-700 text-white font-semibold px-8 py-3 rounded-lg transition shadow-lg hover:shadow-xl"
                    >
                      Register
                    </button>
                  </div>

                  {/* Sign In Link */}
                  <div className="text-center pt-4 border-t border-slate-700">
                    <p className="text-slate-400 text-sm">
                      Already have an account?{' '}
                      <button
                        type="button"
                        onClick={() => setActiveTab('signin')}
                        className="text-blue-400 hover:text-blue-300 font-semibold transition"
                      >
                        Sign In
                      </button>
                    </p>
                  </div>
                </>
              )}
            </form>
          </div>
        </div>

        {/* Footer */}
        <div className="text-center mt-8">
          <p className="text-slate-500 text-sm">
            © Copyright 2024. All Rights Reserved.
          </p>
        </div>
      </div>
    </div>
  );
}
