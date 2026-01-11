'use client';

import Link from 'next/link';
import { Clock, TrendingUp, MapPin, Calendar, Search, Edit, Trash2, Upload, ChevronLeft, ChevronRight } from 'lucide-react';
import { useState } from 'react';
import MapWrapper from "../components/MapWrapper";

// Mock data - expanded
const stats = {
  totalSessions: 24,
  totalLaps: 486,
  bestLapTime: '48.234',
  avgLapTime: '52.891',
};

const allSessions = [
  { id: 1, driver: 'FARIS', track: 'Sentul Karting Circuit', city: 'Bogor', date: '2026-01-06', time: '14:30', category: 'Vespa Tune Up', laps: 25, bestLap: '48.234' },
  { id: 2, driver: 'FARIS', track: 'BSD Karting Track', city: 'Tangerang', date: '2026-01-05', time: '16:45', category: 'Vespa Tune Up', laps: 18, bestLap: '45.891' },
  { id: 3, driver: 'FARIS', track: 'Sentul Karting Circuit', city: 'Bogor', date: '2026-01-03', time: '10:20', category: 'Vespa Tune Up', laps: 22, bestLap: '52.891' },
  { id: 4, driver: 'FARIS', track: 'Genk Karting Belgium', city: 'Genk', date: '2025-12-28', time: '15:00', category: 'Rental Kart', laps: 20, bestLap: '49.123' },
  { id: 5, driver: 'FARIS', track: 'BSD Karting Track', city: 'Tangerang', date: '2025-12-20', time: '11:30', category: 'Vespa Tune Up', laps: 16, bestLap: '46.234' },
];

const trackGroups = [
  { id: 1, name: 'Sentul Karting Circuit', city: 'Bogor', country: 'Indonesia', sessions: 8, flag: '🇮🇩' },
  { id: 2, name: 'BSD Karting Track', city: 'Tangerang', country: 'Indonesia', sessions: 6, flag: '🇮🇩' },
  { id: 3, name: 'Genk Karting Belgium', city: 'Genk', country: 'Belgium', sessions: 4, flag: '🇧🇪' },
];

export default function DashboardPage() {
  const [searchQuery, setSearchQuery] = useState('');
  const [rowsPerPage, setRowsPerPage] = useState(10);
  const [currentPage, setCurrentPage] = useState(1);

  const filteredSessions = allSessions.filter(session =>
    session.track.toLowerCase().includes(searchQuery.toLowerCase()) ||
    session.city.toLowerCase().includes(searchQuery.toLowerCase()) ||
    session.category.toLowerCase().includes(searchQuery.toLowerCase())
  );

  const totalPages = Math.ceil(filteredSessions.length / rowsPerPage);
  const startIndex = (currentPage - 1) * rowsPerPage;
  const displayedSessions = filteredSessions.slice(startIndex, startIndex + rowsPerPage);

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900">
      {/* Navigation */}
      <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
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
            <div className="hidden md:flex space-x-6">
              <Link href="/dashboard" className="text-white font-semibold">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Dashboard Content */}
      <div className="container mx-auto px-6 py-12">
        <div className="mb-8">
          <h1 className="text-4xl font-bold text-white mb-2">Dashboard</h1>
          <p className="text-slate-400">Welcome back! Here's your racing overview</p>
        </div>

        {/* Stats Grid */}
        <div className="grid md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
          <StatCard
            icon={<Calendar className="w-6 h-6" />}
            label="Total Sessions"
            value={stats.totalSessions.toString()}
            color="orange"
          />
          <StatCard
            icon={<TrendingUp className="w-6 h-6" />}
            label="Total Laps"
            value={stats.totalLaps.toString()}
            color="blue"
          />
          <StatCard
            icon={<Clock className="w-6 h-6" />}
            label="Best Lap Time"
            value={stats.bestLapTime}
            color="green"
          />
          <StatCard
            icon={<Clock className="w-6 h-6" />}
            label="Avg Lap Time"
            value={stats.avgLapTime}
            color="purple"
          />
        </div>

        {/* Live Telemetry Section */}
        <div className="mb-8">
          <h2 className="text-2xl font-bold text-white mb-6">Live Telemetry</h2>
          <div className="w-full h-[400px] bg-slate-800 rounded-xl overflow-hidden border border-slate-700 relative shadow-lg">
            <MapWrapper />
          </div>
        </div>

        {/* GPX Upload Widget */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 mb-8">
          <h2 className="text-2xl font-bold text-white mb-6 flex items-center">
            <Upload className="w-6 h-6 mr-2 text-orange-500" />
            Upload GPX Sessions
          </h2>

          <div className="border-2 border-dashed border-slate-700 rounded-xl p-8 text-center hover:border-orange-500 transition cursor-pointer group relative">
            <input
              type="file"
              multiple
              accept=".gpx"
              className="absolute inset-0 w-full h-full opacity-0 cursor-pointer"
              onChange={(e) => {
                // Mock upload logic
                const files = e.target.files;
                if (files && files.length > 0) {
                  // Simulate upload progress
                  const progressBar = document.getElementById('progress_bar');
                  const progressText = document.getElementById('progress_text');
                  if (progressBar && progressText) {
                    progressBar.style.width = '0%';
                    progressText.innerText = '0%';

                    let progress = 0;
                    const interval = setInterval(() => {
                      progress += 5;
                      progressBar.style.width = `${progress}%`;
                      progressText.innerText = `${progress}%`;
                      if (progress >= 100) {
                        clearInterval(interval);
                        setTimeout(() => {
                          alert(`Successfully uploaded ${files.length} GPX files!`);
                          progressBar.style.width = '0%';
                          progressText.innerText = '';
                        }, 500);
                      }
                    }, 100);
                  }
                }
              }}
            />
            <div className="flex flex-col items-center justify-center pointer-events-none">
              <div className="bg-slate-800 p-4 rounded-full mb-4 group-hover:bg-slate-700 transition">
                <Upload className="w-8 h-8 text-slate-400 group-hover:text-orange-500 transition" />
              </div>
              <p className="text-lg text-slate-300 font-semibold mb-2">
                Drag & Drop GPX files here
              </p>
              <p className="text-slate-500 text-sm">
                or click to select manually
              </p>
            </div>
          </div>

          {/* Progress Bar Mockup */}
          <div className="mt-6">
            <div className="flex items-center justify-between mb-2">
              <span className="text-slate-400 text-sm">Upload Status</span>
              <span id="progress_text" className="text-orange-400 text-sm font-bold"></span>
            </div>
            <div className="w-full bg-slate-900 rounded-full h-2 overflow-hidden">
              <div id="progress_bar" className="bg-gradient-to-r from-orange-500 to-red-600 h-full rounded-full transition-all duration-200" style={{ width: '0%' }}></div>
            </div>
          </div>
        </div>

        {/* All My Sessions Table */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 mb-8">
          <div className="flex items-center justify-between mb-6">
            <h2 className="text-2xl font-bold text-white">All My Sessions</h2>
            <button className="bg-orange-500 hover:bg-orange-600 text-white px-4 py-2 rounded-lg transition flex items-center">
              <Upload className="w-4 h-4 mr-2" />
              Upload GPX
            </button>
          </div>

          {/* Search */}
          <div className="mb-4">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-slate-400 w-5 h-5" />
              <input
                type="text"
                placeholder="Search sessions..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="w-full bg-slate-900 border border-slate-700 rounded-lg pl-10 pr-4 py-2 text-white placeholder-slate-400 focus:outline-none focus:border-orange-500"
              />
            </div>
          </div>

          {/* Table */}
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-slate-700">
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">ID</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Driver</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Date/Time</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Track</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">City</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Category</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Laps</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Best Lap</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Actions</th>
                </tr>
              </thead>
              <tbody>
                {displayedSessions.map((session) => (
                  <tr key={session.id} className="border-b border-slate-700/50 hover:bg-slate-700/30 transition">
                    <td className="py-3 px-4 text-slate-300">{session.id}</td>
                    <td className="py-3 px-4 text-white font-semibold">{session.driver}</td>
                    <td className="py-3 px-4 text-slate-300">
                      <Link href={`/sessions/${session.id}`} className="text-orange-400 hover:text-orange-300">
                        {session.date} {session.time}
                      </Link>
                    </td>
                    <td className="py-3 px-4 text-white">{session.track}</td>
                    <td className="py-3 px-4 text-slate-300">{session.city}</td>
                    <td className="py-3 px-4 text-slate-300">{session.category}</td>
                    <td className="py-3 px-4 text-white">{session.laps}</td>
                    <td className="py-3 px-4 text-orange-400 font-mono font-semibold">{session.bestLap}</td>
                    <td className="py-3 px-4">
                      <div className="flex space-x-2">
                        <button className="text-blue-400 hover:text-blue-300 transition">
                          <Edit className="w-4 h-4" />
                        </button>
                        <button className="text-red-400 hover:text-red-300 transition">
                          <Trash2 className="w-4 h-4" />
                        </button>
                      </div>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>

          {/* Pagination */}
          <div className="flex items-center justify-between mt-4">
            <div className="flex items-center space-x-2">
              <span className="text-slate-400 text-sm">Show</span>
              <select
                value={rowsPerPage}
                onChange={(e) => setRowsPerPage(Number(e.target.value))}
                className="bg-slate-900 border border-slate-700 rounded px-2 py-1 text-white text-sm focus:outline-none focus:border-orange-500"
              >
                <option value={10}>10</option>
                <option value={25}>25</option>
                <option value={50}>50</option>
                <option value={100}>100</option>
              </select>
              <span className="text-slate-400 text-sm">entries</span>
            </div>
            <div className="flex items-center space-x-2">
              <button
                onClick={() => setCurrentPage(Math.max(1, currentPage - 1))}
                disabled={currentPage === 1}
                className="p-2 bg-slate-900 border border-slate-700 rounded hover:border-orange-500 transition disabled:opacity-50 disabled:cursor-not-allowed"
              >
                <ChevronLeft className="w-4 h-4 text-white" />
              </button>
              <span className="text-slate-300 text-sm">
                Page {currentPage} of {totalPages}
              </span>
              <button
                onClick={() => setCurrentPage(Math.min(totalPages, currentPage + 1))}
                disabled={currentPage === totalPages}
                className="p-2 bg-slate-900 border border-slate-700 rounded hover:border-orange-500 transition disabled:opacity-50 disabled:cursor-not-allowed"
              >
                <ChevronRight className="w-4 h-4 text-white" />
              </button>
            </div>
          </div>
        </div>

        {/* Track Groups */}
        <div className="mb-8">
          <h2 className="text-2xl font-bold text-white mb-6">Sessions by Track</h2>
          <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-6">
            {trackGroups.map((track) => (
              <TrackGroupCard key={track.id} track={track} />
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}

function StatCard({ icon, label, value, color }: { icon: React.ReactNode; label: string; value: string; color: string }) {
  const colorClasses = {
    orange: 'from-orange-500 to-red-600',
    blue: 'from-blue-500 to-cyan-600',
    green: 'from-green-500 to-emerald-600',
    purple: 'from-purple-500 to-pink-600',
  };

  return (
    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
      <div className={`inline-flex p-3 rounded-lg bg-gradient-to-br ${colorClasses[color as keyof typeof colorClasses]} mb-4`}>
        <div className="text-white">{icon}</div>
      </div>
      <div className="text-slate-400 text-sm mb-1">{label}</div>
      <div className="text-3xl font-bold text-white">{value}</div>
    </div>
  );
}

function TrackGroupCard({ track }: { track: any }) {
  return (
    <Link href={`/tracks/${track.id}`}>
      <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl overflow-hidden hover:border-orange-500 transition group">
        {/* Track Map Placeholder */}
        <div className="h-40 bg-gradient-to-br from-slate-700 to-slate-800 flex items-center justify-center">
          <MapPin className="w-12 h-12 text-slate-600 group-hover:text-orange-500 transition" />
        </div>

        {/* Track Info */}
        <div className="p-6">
          <div className="flex items-start justify-between mb-2">
            <h3 className="text-white font-semibold text-lg group-hover:text-orange-500 transition">
              {track.name}
            </h3>
            <span className="text-2xl">{track.flag}</span>
          </div>

          <p className="text-slate-400 text-sm mb-4">
            {track.city}, {track.country}
          </p>

          <div className="flex items-center justify-between">
            <span className="text-slate-500 text-sm">Sessions</span>
            <span className="text-orange-400 font-bold text-xl">{track.sessions}</span>
          </div>
        </div>
      </div>
    </Link>
  );
}
