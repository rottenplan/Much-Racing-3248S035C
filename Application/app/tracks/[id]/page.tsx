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
    <div className="min-h-screen bg-background text-foreground pb-24">
      {/* Navigation - replaced by BottomNav mostly, but keeping header for detail */}
      <header className="carbon-bg backdrop-blur-md border-b border-border-color sticky top-0 z-20">
        <div className="container mx-auto px-4 py-4 flex items-center justify-between">
          <Link href="/tracks" className="p-2 -ml-2 hover:bg-white/5 rounded-full transition">
            <ArrowLeft className="w-6 h-6 text-foreground" />
          </Link>
          <div className="text-center">
            <h1 className="text-lg font-racing tracking-wide truncate max-w-[200px]">{track.name}</h1>
          </div>
          <div className="w-10"></div> {/* Spacer */}
        </div>
      </header>

      {/* Content */}
      <div className="container mx-auto px-4 py-6 space-y-6">

        {/* Track Header Card */}
        <div className="carbon-bg border border-border-color rounded-xl p-6 relative overflow-hidden">
          {/* Background decorative element */}
          <div className="absolute top-0 right-0 p-10 opacity-5">
            <MapPin className="w-64 h-64 text-primary" />
          </div>

          <div className="flex flex-col md:flex-row md:items-start md:justify-between mb-6 relative z-10">
            <div>
              <div className="flex items-center space-x-3 mb-2">
                <h2 className="text-2xl font-racing text-white">{track.name}</h2>
                <span className="bg-primary/20 text-primary text-xs px-2 py-1 rounded font-bold">
                  {track.type}
                </span>
              </div>
              <p className="text-text-secondary text-sm flex items-center">
                <MapPin className="w-4 h-4 mr-1" />
                {track.city}, {track.country}
              </p>
            </div>
            <div className="mt-4 md:mt-0 text-right">
              <div className="text-3xl font-data font-bold text-highlight">
                {track.bestLap}
              </div>
              <div className="text-text-secondary text-xs uppercase tracking-wider">Best Lap Time</div>
            </div>
          </div>

          <p className="text-foreground/80 text-sm mb-6 relative z-10 max-w-2xl">{track.description}</p>

          {/* Track Stats */}
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4 relative z-10">
            <StatBox label="LENGTH" value={`${track.length}m`} />
            <StatBox label="TURNS" value={track.turns.toString()} />
            <StatBox label="SECTORS" value={track.sectors.toString()} />
            <StatBox label="SESSIONS" value={track.sessions.toString()} />
          </div>
        </div>

        {/* Map Section */}
        <div className="carbon-bg border border-border-color rounded-xl p-6">
          <h2 className="text-xl font-racing text-white mb-4 flex items-center gap-2">
            <MapPin className="w-5 h-5 text-primary" />
            TRACK MAP
          </h2>

          {/* Map Placeholder */}
          <div className="h-64 bg-background-secondary rounded-lg flex items-center justify-center border border-border-color relative overflow-hidden">
            <div className="absolute inset-0 bg-[linear-gradient(rgba(255,255,255,0.02)_1px,transparent_1px),linear-gradient(90deg,rgba(255,255,255,0.02)_1px,transparent_1px)] bg-[size:20px_20px]"></div>
            <div className="text-center relative z-10">
              <MapPin className="w-12 h-12 text-highlight mx-auto mb-2 opacity-80" />
              <p className="text-text-secondary text-sm">Interactive map visualization</p>
              <p className="text-text-secondary text-[10px] mt-1 font-data">
                {track.coordinates.lat}, {track.coordinates.lng}
              </p>
            </div>
          </div>
        </div>

        {/* Leaderboard */}
        <div className="carbon-bg border border-border-color rounded-xl overflow-hidden">
          <div className="p-4 border-b border-border-color flex items-center justify-between">
            <h2 className="text-xl font-racing text-white">LEADERBOARD</h2>
            <span className="text-text-secondary text-xs flex items-center bg-background-secondary px-2 py-1 rounded">
              <Users className="w-3 h-3 mr-1" />
              {leaderboard.length} drivers
            </span>
          </div>

          <div className="overflow-x-auto">
            <table className="w-full text-sm">
              <thead className="bg-background-secondary">
                <tr>
                  <th className="text-left text-text-secondary font-racing text-xs py-3 px-4">#</th>
                  <th className="text-left text-text-secondary font-racing text-xs py-3 px-4">DRIVER</th>
                  <th className="text-left text-text-secondary font-racing text-xs py-3 px-4">TIME</th>
                  <th className="text-left text-text-secondary font-racing text-xs py-3 px-4">DATE</th>
                  <th className="text-left text-text-secondary font-racing text-xs py-3 px-4">VEHICLE</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-border-color">
                {leaderboard.map((entry) => (
                  <tr key={entry.rank} className="hover:bg-card-bg transition">
                    <td className="py-3 px-4">
                      <div className={`w-6 h-6 rounded-full flex items-center justify-center font-bold text-xs ${entry.rank === 1 ? 'bg-highlight text-white' :
                          entry.rank === 2 ? 'bg-zinc-400 text-black' :
                            entry.rank === 3 ? 'bg-orange-700 text-white' :
                              'bg-background-secondary text-text-secondary'
                        }`}>
                        {entry.rank}
                      </div>
                    </td>
                    <td className="py-3 px-4 text-foreground font-medium">{entry.driver}</td>
                    <td className="py-3 px-4">
                      <span className="text-highlight font-data font-bold">{entry.lapTime}</span>
                    </td>
                    <td className="py-3 px-4 text-text-secondary text-xs">{entry.date}</td>
                    <td className="py-3 px-4 text-text-secondary text-xs">{entry.kart}</td>
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
    <div className="bg-background-secondary border border-border-color rounded-lg p-3 text-center">
      <div className="text-text-secondary text-[10px] font-racing mb-1">{label}</div>
      <div className="text-foreground text-lg font-data font-bold">{value}</div>
    </div>
  );
}
