import Link from 'next/link';
import { Clock, MapPin, Calendar, TrendingUp } from 'lucide-react';

// Mock sessions data
const sessions = [
  { id: 1, track: 'Genk Karting Belgium', date: '2026-01-06', time: '14:30', laps: 25, bestLap: '48.234', avgLap: '51.123', totalTime: '21:15' },
  { id: 2, track: 'BSD Karting Track', date: '2026-01-05', time: '16:45', laps: 18, bestLap: '45.891', avgLap: '47.234', totalTime: '14:10' },
  { id: 3, track: 'Sentul Karting Circuit', date: '2026-01-03', time: '10:20', laps: 22, bestLap: '52.891', avgLap: '54.567', totalTime: '19:45' },
  { id: 4, track: 'Genk Karting Belgium', date: '2025-12-28', time: '15:00', laps: 20, bestLap: '49.123', avgLap: '52.456', totalTime: '17:30' },
];

export default function SessionsPage() {
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
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-white font-semibold">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-12">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-8">
          <div>
            <h1 className="text-4xl font-bold text-white mb-2">My Sessions</h1>
            <p className="text-slate-400">View and analyze your racing sessions</p>
          </div>
          <Link href="/device" className="mt-4 md:mt-0 bg-orange-500 hover:bg-orange-600 text-white px-6 py-3 rounded-lg transition">
            Upload New Session
          </Link>
        </div>

        {/* Sessions List */}
        <div className="space-y-4">
          {sessions.map((session) => (
            <SessionCard key={session.id} session={session} />
          ))}
        </div>
      </div>
    </div>
  );
}

function SessionCard({ session }: { session: any }) {
  return (
    <Link href={`/sessions/${session.id}`}>
      <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 hover:border-orange-500 transition">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-4">
          <div>
            <h3 className="text-white text-xl font-semibold mb-2 flex items-center">
              <MapPin className="w-5 h-5 mr-2 text-orange-500" />
              {session.track}
            </h3>
            <div className="flex items-center space-x-4 text-sm text-slate-400">
              <span className="flex items-center">
                <Calendar className="w-4 h-4 mr-1" />
                {session.date}
              </span>
              <span className="flex items-center">
                <Clock className="w-4 h-4 mr-1" />
                {session.time}
              </span>
            </div>
          </div>
          <div className="mt-4 md:mt-0">
            <div className="text-3xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600">
              {session.bestLap}
            </div>
            <div className="text-slate-400 text-sm text-center">Best Lap</div>
          </div>
        </div>

        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 pt-4 border-t border-slate-700">
          <div>
            <div className="text-slate-500 text-sm">Total Laps</div>
            <div className="text-white font-semibold">{session.laps}</div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Avg Lap</div>
            <div className="text-white font-semibold">{session.avgLap}</div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Total Time</div>
            <div className="text-white font-semibold">{session.totalTime}</div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Improvement</div>
            <div className="text-green-400 font-semibold flex items-center">
              <TrendingUp className="w-4 h-4 mr-1" />
              -2.3s
            </div>
          </div>
        </div>
      </div>
    </Link>
  );
}
