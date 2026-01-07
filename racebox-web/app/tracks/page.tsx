import Link from 'next/link';
import { MapPin, Search, Filter } from 'lucide-react';

// Mock track data
const tracks = [
  { id: 1, name: 'Genk Karting Belgium', country: 'Belgium', city: 'Genk', length: 1360, type: 'Karting', bestLap: '48.234' },
  { id: 2, name: 'Sepang International Circuit', country: 'Malaysia', city: 'Sepang', length: 5543, type: 'Circuit', bestLap: '1:31.219' },
  { id: 3, name: 'Sentul Karting Circuit', country: 'Indonesia', city: 'Bogor', length: 1200, type: 'Karting', bestLap: '52.891' },
  { id: 4, name: 'Marina Bay Street Circuit', country: 'Singapore', city: 'Singapore', length: 5063, type: 'Street', bestLap: '1:41.905' },
  { id: 5, name: 'Suzuka Circuit', country: 'Japan', city: 'Suzuka', length: 5807, type: 'Circuit', bestLap: '1:30.983' },
  { id: 6, name: 'BSD Karting Track', country: 'Indonesia', city: 'Tangerang', length: 980, type: 'Karting', bestLap: '45.123' },
];

export default function TracksPage() {
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

      {/* Header */}
      <div className="container mx-auto px-6 py-12">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-8">
          <div>
            <h1 className="text-4xl font-bold text-white mb-2">Track Database</h1>
            <p className="text-slate-400">Browse and discover racing tracks worldwide</p>
          </div>
          <Link href="/tracks/create" className="mt-4 md:mt-0 bg-orange-500 hover:bg-orange-600 text-white px-6 py-3 rounded-lg transition inline-flex items-center">
            <MapPin className="w-5 h-5 mr-2" />
            Create New Track
          </Link>
        </div>

        {/* Search and Filter */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 mb-8">
          <div className="grid md:grid-cols-3 gap-4">
            <div className="md:col-span-2">
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-slate-400 w-5 h-5" />
                <input
                  type="text"
                  placeholder="Search tracks by name, location..."
                  className="w-full bg-slate-900 border border-slate-700 rounded-lg pl-10 pr-4 py-3 text-white placeholder-slate-400 focus:outline-none focus:border-orange-500"
                />
              </div>
            </div>
            <div>
              <button className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-3 text-white hover:border-orange-500 transition flex items-center justify-center">
                <Filter className="w-5 h-5 mr-2" />
                Filters
              </button>
            </div>
          </div>
        </div>

        {/* Track Grid */}
        <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-6">
          {tracks.map((track) => (
            <TrackCard key={track.id} track={track} />
          ))}
        </div>

        {/* Stats */}
        <div className="mt-12 grid grid-cols-2 md:grid-cols-4 gap-6">
          <StatCard number="500+" label="Total Tracks" />
          <StatCard number="48" label="Countries" />
          <StatCard number="10K+" label="Sessions" />
          <StatCard number="50K+" label="Laps" />
        </div>
      </div>
    </div>
  );
}

function TrackCard({ track }: { track: any }) {
  return (
    <Link href={`/tracks/${track.id}`}>
      <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl overflow-hidden hover:border-orange-500 transition group">
        {/* Track Map Placeholder */}
        <div className="h-48 bg-gradient-to-br from-slate-700 to-slate-800 flex items-center justify-center">
          <MapPin className="w-16 h-16 text-slate-600 group-hover:text-orange-500 transition" />
        </div>
        
        {/* Track Info */}
        <div className="p-6">
          <div className="flex items-start justify-between mb-2">
            <h3 className="text-white font-semibold text-lg group-hover:text-orange-500 transition">
              {track.name}
            </h3>
            <span className="bg-orange-500/20 text-orange-400 text-xs px-2 py-1 rounded">
              {track.type}
            </span>
          </div>
          
          <p className="text-slate-400 text-sm mb-4">
            {track.city}, {track.country}
          </p>
          
          <div className="grid grid-cols-2 gap-4 text-sm">
            <div>
              <div className="text-slate-500">Length</div>
              <div className="text-white font-semibold">{track.length}m</div>
            </div>
            <div>
              <div className="text-slate-500">Best Lap</div>
              <div className="text-orange-400 font-semibold">{track.bestLap}</div>
            </div>
          </div>
        </div>
      </div>
    </Link>
  );
}

function StatCard({ number, label }: { number: string; label: string }) {
  return (
    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 text-center">
      <div className="text-3xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600 mb-2">
        {number}
      </div>
      <div className="text-slate-400">{label}</div>
    </div>
  );
}
