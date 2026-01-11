'use client';

import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { useEffect, useState } from 'react';

export default function Navbar() {
    const [isLoggedIn, setIsLoggedIn] = useState(false);
    const pathname = usePathname();

    useEffect(() => {
        // Check for auth_token cookie
        const token = document.cookie.split('; ').find(row => row.startsWith('auth_token='));
        setIsLoggedIn(!!token);
    }, [pathname]);

    return (
        <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
            <div className="container mx-auto px-6 py-4">
                <div className="flex items-center justify-between">
                    <div className="flex items-center space-x-2">
                        <Link href="/" className="flex items-center space-x-2">
                            <div className="w-12 h-12 relative">
                                <img
                                    src="/logo.png"
                                    alt="Much Racing Logo"
                                    className="w-full h-full object-contain"
                                />
                            </div>
                            <span className="text-white text-2xl font-bold">Much Racing</span>
                        </Link>
                    </div>

                    {/* Conditional Navigation */}
                    {isLoggedIn && (
                        <div className="hidden md:flex space-x-6">
                            <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
                            <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
                            <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
                            <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
                        </div>
                    )}

                    <div>
                        {isLoggedIn ? (
                            <button
                                onClick={() => {
                                    document.cookie = 'auth_token=; path=/; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
                                    setIsLoggedIn(false);
                                    window.location.href = '/';
                                }}
                                className="bg-red-600 hover:bg-red-700 text-white px-6 py-2 rounded-lg transition"
                            >
                                Logout
                            </button>
                        ) : (
                            <Link href="/login" className="bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition">
                                Login
                            </Link>
                        )}
                    </div>
                </div>
            </div>
        </nav>
    );
}
