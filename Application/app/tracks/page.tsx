import Link from 'next/link';
import { MapPin, Search, Filter, Zap } from 'lucide-react';


// Mock track data
// Mock track data - expanded
const tracks = [
  { id: 1, name: 'Genk Karting Belgium', country: 'Belgium', city: 'Genk', length: 1360, type: 'Karting', bestLap: '48.234', flag: '🇧🇪' },
  { id: 2, name: 'Sepang International Circuit', country: 'Malaysia', city: 'Sepang', length: 5543, type: 'Circuit', bestLap: '1:31.219', flag: '🇲🇾' },
  { id: 3, name: 'Sentul Karting Circuit', country: 'Indonesia', city: 'Bogor', length: 1200, type: 'Karting', bestLap: '52.891', flag: '🇮🇩' },
  { id: 4, name: 'Marina Bay Street Circuit', country: 'Singapore', city: 'Singapore', length: 5063, type: 'Street', bestLap: '1:41.905', flag: '🇸🇬' },
  { id: 5, name: 'Suzuka Circuit', country: 'Japan', city: 'Suzuka', length: 5807, type: 'Circuit', bestLap: '1:30.983', flag: '🇯🇵' },
  { id: 6, name: 'BSD Karting Track', country: 'Indonesia', city: 'Tangerang', length: 980, type: 'Karting', bestLap: '45.123', flag: '🇮🇩' },
  { id: 7, name: 'Chang International Circuit', country: 'Thailand', city: 'Buriram', length: 4554, type: 'Circuit', bestLap: '1:23.456', flag: '🇹🇭' },
  { id: 8, name: 'Phillip Island', country: 'Australia', city: 'Victoria', length: 4445, type: 'Circuit', bestLap: '1:28.123', flag: '🇦🇺' },
  { id: 9, name: 'Mandalika International Circuit', country: 'Indonesia', city: 'Lombok', length: 4310, type: 'Circuit', bestLap: '1:32.444', flag: '🇮🇩' },
];

export default function TracksPage() {
  return (
    <div className="min-h-screen bg-background text-foreground pb-24">
      {/* Header with Carbon Fiber */}


      {/* Content */}
      <div className="container mx-auto px-4 py-6">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-6">
          <div>
            <p className="text-text-secondary text-sm">Browse and discover racing tracks</p>
          </div>
          <Link href="/tracks/create" className="mt-3 md:mt-0 bg-primary hover:bg-primary-hover text-white px-4 py-2 rounded-lg transition inline-flex items-center gap-2 font-racing text-sm">
            <MapPin className="w-4 h-4" />
            CREATE TRACK
          </Link>
        </div>

        {/* Search and Filter */}
        <div className="carbon-bg border border-border-color rounded-xl p-4 mb-6">
          <div className="grid md:grid-cols-3 gap-3">
            <div className="md:col-span-2">
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-text-secondary w-4 h-4" />
                <input
                  type="text"
                  placeholder="Search tracks by name, location..."
                  className="w-full bg-background-secondary border border-border-color rounded-lg pl-10 pr-4 py-2 text-foreground placeholder-text-secondary focus:outline-none focus:border-primary transition text-sm"
                />
              </div>
            </div>
            <div>
              <button className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-2 text-foreground hover:border-primary transition flex items-center justify-center gap-2 font-racing text-sm">
                <Filter className="w-4 h-4" />
                FILTERS
              </button>
            </div>
          </div>
        </div>

        {/* Track Grid */}
        <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-4 mb-6">
          {tracks.map((track) => (
            <TrackCard key={track.id} track={track} />
          ))}
        </div>

        {/* Stats */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <StatCard number="500+" label="TOTAL TRACKS" />
          <StatCard number="48" label="COUNTRIES" />
          <StatCard number="10K+" label="SESSIONS" />
          <StatCard number="50K+" label="LAPS" />
        </div>
      </div>

    </div>
  );
}

function TrackCard({ track }: { track: any }) {
  return (
    <Link href={`/tracks/${track.id}`}>
      <div className="carbon-bg border border-border-color rounded-xl overflow-hidden hover:border-primary/50 transition group">
        {/* Track Map Placeholder */}
        <div className="h-40 bg-gradient-to-br from-background-secondary to-background flex items-center justify-center">
          <MapPin className="w-12 h-12 text-text-secondary group-hover:text-primary transition" />
        </div>

        {/* Track Info */}
        <div className="p-4">
          <div className="flex items-start justify-between mb-2">
            <h3 className="text-foreground font-racing text-sm group-hover:text-primary transition flex-1">
              {track.name}
            </h3>
            <span className="bg-primary/20 text-primary text-xs px-2 py-1 rounded font-bold ml-2">
              {track.type}
            </span>
          </div>

          <p className="text-text-secondary text-xs mb-3 flex items-center gap-1">
            <span>{track.flag}</span>
            {track.city}, {track.country}
          </p>

          <div className="grid grid-cols-2 gap-3 text-xs">
            <div>
              <div className="text-text-secondary">Length</div>
              <div className="text-foreground font-data font-bold">{track.length}m</div>
            </div>
            <div>
              <div className="text-text-secondary">Best Lap</div>
              <div className="text-highlight font-data font-bold">{track.bestLap}</div>
            </div>
          </div>
        </div>
      </div>
    </Link>
  );
}

function StatCard({ number, label }: { number: string; label: string }) {
  return (
    <div className="carbon-bg border border-border-color rounded-xl p-4 text-center">
      <div className="text-2xl font-data font-bold text-transparent bg-clip-text bg-gradient-to-r from-primary to-highlight mb-1">
        {number}
      </div>
      <div className="text-text-secondary text-xs font-racing">{label}</div>
    </div>
  );
}
