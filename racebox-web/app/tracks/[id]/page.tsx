'use client';

import Link from 'next/link';
import { MapPin, ArrowLeft, Clock, TrendingUp, Users, Calendar } from 'lucide-react';
import { useParams } from 'next/navigation';

// Mock track data
const trackData: any = {
  '1': {
    id: 1,
    name: 'Genk Karting Belgium',
    country: 'Belgium',
    city: 'Genk',
    length: 1360,
    type: 'Karting',
    bestLap: '48.234',
    description: 'One of the most challenging karting circuits in Europe, featuring technical corners and high-speed sections.',
    coordinates: { lat: 50.9667, lng: 5.5833 },
    sectors: 3,
    turns: 18,
    elevation: 12,
    sessions: 156,
    avgLapTime: '51.234',
  },
  '2': {
    id: 2,
    name: 'Sepang International Circuit',
    country: 'Malaysia',
    city: 'Sepang',
    length: 5543,
    type: 'Circuit',
    bestLap: '1:31.219',
    description: 'World-class FIA Grade 1 circuit, home to the Malaysian Grand Prix.',
    coordinates: { lat: 2.7608, lng: 101.7380 },
    sectors: 3,
    turns: 15,
    elevation: 8,
    sessions: 89,
    avgLapTime: '1:35.891',
  },
  '3': {
    id: 3,
    name: 'Sentul Karting Circuit',
    country: 'Indonesia',
    city: 'Bogor',
    length: 1200,
    type: 'Karting',
    bestLap: '52.891',
    description: 'Technical karting circuit located in Sentul International Circuit complex.',
    coordinates: { lat: -6.6833, lng: 106.8333 },
    sectors: 2,
    turns: 14,
    elevation: 15,
    sessions: 234,
    avgLapTime: '54.567',
  },
};

// Mock leaderboard data
const leaderboard = [
  { rank: 1, driver: 'John Doe', lapTime: '48.234', date: '2026-01-05', kart: 'CRG Road Rebel' },
  { rank: 2, driver: 'Jane Smith', lapTime: '48.891', date: '2026-01-03', kart: 'Tony Kart Racer' },
  { rank: 3, driver: 'Mike Johnson', lapTime: '49.123', date: '2025-12-28', kart: 'Birel ART' },
  { rank: 4, driver: 'Sarah Williams', lapTime: '49.456', date: '2025-12-20', kart: 'CRG Road Rebel' },
  { rank: 5, driver: 'Tom Brown', lapTime: '49.789', date: '2025-12-15', kart: 'Kosmic Mercury' },
];

export default function TrackDetailPage() {
  const params = useParams();
  const trackId = params.id as string;
  const track = trackData[trackId];

  if (!track) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 flex items-center justify-center">
        <div className="text-white text-2xl">Track not found</div>
      </div>
    );
  }

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
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
              <Link href="/tracks" className="text-white font-semibold">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-12">
        {/* Back Button */}
        <Link href="/tracks" className="inline-flex items-center text-slate-300 hover:text-white transition mb-6">
          <ArrowLeft className="w-5 h-5 mr-2" />
          Back to Tracks
        </Link>

        {/* Track Header */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <div className="flex flex-col md:flex-row md:items-start md:justify-between mb-6">
            <div>
              <div className="flex items-center space-x-3 mb-2">
                <h1 className="text-4xl font-bold text-white">{track.name}</h1>
                <span className="bg-orange-500/20 text-orange-400 px-3 py-1 rounded-lg text-sm">
                  {track.type}
                </span>
              </div>
              <p className="text-slate-400 text-lg flex items-center">
                <MapPin className="w-5 h-5 mr-2" />
                {track.city}, {track.country}
              </p>
            </div>
            <div className="mt-4 md:mt-0">
              <div className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600">
                {track.bestLap}
              </div>
              <div className="text-slate-400 text-center">Best Lap Time</div>
            </div>
          </div>

          <p className="text-slate-300 mb-6">{track.description}</p>

          {/* Track Stats */}
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            <StatBox label="Length" value={`${track.length}m`} />
            <StatBox label="Turns" value={track.turns.toString()} />
            <StatBox label="Sectors" value={track.sectors.toString()} />
            <StatBox label="Sessions" value={track.sessions.toString()} />
          </div>
        </div>

        {/* Map Section */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <h2 className="text-2xl font-bold text-white mb-4">Track Map</h2>
          
          {/* Map Placeholder */}
          <div className="h-96 bg-gradient-to-br from-slate-700 to-slate-800 rounded-lg flex items-center justify-center border border-slate-600">
            <div className="text-center">
              <MapPin className="w-16 h-16 text-orange-500 mx-auto mb-4" />
              <p className="text-slate-400">Interactive map will be displayed here</p>
              <p className="text-slate-500 text-sm mt-2">
                Coordinates: {track.coordinates.lat}, {track.coordinates.lng}
              </p>
            </div>
          </div>
        </div>

        {/* Leaderboard */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8">
          <div className="flex items-center justify-between mb-6">
            <h2 className="text-2xl font-bold text-white">Best Lap Times</h2>
            <span className="text-slate-400 flex items-center">
              <Users className="w-5 h-5 mr-2" />
              {leaderboard.length} drivers
            </span>
          </div>

          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-slate-700">
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Rank</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Driver</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Lap Time</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Date</th>
                  <th className="text-left text-slate-400 font-semibold py-3 px-4">Kart</th>
                </tr>
              </thead>
              <tbody>
                {leaderboard.map((entry) => (
                  <tr key={entry.rank} className="border-b border-slate-700/50 hover:bg-slate-700/30 transition">
                    <td className="py-4 px-4">
                      <div className={`w-8 h-8 rounded-full flex items-center justify-center font-bold ${
                        entry.rank === 1 ? 'bg-yellow-500/20 text-yellow-400' :
                        entry.rank === 2 ? 'bg-slate-400/20 text-slate-300' :
                        entry.rank === 3 ? 'bg-orange-700/20 text-orange-400' :
                        'bg-slate-700 text-slate-400'
                      }`}>
                        {entry.rank}
                      </div>
                    </td>
                    <td className="py-4 px-4 text-white font-semibold">{entry.driver}</td>
                    <td className="py-4 px-4">
                      <span className="text-orange-400 font-mono font-semibold">{entry.lapTime}</span>
                    </td>
                    <td className="py-4 px-4 text-slate-300">{entry.date}</td>
                    <td className="py-4 px-4 text-slate-400">{entry.kart}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
}

function StatBox({ label, value }: { label: string; value: string }) {
  return (
    <div className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
      <div className="text-slate-500 text-sm mb-1">{label}</div>
      <div className="text-white text-2xl font-bold">{value}</div>
    </div>
  );
}
