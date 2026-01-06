'use client';

import { useState } from 'react';
import Link from 'next/link';
import { 
  Trash2, 
  Save, 
  Lock, 
  Smartphone, 
  Plus,
  ChevronDown
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
    <div className="min-h-screen bg-slate-900 text-slate-200">
       {/* Navigation (Simplified for consistency with other pages) */}
       <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
            <Link href="/" className="flex items-center space-x-2">
              <div className="w-10 h-10 bg-gradient-to-br from-orange-500 to-red-600 rounded-lg flex items-center justify-center">
                <span className="text-white font-bold text-xl">R</span>
              </div>
              <span className="text-white text-2xl font-bold">Much Racing</span>
            </Link>
            <div className="hidden md:flex space-x-6">
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
            <div className="flex items-center space-x-4">
                 <div className="flex flex-col items-end mr-2">
                    <span className="text-sm font-semibold text-white">{profile.username}</span>
                    <span className="text-xs text-slate-400">{profile.email}</span>
                </div>
                 {/* Current User Avatar Placeholder */}
                 <div className="w-10 h-10 rounded-full bg-gradient-to-r from-yellow-400 to-yellow-600 flex items-center justify-center border-2 border-slate-700">
                    <span className="text-slate-900 font-bold">M</span>
                  </div>
            </div>
          </div>
        </div>
      </nav>

      {/* Main Content */}
      <main className="container mx-auto px-6 py-8">
        {/* Page Header */}
        <div className="mb-8">
            <h1 className="text-2xl font-bold text-white mb-2">My Account</h1>
             <div className="h-1 w-20 bg-gradient-to-r from-orange-500 to-blue-600 rounded"></div>
        </div>
       
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-8">
          
          {/* LEFT COLUMN: Profile Info */}
          <div className="lg:col-span-2 space-y-6">
            
            {/* Profile Header Card */}
            <div className="bg-slate-800 rounded-xl overflow-hidden border border-slate-700 shadow-lg">
                <div className="bg-[#0ea5e9] p-6 flex flex-col sm:flex-row items-center sm:items-start gap-6 relative">
                    {/* Unique Avatar Circle */}
                    <div className="w-24 h-24 rounded-full bg-white border-4 border-white shadow-xl flex items-center justify-center overflow-hidden shrink-0">
                         {/* Mock Avatar Design matching reference */}
                        <div className="w-full h-full bg-yellow-400 relative">
                             <div className="absolute top-1/4 left-0 w-full h-2 bg-slate-800"></div>
                             <div className="absolute top-1/2 left-0 w-full h-4 bg-slate-800 flex items-center justify-center text-[10px] text-white font-mono">MUCH</div>
                             <div className="absolute bottom-1 bg-green-500 w-full h-1/3 opacity-30"></div>
                        </div>
                    </div>
                    
                    <div className="flex-1 text-center sm:text-left z-10">
                        <h2 className="text-3xl font-bold text-white">{profile.username}</h2>
                        <p className="text-blue-100">{profile.email}</p>
                    </div>

                     <div className="absolute bottom-2 right-4 text-xs text-blue-100/80">
                        Last Connexion: {profile.lastConnection}
                    </div>
                </div>

                {/* Account Informations Form */}
                <div className="p-8">
                    <h3 className="text-xl font-semibold text-blue-400 mb-6">Account Informations</h3>
                    
                    <form onSubmit={handleProfileUpdate} className="space-y-6">
                        
                        {/* Username (Read-only) */}
                        <div className="grid grid-cols-1 md:grid-cols-4 gap-4 items-center">
                            <label className="text-slate-400 font-medium md:text-right">User Name:</label>
                            <div className="md:col-span-3">
                                <input 
                                    type="text" 
                                    value={profile.username} 
                                    readOnly 
                                    className="w-full bg-slate-700/50 border border-slate-600 text-slate-300 rounded px-4 py-2 cursor-not-allowed focus:outline-none"
                                />
                            </div>
                        </div>

                        {/* Email (Read-only) */}
                         <div className="grid grid-cols-1 md:grid-cols-4 gap-4 items-center">
                            <label className="text-slate-400 font-medium md:text-right">Email:</label>
                            <div className="md:col-span-3">
                                <input 
                                    type="text" 
                                    value={profile.email} 
                                    readOnly 
                                    className="w-full bg-slate-700/50 border border-slate-600 text-slate-300 rounded px-4 py-2 cursor-not-allowed focus:outline-none"
                                />
                            </div>
                        </div>

                        {/* Driver Number */}
                         <div className="grid grid-cols-1 md:grid-cols-4 gap-4 items-center">
                            <label className="text-slate-400 font-medium md:text-right">Driver number:</label>
                            <div className="md:col-span-3 flex items-center">
                                <div className="relative">
                                     <input 
                                        type="number" 
                                        value={profile.driverNumber}
                                        onChange={(e) => setProfile({...profile, driverNumber: parseInt(e.target.value)})}
                                        className="w-24 bg-slate-100 border border-slate-300 text-slate-800 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                                    />
                                </div>
                            </div>
                        </div>

                         {/* Country */}
                         <div className="grid grid-cols-1 md:grid-cols-4 gap-4 items-center">
                            <label className="text-slate-400 font-medium md:text-right">Country:</label>
                            <div className="md:col-span-3">
                                <select 
                                    value={profile.country}
                                    onChange={(e) => setProfile({...profile, country: e.target.value})}
                                    className="w-full bg-slate-100 border border-slate-300 text-slate-800 rounded px-4 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                                >
                                    {countries.map(c => <option key={c} value={c}>{c}</option>)}
                                </select>
                            </div>
                        </div>

                         {/* Default Category */}
                         <div className="grid grid-cols-1 md:grid-cols-4 gap-4 items-center">
                            <label className="text-slate-400 font-medium md:text-right">Default category:</label>
                            <div className="md:col-span-3 flex gap-2">
                                <select 
                                    value={profile.category}
                                    onChange={(e) => setProfile({...profile, category: e.target.value})}
                                    className="flex-1 bg-slate-100 border border-slate-300 text-slate-800 rounded px-4 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                                >
                                    {categories.map(c => <option key={c} value={c}>{c}</option>)}
                                </select>
                                <button type="button" className="bg-orange-500 hover:bg-orange-600 text-white p-2 rounded transition">
                                    <Plus className="w-5 h-5" />
                                </button>
                            </div>
                        </div>

                        {/* Action Buttons */}
                        <div className="flex flex-col sm:flex-row gap-4 pt-6 border-t border-slate-700/50 mt-8">
                            <button 
                                type="button" 
                                onClick={handleRemoveAccount}
                                className="bg-red-600 hover:bg-red-700 text-white px-6 py-2 rounded flex items-center justify-center gap-2 transition"
                            >
                                <Trash2 className="w-4 h-4" />
                                Remove Account
                            </button>
                            <div className="flex-1"></div>
                            <button 
                                type="submit" 
                                className="bg-[#007cc3] hover:bg-[#006bb3] text-white px-6 py-2 rounded flex items-center justify-center gap-2 transition shadow-md"
                            >
                                <Save className="w-4 h-4" />
                                Save modifications
                            </button>
                        </div>

                    </form>
                </div>
            </div>

          </div>

          {/* RIGHT COLUMN: Settings & Devices */}
          <div className="space-y-6">
            
            {/* Change Password Card */}
            <div className="bg-slate-200 rounded-xl overflow-hidden shadow-lg text-slate-800">
                <div className="p-4 border-b border-slate-300 flex items-center justify-between">
                     <h3 className="text-lg font-semibold flex items-center gap-2 text-slate-700">
                        <Lock className="w-4 h-4" />
                        Change Password
                     </h3>
                     <ChevronDown className="w-4 h-4 text-slate-500" />
                </div>
                <div className="p-6">
                    <form onSubmit={handlePasswordChange} className="space-y-4">
                        <div>
                            <label className="block text-sm text-slate-600 mb-1">Old password:</label>
                            <input 
                                type="password" 
                                value={passwords.old}
                                onChange={(e) => setPasswords({...passwords, old: e.target.value})}
                                className="w-full bg-white border border-slate-300 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                            />
                        </div>
                        <div>
                            <label className="block text-sm text-slate-600 mb-1">New password:</label>
                            <input 
                                type="password" 
                                value={passwords.new}
                                onChange={(e) => setPasswords({...passwords, new: e.target.value})}
                                className="w-full bg-white border border-slate-300 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                            />
                        </div>
                        <div>
                            <label className="block text-sm text-slate-600 mb-1">Confirm password:</label>
                            <input 
                                type="password" 
                                value={passwords.confirm}
                                onChange={(e) => setPasswords({...passwords, confirm: e.target.value})}
                                className="w-full bg-white border border-slate-300 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                            />
                        </div>
                        <button 
                            type="submit" 
                            className="w-full bg-[#007cc3] hover:bg-[#006bb3] text-white py-2 rounded mt-2 transition font-medium"
                        >
                            Change password
                        </button>
                    </form>
                </div>
            </div>

            {/* Devices Card */}
            <div className="bg-slate-200 rounded-xl overflow-hidden shadow-lg text-slate-800">
                 <div className="p-4 border-b border-slate-300 flex items-center justify-between">
                     <h3 className="text-lg font-semibold flex items-center gap-2 text-slate-700">
                        <Smartphone className="w-4 h-4" />
                        Devices
                     </h3>
                     <ChevronDown className="w-4 h-4 text-slate-500" />
                </div>
                 <div className="p-0">
                    <table className="w-full text-sm">
                        <thead className="bg-slate-300 text-slate-600">
                            <tr>
                                <th className="px-4 py-2 text-left w-12">#</th>
                                <th className="px-4 py-2 text-left">Device</th>
                                <th className="px-4 py-2 text-left">Activated</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-slate-300 bg-white">
                            {devices.map((device, index) => (
                                <tr key={device.id} className="hover:bg-slate-50">
                                    <td className="px-4 py-3 text-slate-500">{index + 1}</td>
                                    <td className="px-4 py-3 font-mono text-slate-700">{device.id}</td>
                                    <td className="px-4 py-3 text-slate-600">{device.activated ? 'YES' : 'NO'}</td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                     {devices.length === 0 && (
                        <div className="p-4 text-center text-slate-500 italic">No devices registered</div>
                    )}
                 </div>
            </div>

          </div>
        </div>
        
        {/* Footer */}
        <div className="mt-12 text-center text-slate-500 text-sm">
            Much Racing - 2026 - <a href="#" className="text-[#007cc3] hover:underline">Contact us</a>
        </div>
      </main>
    </div>
  );
}
