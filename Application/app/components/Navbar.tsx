'use client';

import Link from 'next/link';
import { usePathname } from 'next/navigation';
import { useEffect, useState } from 'react';
import { Gauge, Map, MonitorSmartphone, Menu, Cloud, Zap, User, Timer, LogOut } from "lucide-react";

export default function Navbar() {
    const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);
    const [isLoggedIn, setIsLoggedIn] = useState(false);
    const pathname = usePathname();

    useEffect(() => {
        // Simple auth check similar to BottomNav
        const token = document.cookie.split('; ').find(row => row.startsWith('auth_token='));
        setIsLoggedIn(!!token);
    }, [pathname]);

    // Close mobile menu when path changes
    useEffect(() => {
        setIsMobileMenuOpen(false);
    }, [pathname]);

    // Don't show navbar on login page to keep it clean, or maybe show a simple version
    if (pathname === '/login') return null;

    return (
        <nav className="fixed top-0 left-0 right-0 z-[9999] carbon-bg border-b border-border-color backdrop-blur-md h-16 shadow-lg">
            <div className="container mx-auto px-4 h-full">
                <div className="flex items-center justify-between h-full">

                    {/* Brand */}
                    <Link href={isLoggedIn ? "/dashboard" : "/"} className="flex items-center gap-3 group">
                        <div className="w-10 h-10 relative flex items-center justify-center bg-background-secondary rounded-lg border border-border-color group-hover:border-primary transition">
                            <Zap className="w-6 h-6 text-primary" />
                        </div>
                        <span className="text-foreground text-xl font-racing tracking-wide group-hover:text-primary transition">
                            MUCH RACING
                        </span>
                    </Link>

                    {/* Desktop Navigation */}
                    {isLoggedIn && (
                        <div className="hidden md:flex items-center space-x-1">
                            <NavLink href="/dashboard" icon={<Gauge className="w-4 h-4" />} label="DASHBOARD" isActive={pathname === '/dashboard'} />
                            <NavLink href="/tracks" icon={<Map className="w-4 h-4" />} label="TRACKS" isActive={pathname.startsWith('/tracks')} />
                            <NavLink href="/sessions" icon={<Timer className="w-4 h-4" />} label="SESSIONS" isActive={pathname.startsWith('/sessions')} />
                            <NavLink href="/device" icon={<MonitorSmartphone className="w-4 h-4" />} label="DEVICES" isActive={pathname.startsWith('/device')} />
                            <NavLink href="/account" icon={<User className="w-4 h-4" />} label="ACCOUNT" isActive={pathname === '/account'} />
                        </div>
                    )}

                    {/* Mobile Menu / Actions */}
                    <div className="flex items-center gap-4">
                        {!isLoggedIn && (
                            <Link href="/login" className="bg-primary hover:bg-primary-hover text-white px-5 py-2 rounded font-racing text-sm transition shadow-lg shadow-primary/20">
                                LOGIN
                            </Link>
                        )}
                        {/* Mobile Menu Button */}
                        {isLoggedIn && (
                            <div className="flex items-center gap-2">
                                <button
                                    onClick={() => {
                                        document.cookie = 'auth_token=; path=/; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
                                        window.location.href = '/login';
                                    }}
                                    className="hidden md:block p-2 text-text-secondary hover:text-warning transition-colors"
                                    title="Logout"
                                >
                                    <LogOut className="w-5 h-5" />
                                </button>
                                <button
                                    onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen)}
                                    className="md:hidden p-2 text-text-secondary hover:text-white"
                                >
                                    <Menu className="w-6 h-6" />
                                </button>
                            </div>
                        )}
                    </div>
                </div>
            </div>

            {/* Mobile Menu Overlay */}
            {isLoggedIn && isMobileMenuOpen && (
                <div className="md:hidden absolute top-16 left-0 right-0 carbon-bg border-b border-border-color shadow-2xl animate-in slide-in-from-top-4 duration-200">
                    <div className="flex flex-col p-4 space-y-2">
                        <MobileNavLink href="/dashboard" icon={<Gauge className="w-4 h-4" />} label="DASHBOARD" isActive={pathname === '/dashboard'} />
                        <MobileNavLink href="/tracks" icon={<Map className="w-4 h-4" />} label="TRACKS" isActive={pathname.startsWith('/tracks')} />
                        <MobileNavLink href="/sessions" icon={<Timer className="w-4 h-4" />} label="SESSIONS" isActive={pathname.startsWith('/sessions')} />
                        <MobileNavLink href="/device" icon={<MonitorSmartphone className="w-4 h-4" />} label="DEVICES" isActive={pathname.startsWith('/device')} />
                        <MobileNavLink href="/account" icon={<User className="w-4 h-4" />} label="ACCOUNT" isActive={pathname === '/account'} />

                        <div className="h-[1px] bg-border-color my-2"></div>

                        <button
                            onClick={() => {
                                document.cookie = 'auth_token=; path=/; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
                                window.location.href = '/login';
                            }}
                            className="flex items-center gap-3 px-4 py-3 rounded-lg text-text-secondary hover:text-warning hover:bg-white/5 transition-all font-racing text-sm w-full text-left"
                        >
                            <LogOut className="w-4 h-4" />
                            LOGOUT
                        </button>
                    </div>
                </div>
            )}

            {/* Mobile Bottom Bar Shadow (Optional visual separation) */}
            <div className="absolute bottom-0 left-0 right-0 h-[1px] bg-gradient-to-r from-transparent via-primary/50 to-transparent"></div>
        </nav>
    );
}

function NavLink({ href, icon, label, isActive }: { href: string; icon: React.ReactNode; label: string; isActive: boolean }) {
    return (
        <Link
            href={href}
            className={`
                flex items-center gap-2 px-4 py-2 rounded-md transition-all duration-200 font-racing text-sm
                ${isActive
                    ? "text-primary bg-primary/10 border border-primary/20"
                    : "text-text-secondary hover:text-foreground hover:bg-white/5"
                }
            `}
        >
            {icon}
            <span>{label}</span>
        </Link>
    )
}

function MobileNavLink({ href, icon, label, isActive }: { href: string; icon: React.ReactNode; label: string; isActive: boolean }) {
    return (
        <Link
            href={href}
            className={`
                flex items-center gap-3 px-4 py-3 rounded-lg transition-all duration-200 font-racing text-sm
                ${isActive
                    ? "text-primary bg-primary/10 border border-primary/20"
                    : "text-text-secondary hover:text-foreground hover:bg-white/5"
                }
            `}
        >
            {icon}
            <span>{label}</span>
        </Link>
    )
}
