'use client';

import { useState } from 'react';
import Link from 'next/link';
import {
    Trash2,
    Save,
    Lock,
    Smartphone,
    Plus,
    ChevronDown,
    User,
    Zap
} from 'lucide-react';


export default function AccountPage() {
    // Mock User Data
    const [profile, setProfile] = useState({
        username: 'Muchdas',
        email: 'faisalmuchdas@gmail.com',
        driverNumber: 190,
        country: 'Indonesia',
        category: 'Vespa Tune Up',
        lastConnection: '2026-01-06 22:22:17'
    });

    // Mock Devices Data
    const [devices] = useState([
        { id: '94:89:7E:E5:12:CC', activated: true },
        { id: '80:F3:DA:AD:59:18', activated: true }
    ]);

    // Form States
    const [passwords, setPasswords] = useState({
        old: '',
        new: '',
        confirm: ''
    });

    const categories = ['Vespa Tune Up', 'Sport 150cc', 'Underbone 2T', 'Supermoto'];
    const countries = ['Indonesia', 'Malaysia', 'Singapore', 'Thailand', 'Vietnam'];

    const handleProfileUpdate = (e: React.FormEvent) => {
        e.preventDefault();
        alert('Modifications saved successfully!');
    };

    const handlePasswordChange = (e: React.FormEvent) => {
        e.preventDefault();
        if (passwords.new !== passwords.confirm) {
            alert('New passwords do not match!');
            return;
        }
        alert('Password changed successfully!');
        setPasswords({ old: '', new: '', confirm: '' });
    };

    const handleRemoveAccount = () => {
        if (confirm('Are you sure you want to remove your account? This action cannot be undone.')) {
            alert('Account removal request submitted.');
        }
    };

    return (
        <div className="min-h-screen bg-background text-foreground pb-24">
            {/* Header with Carbon Fiber */}


            {/* Main Content */}
            <main className="container mx-auto px-4 py-6">

                <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">

                    {/* LEFT COLUMN: Profile Info */}
                    <div className="lg:col-span-2 space-y-6">

                        {/* Profile Header Card */}
                        <div className="carbon-bg rounded-xl overflow-hidden border border-border-color shadow-lg">
                            <div className="bg-gradient-to-r from-primary to-highlight p-6 flex flex-col sm:flex-row items-center sm:items-start gap-4 relative">
                                {/* Avatar Circle */}
                                <div className="w-20 h-20 rounded-full bg-background border-4 border-background shadow-xl flex items-center justify-center overflow-hidden shrink-0">
                                    <div className="w-full h-full bg-gradient-to-br from-warning to-highlight relative flex items-center justify-center">
                                        <span className="text-background font-racing text-2xl font-bold">M</span>
                                    </div>
                                </div>

                                <div className="flex-1 text-center sm:text-left z-10">
                                    <h2 className="text-2xl font-racing text-white">{profile.username}</h2>
                                    <p className="text-white/80 text-sm">{profile.email}</p>
                                </div>

                                <div className="absolute bottom-2 right-4 text-xs text-white/60 font-data">
                                    Last: {profile.lastConnection}
                                </div>
                            </div>

                            {/* Account Informations Form */}
                            <div className="p-6">
                                <h3 className="text-lg font-racing text-primary mb-4">ACCOUNT INFORMATION</h3>

                                <form onSubmit={handleProfileUpdate} className="space-y-4">

                                    {/* Username (Read-only) */}
                                    <div className="grid grid-cols-1 md:grid-cols-4 gap-3 items-center">
                                        <label className="text-text-secondary font-medium text-sm md:text-right">User Name:</label>
                                        <div className="md:col-span-3">
                                            <input
                                                type="text"
                                                value={profile.username}
                                                readOnly
                                                className="w-full bg-background-secondary border border-border-color text-text-secondary rounded px-3 py-2 cursor-not-allowed focus:outline-none text-sm"
                                            />
                                        </div>
                                    </div>

                                    {/* Email (Read-only) */}
                                    <div className="grid grid-cols-1 md:grid-cols-4 gap-3 items-center">
                                        <label className="text-text-secondary font-medium text-sm md:text-right">Email:</label>
                                        <div className="md:col-span-3">
                                            <input
                                                type="text"
                                                value={profile.email}
                                                readOnly
                                                className="w-full bg-background-secondary border border-border-color text-text-secondary rounded px-3 py-2 cursor-not-allowed focus:outline-none text-sm"
                                            />
                                        </div>
                                    </div>

                                    {/* Driver Number */}
                                    <div className="grid grid-cols-1 md:grid-cols-4 gap-3 items-center">
                                        <label className="text-text-secondary font-medium text-sm md:text-right">Driver #:</label>
                                        <div className="md:col-span-3">
                                            <input
                                                type="number"
                                                value={profile.driverNumber}
                                                onChange={(e) => setProfile({ ...profile, driverNumber: parseInt(e.target.value) })}
                                                className="w-24 bg-card-bg border border-border-color text-foreground rounded px-3 py-2 focus:outline-none focus:border-primary transition font-data text-sm"
                                            />
                                        </div>
                                    </div>

                                    {/* Country */}
                                    <div className="grid grid-cols-1 md:grid-cols-4 gap-3 items-center">
                                        <label className="text-text-secondary font-medium text-sm md:text-right">Country:</label>
                                        <div className="md:col-span-3">
                                            <select
                                                value={profile.country}
                                                onChange={(e) => setProfile({ ...profile, country: e.target.value })}
                                                className="w-full bg-card-bg border border-border-color text-foreground rounded px-3 py-2 focus:outline-none focus:border-primary transition text-sm"
                                            >
                                                {countries.map(c => <option key={c} value={c}>{c}</option>)}
                                            </select>
                                        </div>
                                    </div>

                                    {/* Default Category */}
                                    <div className="grid grid-cols-1 md:grid-cols-4 gap-3 items-center">
                                        <label className="text-text-secondary font-medium text-sm md:text-right">Category:</label>
                                        <div className="md:col-span-3 flex gap-2">
                                            <select
                                                value={profile.category}
                                                onChange={(e) => setProfile({ ...profile, category: e.target.value })}
                                                className="flex-1 bg-card-bg border border-border-color text-foreground rounded px-3 py-2 focus:outline-none focus:border-primary transition text-sm"
                                            >
                                                {categories.map(c => <option key={c} value={c}>{c}</option>)}
                                            </select>
                                            <button type="button" className="bg-primary hover:bg-primary-hover text-white p-2 rounded transition">
                                                <Plus className="w-4 h-4" />
                                            </button>
                                        </div>
                                    </div>

                                    {/* Action Buttons */}
                                    <div className="flex flex-col sm:flex-row gap-3 pt-4 border-t border-border-color mt-6">
                                        <button
                                            type="button"
                                            onClick={() => {
                                                document.cookie = 'auth_token=; path=/; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
                                                window.location.href = '/login';
                                            }}
                                            className="bg-background-secondary hover:bg-card-bg text-foreground border border-border-color px-4 py-2 rounded flex items-center justify-center gap-2 transition text-sm font-racing"
                                        >
                                            LOGOUT
                                        </button>

                                        <div className="flex-1"></div>

                                        <button
                                            type="button"
                                            onClick={handleRemoveAccount}
                                            className="text-warning hover:text-warning/80 text-xs underline underline-offset-4"
                                        >
                                            Remove Account
                                        </button>

                                        <button
                                            type="submit"
                                            className="bg-primary hover:bg-primary-hover text-white px-6 py-2 rounded flex items-center justify-center gap-2 transition shadow-md text-sm font-racing"
                                        >
                                            <Save className="w-4 h-4" />
                                            SAVE CHANGES
                                        </button>
                                    </div>
                                </form>
                            </div>
                        </div>

                    </div>

                    {/* RIGHT COLUMN: Settings & Devices */}
                    <div className="space-y-6">

                        {/* Change Password Card */}
                        <div className="carbon-bg rounded-xl overflow-hidden shadow-lg border border-border-color">
                            <div className="p-4 border-b border-border-color flex items-center justify-between">
                                <h3 className="text-base font-racing flex items-center gap-2 text-foreground">
                                    <Lock className="w-4 h-4 text-primary" />
                                    CHANGE PASSWORD
                                </h3>
                                <ChevronDown className="w-4 h-4 text-text-secondary" />
                            </div>
                            <div className="p-4">
                                <form onSubmit={handlePasswordChange} className="space-y-3">
                                    <div>
                                        <label className="block text-text-secondary text-xs mb-1">Old password:</label>
                                        <input
                                            type="password"
                                            value={passwords.old}
                                            onChange={(e) => setPasswords({ ...passwords, old: e.target.value })}
                                            className="w-full bg-background-secondary border border-border-color rounded px-3 py-2 text-foreground focus:outline-none focus:border-primary transition text-sm"
                                        />
                                    </div>
                                    <div>
                                        <label className="block text-text-secondary text-xs mb-1">New password:</label>
                                        <input
                                            type="password"
                                            value={passwords.new}
                                            onChange={(e) => setPasswords({ ...passwords, new: e.target.value })}
                                            className="w-full bg-background-secondary border border-border-color rounded px-3 py-2 text-foreground focus:outline-none focus:border-primary transition text-sm"
                                        />
                                    </div>
                                    <div>
                                        <label className="block text-text-secondary text-xs mb-1">Confirm password:</label>
                                        <input
                                            type="password"
                                            value={passwords.confirm}
                                            onChange={(e) => setPasswords({ ...passwords, confirm: e.target.value })}
                                            className="w-full bg-background-secondary border border-border-color rounded px-3 py-2 text-foreground focus:outline-none focus:border-primary transition text-sm"
                                        />
                                    </div>
                                    <button
                                        type="submit"
                                        className="w-full bg-primary hover:bg-primary-hover text-white py-2 rounded mt-2 transition font-racing text-sm"
                                    >
                                        CHANGE PASSWORD
                                    </button>
                                </form>
                            </div>
                        </div>

                        {/* Devices Card */}
                        <div className="carbon-bg rounded-xl overflow-hidden shadow-lg border border-border-color">
                            <div className="p-4 border-b border-border-color flex items-center justify-between">
                                <h3 className="text-base font-racing flex items-center gap-2 text-foreground">
                                    <Smartphone className="w-4 h-4 text-primary" />
                                    DEVICES
                                </h3>
                                <ChevronDown className="w-4 h-4 text-text-secondary" />
                            </div>
                            <div className="p-0">
                                <table className="w-full text-xs">
                                    <thead className="bg-background-secondary text-text-secondary">
                                        <tr>
                                            <th className="px-3 py-2 text-left w-8 font-racing">#</th>
                                            <th className="px-3 py-2 text-left font-racing">DEVICE</th>
                                            <th className="px-3 py-2 text-left font-racing">ACTIVE</th>
                                        </tr>
                                    </thead>
                                    <tbody className="divide-y divide-border-color">
                                        {devices.map((device, index) => (
                                            <tr key={device.id} className="hover:bg-card-bg transition">
                                                <td className="px-3 py-2 text-text-secondary">{index + 1}</td>
                                                <td className="px-3 py-2 font-data text-foreground">{device.id}</td>
                                                <td className="px-3 py-2">
                                                    <span className={`text-xs font-bold ${device.activated ? 'text-highlight' : 'text-text-secondary'}`}>
                                                        {device.activated ? 'YES' : 'NO'}
                                                    </span>
                                                </td>
                                            </tr>
                                        ))}
                                    </tbody>
                                </table>
                                {devices.length === 0 && (
                                    <div className="p-4 text-center text-text-secondary italic text-sm">No devices registered</div>
                                )}
                            </div>
                        </div>

                    </div>
                </div>

                {/* Footer */}
                <div className="mt-8 text-center text-text-secondary text-xs">
                    Much Racing - 2026 - <Link href="/contact" className="text-primary hover:underline">Contact us</Link>
                </div>
            </main>

        </div>
    );
}
