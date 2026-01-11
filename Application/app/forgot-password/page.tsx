'use client';

import { useState } from 'react';
import Link from 'next/link';
import { Mail, ArrowLeft, CheckCircle } from 'lucide-react';

export default function ForgotPasswordPage() {
  const [email, setEmail] = useState('');
  const [isSubmitted, setIsSubmitted] = useState(false);

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    // TODO: Implement password reset logic
    console.log('Password reset requested for:', email);
    setIsSubmitted(true);
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 flex items-center justify-center p-4">
      <div className="w-full max-w-md">
        {/* Logo */}
        <div className="text-center mb-8">
          <Link href="/" className="inline-block relative w-64 h-24">
            <h1 className="sr-only">Much Racing</h1>
            <img 
              src="/logo.png" 
              alt="Much Racing Logo" 
              className="w-full h-full object-contain"
            />
          </Link>
        </div>

        {/* Forgot Password Card */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl overflow-hidden shadow-2xl">
          {/* Header */}
          <div className="bg-blue-600 text-white py-4 px-6">
            <h2 className="text-xl font-semibold">Reset Password</h2>
          </div>

          {/* Content */}
          <div className="p-8">
            {!isSubmitted ? (
              <>
                <p className="text-slate-300 mb-6">
                  Enter your email address and we'll send you a link to reset your password.
                </p>

                <form onSubmit={handleSubmit} className="space-y-6">
                  {/* Email Field */}
                  <div>
                    <label className="block text-slate-300 text-sm font-medium mb-2">
                      Email Address
                    </label>
                    <div className="relative">
                      <input
                        type="email"
                        value={email}
                        onChange={(e) => setEmail(e.target.value)}
                        className="w-full bg-slate-900/50 border border-slate-600 rounded-lg px-4 py-3 pr-12 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition"
                        placeholder="Enter your email"
                        required
                      />
                      <Mail className="absolute right-4 top-1/2 -translate-y-1/2 w-5 h-5 text-slate-500" />
                    </div>
                  </div>

                  {/* Submit Button */}
                  <div className="flex justify-end">
                    <button
                      type="submit"
                      className="bg-blue-600 hover:bg-blue-700 text-white font-semibold px-8 py-3 rounded-lg transition shadow-lg hover:shadow-xl"
                    >
                      Send Reset Link
                    </button>
                  </div>
                </form>
              </>
            ) : (
              /* Success Message */
              <div className="text-center py-8">
                <CheckCircle className="w-16 h-16 text-green-500 mx-auto mb-4" />
                <h3 className="text-xl font-semibold text-white mb-2">Check Your Email</h3>
                <p className="text-slate-300 mb-6">
                  We've sent a password reset link to <span className="text-blue-400 font-semibold">{email}</span>
                </p>
                <p className="text-slate-400 text-sm">
                  Didn't receive the email? Check your spam folder or try again.
                </p>
              </div>
            )}

            {/* Back to Login */}
            <div className="mt-6 pt-6 border-t border-slate-700">
              <Link
                href="/login"
                className="flex items-center justify-center space-x-2 text-blue-400 hover:text-blue-300 transition"
              >
                <ArrowLeft className="w-4 h-4" />
                <span className="font-semibold">Back to Login</span>
              </Link>
            </div>
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
