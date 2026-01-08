import Link from 'next/link';
import { Clock, MapPin, Calendar, TrendingUp, Gauge, Activity } from 'lucide-react';
import fs from 'fs';
import path from 'path';

// Helper to get sessions from disk
async function getSessions() {
  const sessionsDir = path.join(process.cwd(), 'data', 'sessions');
  if (!fs.existsSync(sessionsDir)) {
    return [];
  }

  const files = fs.readdirSync(sessionsDir).filter(file => file.endsWith('.json'));
  const sessions = files.map(file => {
    try {
      const content = fs.readFileSync(path.join(sessionsDir, file), 'utf-8');
      return JSON.parse(content);
    } catch (e) {
      console.error(`Error reading session file ${file}`, e);
      return null;
    }
  }).filter(s => s !== null);

  // Sort by date desc
  return sessions.sort((a, b) => new Date(b.uploadDate).getTime() - new Date(a.uploadDate).getTime());
}

export default async function SessionsPage() {
  const sessions = await getSessions();

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
          {sessions.length === 0 ? (
            <div className="text-center py-20 bg-slate-800/30 rounded-xl border border-dashed border-slate-700">
              <p className="text-slate-400 text-lg">No sessions found.</p>
              <p className="text-slate-500 text-sm mt-2">Sync your device to see your runs here.</p>
            </div>
          ) : (
            sessions.map((session: any) => (
              <SessionCard key={session.id} session={session} />
            ))
          )}
        </div>
      </div>
    </div>
  );
}

function SessionCard({ session }: { session: any }) {
  const dateObj = new Date(session.uploadDate);
  const dateStr = dateObj.toLocaleDateString();
  const timeStr = dateObj.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });

  // Calculate Duration roughly (last point time - first point time)
  let duration = "00:00";
  if (session.points && session.points.length > 0) {
    const start = parseFloat(session.points[0].time);
    const end = parseFloat(session.points[session.points.length - 1].time);
    const diffSeconds = (end - start) / 1000;
    const mins = Math.floor(diffSeconds / 60);
    const secs = Math.floor(diffSeconds % 60);
    duration = `${mins}:${secs.toString().padStart(2, '0')}`;
  }

  return (
    <Link href={`/sessions/${session.id}`}>
      <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 hover:border-orange-500 transition">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-4">
          <div>
            <h3 className="text-white text-xl font-semibold mb-2 flex items-center">
              <MapPin className="w-5 h-5 mr-2 text-orange-500" />
              {session.originalFilename || "Unknown Session"}
            </h3>
            <div className="flex items-center space-x-4 text-sm text-slate-400">
              <span className="flex items-center">
                <Calendar className="w-4 h-4 mr-1" />
                {dateStr}
              </span>
              <span className="flex items-center">
                <Clock className="w-4 h-4 mr-1" />
                {timeStr}
              </span>
            </div>
          </div>
          <div className="mt-4 md:mt-0 text-right">
            {/* Placeholder for Best Lap if available later */}
            <div className="text-slate-500 text-sm">Session ID</div>
            <div className="text-slate-300 font-mono text-sm">{session.id}</div>
          </div>
        </div>

        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 pt-4 border-t border-slate-700">
          <div>
            <div className="text-slate-500 text-sm">Data Points</div>
            <div className="text-white font-semibold">{session.stats?.totalPoints || 0}</div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Top Speed</div>
            <div className="text-white font-semibold flex items-center">
              <Gauge className="w-4 h-4 mr-1 text-blue-400" />
              {session.stats?.maxSpeed?.toFixed(1) || 0} km/h
            </div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Max RPM</div>
            <div className="text-white font-semibold flex items-center">
              <Activity className="w-4 h-4 mr-1 text-red-500" />
              {session.stats?.maxRpm?.toFixed(0) || 0}
            </div>
          </div>
          <div>
            <div className="text-slate-500 text-sm">Duration</div>
            <div className="text-green-400 font-semibold flex items-center">
              <Clock className="w-4 h-4 mr-1" />
              {duration}
            </div>
          </div>
        </div>
      </div>
    </Link>
  );
}
